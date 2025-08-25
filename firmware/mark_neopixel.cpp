#include <stdio.h>
#include <pico/stdlib.h>
#include "pico/sem.h"
#include "pico/flash.h"
#include "pico/cyw43_arch.h"
#include "pico/bootrom.h"
#include "hardware/dma.h"
#include "hardware/flash.h"
#include "hardware/watchdog.h"
#include "neopixel.pio.h"
#include <string.h>
#include <memory>

#include "NeoPixelBuffer.h"
#include "Miku.h"
#include <stdlib.h>
#include "debug.h"
#include "configService.h"
#include "deviceConfig.h"
#include "serviceStatus.h"
#include "serviceControl.h"
#include "AnimationRunner.h"
#include "animations/PixelMapperAnimation.h"
#include "animations/WelcomeAnimation.h"
#include "animations/PingAnimation.h"
#include "webServer.h"
#include "wifiConnection.h"
#include "wifiScanner.h"
#include "mqttClient.h"
#include "PatternList.h"
#include "AnimationController.h"
#include "animations/SolidMikuAnimation.h"


#define DMA_CHANNEL 0
#define PIXEL_PIN 2

// Hard buttons
#define PIN_RESET 14
#define PIN_WIFI 15

// How much flash memory to use to store wifi/mqtt config and LED settings. Beware changing these as it will corrupt the block storage
#define STORAGE_SECTORS 32
// Max 2 blocks stored in every sector. Aligning these will help with recovering deleted sectors
#define STORAGE_BLOCK_SIZE 2044


void doPollingSleep(uint ms)
{
#if PICO_CYW43_ARCH_POLL
    auto wakeTime = make_timeout_time_ms(ms);
    //auto totalPollTime = 0ll;
    //auto totalWaitTime = 0ll;
    do
    {
        //auto startPoll = get_absolute_time();
        cyw43_arch_poll();
        //auto endPoll = get_absolute_time();
        //totalPollTime += absolute_time_diff_us(startPoll, endPoll);
        cyw43_arch_wait_for_work_until(wakeTime);
        //totalWaitTime += absolute_time_diff_us(endPoll, get_absolute_time());
    } while(absolute_time_diff_us(get_absolute_time(), wakeTime) > 0);

    //if(totalPollTime > totalWaitTime)
    //{
    //    DBG_PRINT("Long poll: %u/%u ms\n", (int)(totalPollTime/1000), ms);
    //}
    
#else
    // We're using background IRQ callbacks from LWIP, so we can just sleep peacfully...
    sleep_ms(ms);
#endif
}

const WifiConfig *checkConfig(
    std::shared_ptr<DeviceConfig> config);

void runServiceMode(
    std::shared_ptr<WebServer> webServer,
    std::shared_ptr<DeviceConfig> config,
    std::shared_ptr<WifiConnection> wifiConnection,
    std::shared_ptr<AnimationRunner> commandQueue,
    std::shared_ptr<ServiceControl> service);

void runSetupMode(
    std::shared_ptr<WebServer> webServer,
    std::shared_ptr<ServiceControl> service);

bool checkButtonHeld(int inputPin);

int main()
{
#ifndef NDEBUG
    stdio_init_all();
#endif

    // Set up pins for the hard buttons
    gpio_init(PIN_RESET);
    gpio_set_dir(PIN_RESET, GPIO_IN);
    gpio_set_pulls(PIN_RESET, true, false);
    gpio_init(PIN_WIFI);
    gpio_set_dir(PIN_WIFI, GPIO_IN);
    gpio_set_pulls(PIN_WIFI, true, false);


    // LED timers need the async context set up to work
    if (cyw43_arch_init()) {
        sleep_ms(6000);
        DBG_PUT("Pico init failed");
        return -1;
    }


    auto neopixels = std::make_unique<NeoPixelBuffer>(DMA_CHANNEL, DMA_IRQ_0, pio0, 0, PIXEL_PIN, PIXEL_COUNT);

    auto animationRunner = std::make_shared<AnimationRunner>(std::move(neopixels));

    // Pointless startup cycle, to give me enough time to start putty

    animationRunner->SetAnimation(std::make_unique<WelcomeAnimation>(PIXEL_COUNT));
    animationRunner->Start();

    // Pointless startup cycle, to give me enough time to start putty
    doPollingSleep(6000);


    // Store flash settings at the very top of Flash memory
    // ALL STORED DATA WILL BE LOST IF THESE ARE CHANGED
    const uint32_t StorageSize = STORAGE_SECTORS * FLASH_SECTOR_SIZE;

    auto config = std::make_shared<DeviceConfig>(StorageSize, STORAGE_BLOCK_SIZE);
    auto wifiConfig = checkConfig(config);
    if(wifiConfig == nullptr)
    {
        DBG_PUT("Wifi configuration corrupt.");
        return -1;
    }

    // Uncomment to clear all patterns
    //config->SavePatternIds(nullptr, 0);

    auto apMode = !wifiConfig->ssid[0];

    auto service = std::make_shared<ServiceControl>();

    if(checkButtonHeld(PIN_WIFI))
    {
        DBG_PUT("Starting in WIFI Setup mode, as button WIFI button is pressed");
        apMode = true;
    }

    DBG_PUT("Starting the WiFi Scanner...");
    auto wifiConnection = std::make_shared<WifiConnection>(config, apMode);

    // Do an initial WiFi scan before entering AP mode. We don't seem to be able
    // to scan once in AP mode.
    auto wifiScanner = std::make_shared<WifiScanner>(apMode);
    wifiScanner->WaitForScan();

    DBG_PUT("Starting initial animation...");
    animationRunner->SetAnimation(std::make_unique<PingAnimation>());


    // Now connect to WiFi or Enable AP mode
    DBG_PRINT("Enabling WiFi in %s mode...\n", apMode ? "AP" : "Client");
    wifiConnection->Start();

    DBG_PUT("Starting the web interface...");
    auto webServer = std::make_shared<WebServer>(config, wifiConnection);
    webServer->Start();
    auto configService = std::make_shared<ConfigService>(config, webServer, wifiScanner, service);

    if(apMode)
    {
        runSetupMode(webServer, service);
    }
    else
    {
        runServiceMode(webServer, config, wifiConnection, animationRunner, service);
    }

    DBG_PUT("Restarting now!");
    doPollingSleep(1000);

    if(service->IsFirmwareUpdateRequested())
    {
        reset_usb_boot(0,0);
    }
    else
    {
        watchdog_reboot(0,0,500);
    }
    doPollingSleep(5000);
    DBG_PUT("ERM!!! Why no reboot?");
}

bool checkButtonHeld(int inputPin)
{
    int a = 0;
    for(a = 0; !gpio_get(inputPin) && a < 100; a++)
    {
        doPollingSleep(50);
    }
    return a == 100;
}

const WifiConfig *checkConfig(
    std::shared_ptr<DeviceConfig> config)
{
    DBG_PUT("Checking Device config...");
    
    auto needReset = config->GetWifiConfig() == nullptr;
    if(needReset)
        DBG_PUT("WiFi Config not found.");
    else
    {
        needReset = checkButtonHeld(PIN_RESET);
    }

    if(needReset)
    {
        DBG_PUT("Resetting all settings!");
        config->HardReset();
    }

    auto wifiConfig = config->GetWifiConfig();
    if(wifiConfig == nullptr ||
        config->GetMqttConfig() == nullptr)
    {
        DBG_PUT("Settings could not be read back. Error!\n");
        return nullptr;
    }
    return wifiConfig;
}

void runServiceMode(
    std::shared_ptr<WebServer> webServer,
    std::shared_ptr<DeviceConfig> config,
    std::shared_ptr<WifiConnection> wifiConnection,
    std::shared_ptr<AnimationRunner> animationRunner,
    std::shared_ptr<ServiceControl> service)
{

    auto mqttClient = std::make_shared<MqttClient>(config, wifiConnection, "miku/status", "online", "offline");

    mqttClient->Start();

    animationRunner->SetAnimation(std::make_unique<SolidMikuAnimation>());

    // Subscribe by wildcard to reduce overhead
    //mqttClient->SubscribeTopic("miku/cmd");
    
    DBG_PUT("Starting the Service Status Controller...");
    ServiceStatus statusApi(webServer, mqttClient, false);
    DBG_PUT("Starting the Patterns List...");
    PatternList patterns(webServer, config, animationRunner);
    DBG_PUT("Starting the Animation Controller...");
    AnimationController animationController(webServer, config, animationRunner);

    //ScheduledTimer republishTimer([&blinds, &remotes] () {
    //    // Try to republish discovery information for one device
    //    // It should be safe to access the collections from this callback.
    //    if(remotes->TryRepublish()||
    //        blinds->TryRepublish())
    //        // Do another later, if there is more work to be done
    //        // We can't publish everything at once, as lwip needs a chance to run to clear the queues
    //        return 100;
    //
    //    // Everything that needed to be published has been
    //    return 0;
    //}, 0);

    auto mqttConnected = false;

    auto asyncContext = cyw43_arch_async_context();

    DBG_PUT("Entering main loop...");
    bool resetPushed = false;
    while(!service->IsStopRequested()) {

        doPollingSleep(250);

        // Lazy man's notification of MQTT reconnection outside of an IRQ callback
        if(!mqttConnected)
        {
            if(mqttClient->IsConnected())
            {
                mqttConnected = true;
                //republishTimer.ResetTimer(250);
            }
        }
        else if(!mqttClient->IsConnected())
        {
            //republishTimer.ResetTimer(0);
            mqttConnected = false;
        }

        if(!gpio_get(PIN_RESET))
        {
            if(resetPushed)
            {
                DBG_PUT("Reset pushed!");
                service->StopService(false);
            }
            resetPushed = true;
        }
        else
        {
            resetPushed = false;
        }
        if(!gpio_get(PIN_WIFI))
        {
            // TODO: Reset WiFi or something in this case?
            DBG_PUT("WIFI pushed");
        }

    }

    //republishTimer.ResetTimer(0);

    DBG_PUT("Waiting for the command queue to clear...");
    animationRunner->Shutdown();


    DBG_PUT("Saving any unsaved state.\n");
    //remotes->SaveRemoteState();
    //blinds->SaveBlindState(true);

}

void runSetupMode(
    std::shared_ptr<WebServer> webServer,
    std::shared_ptr<ServiceControl> service)
{
    ServiceStatus statusApi(webServer, nullptr, true);

    DBG_PUT("Entering setup loop...");
    auto onOff = false;
    while(!service->IsStopRequested()) {

        doPollingSleep(1000);

        if(!gpio_get(PIN_RESET))
        {
            DBG_PUT("Reset pushed");
            service->StopService();
        }
        if(!gpio_get(PIN_WIFI))
            DBG_PUT("WIFI pushed");
    }

}