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
#include "statusLed.h"
#include "serviceStatus.h"
#include "serviceControl.h"
#include "AnimationRunner.h"
#include "PixelMapperAnimation.h"
#include "webServer.h"
#include "wifiConnection.h"
#include "wifiScanner.h"
#include "mqttClient.h"
#include "PatternList.h"


#define DMA_CHANNEL 0
#define PIXEL_PIN 2

// Hard buttons
#define PIN_STATUSLED 13
#define PIN_RESET 14
#define PIN_WIFI 15

// How much flash memory to use to store wifi/mqtt config and LED settings. Beware changing these as it will corrupt the block storage
#define STORAGE_SECTORS 32
// Max 2 blocks stored in every sector. Aligning these will help with recovering deleted sectors
#define STORAGE_BLOCK_SIZE 2044


static uint32_t miku_all_parts(uint32_t frame, NeoPixelFrame *neopixels)
{


    for(auto partIndex = 0; partIndex < sizeof(mikuParts)/sizeof(mikuParts[0]); partIndex++)
    {
        auto part = mikuParts[partIndex];
        for(auto idx = part.index; idx < part.index + part.length; idx++)
        {
            neopixels->SetPixel(idx, part.colour.fade(128).gammaCorrected() );
        }
    }

    return 1000;
}


static uint32_t miku_part_cycle(uint32_t frame, NeoPixelFrame *neopixels)
{
    memset(neopixels->GetBuffer(), 0, sizeof(neopixel) * PIXEL_COUNT);

    int fade = frame % 256;
    int partIndex = (frame / 256) % 10;
    auto part = mikuParts[partIndex];

    // Fading out
    for(auto idx = part.index; idx < part.index + part.length; idx++)
    {
        neopixels->SetPixel(idx, part.colour.fade(255 - fade).gammaCorrected() );
    }

    // Bright bit
    partIndex = (partIndex + 1) % 10;
    part = mikuParts[partIndex];
    for(auto idx = part.index; idx < part.index + part.length; idx++)
    {
        neopixels->SetPixel(idx, part.colour.gammaCorrected());
    }

    // Bright bit
    partIndex = (partIndex + 1) % 10;
    part = mikuParts[partIndex];
    for(auto idx = part.index; idx < part.index + part.length; idx++)
    {
        neopixels->SetPixel(idx, part.colour.gammaCorrected());
    }

    // Fading in
    partIndex = (partIndex + 1) % 10;
    part = mikuParts[partIndex];
    for(auto idx = part.index; idx < part.index + part.length; idx++)
    {
        neopixels->SetPixel(idx, part.colour.fade(fade).gammaCorrected());
    }

    return 0;
}

static uint32_t ring_spin(uint32_t frame, NeoPixelFrame *neopixels)
{
    static uint8_t intensity = 255;
    static uint8_t colIndex = 0;

    memset(neopixels->GetBuffer(), 0, sizeof(neopixel) * PIXEL_COUNT);

    neopixel col{0};

    auto idx = frame % PIXEL_COUNT;

    neopixels->SetPixel(idx, 0xFFFFFF00);
    neopixels->SetPixel((idx + 50) % PIXEL_COUNT, 0x0000FF00);
    neopixels->SetPixel((idx + 100) % PIXEL_COUNT, 0xFF000000);
    neopixels->SetPixel((idx + 150) % PIXEL_COUNT, 0x00FF0000);
    neopixels->SetPixel((idx + 200) % PIXEL_COUNT, 0x0000FF00);
    neopixels->SetPixel((idx + 250) % PIXEL_COUNT, 0xFF000000);
    neopixels->SetPixel((idx + 300) % PIXEL_COUNT, 0x00FF0000);

    return 10;
}

uint32_t blurDrops(uint32_t frame, NeoPixelFrame *neopixels)
{
    auto input = neopixels->GetLastBuffer();
    auto output = neopixels->GetBuffer();


    for(auto x = 0; x < PIXEL_COUNT; x++)
    {
        auto left = (x - 1) % PIXEL_COUNT;
        auto right = (x + 1) % PIXEL_COUNT;
        output[x].red = ((int)input[left].red + input[x].red + input[x].red + input[right].red) / 4;
        output[x].green = ((int)input[left].green + input[x].green + input[x].green + input[right].green) / 4;
        output[x].blue = ((int)input[left].blue + input[x].blue + input[x].blue + input[right].blue) / 4;

        if((rand() & 1023) == 100)
        {
            //puts("drip");
            output[x].colour = 0;
            switch(rand() % 3)
            {
                case 0:
                    output[x].red = 127;
                    break;
                case 1:
                    output[x].green = 127;
                    break;
                case 2:
                    output[x].blue = 127;
                    output[x].red = 127;
                    output[x].green = 127;
                    break;
            }
            output[x].red += rand() & 127;
            output[x].green += rand() & 127;
            output[x].blue += rand() & 127;           
        }        
    }



    return 10;
}

neopixel _pattern[127][127];

uint32_t pattern_scroll(uint32_t frame, NeoPixelFrame *neopixels)
{
    const neopixel magenta(200, 0, 200);
    const neopixel cyan(0, 200, 200);

    auto offset = frame & 31;


    if(frame & 0x800)
    {
        for(auto y = 0; y < 127; y++)
        {
            for(auto x = 0; x < 127; x++)
            {
                _pattern[y][x] = ((y + offset) & 0x10) ? magenta : cyan;
            }
        }
    }
    else
    {
        for(auto y = 0; y < 127; y++)
        {
            for(auto x = 0; x < 127; x++)
            {
                _pattern[x][y] = ((y + offset) & 0x10) ? magenta : cyan;
            }
        }
    }

    for(auto i = 0; i < PIXEL_COUNT; i++)
    {
        neopixels->SetPixel(i, _pattern[PixelPositions[i].y][PixelPositions[i].x]);
    }

    return 10;
}





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
    std::shared_ptr<DeviceConfig> config,
    StatusLed *statusLed);

void runServiceMode(
    std::shared_ptr<WebServer> webServer,
    std::shared_ptr<DeviceConfig> config,
    std::shared_ptr<WifiConnection> wifiConnection,
    std::shared_ptr<AnimationRunner> commandQueue,
    std::shared_ptr<ServiceControl> service,
    StatusLed *mqttLed);

void runSetupMode(
    std::shared_ptr<WebServer> webServer,
    std::shared_ptr<ServiceControl> service);

bool checkButtonHeld(int inputPin, StatusLed *led);

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

    // Pointless startup cycle, to give me enough time to start putty

    StatusLed statusLed(PIN_STATUSLED);

    // Pointless startup cycle, to give me enough time to start putty
    statusLed.Pulse(256, 768, 128);
    doPollingSleep(6000);
    statusLed.TurnOff();

    auto neopixels = std::make_unique<NeoPixelBuffer>(DMA_CHANNEL, DMA_IRQ_0, pio0, 0, PIXEL_PIN, PIXEL_COUNT);

    auto animationRunner = std::make_shared<AnimationRunner>(std::move(neopixels));

    animationRunner->SetAnimation(std::make_unique<PixelMapperAnimation>(PIXEL_COUNT, 2));
    animationRunner->Start();

    // Store flash settings at the very top of Flash memory
    // ALL STORED DATA WILL BE LOST IF THESE ARE CHANGED
    const uint32_t StorageSize = STORAGE_SECTORS * FLASH_SECTOR_SIZE;

    auto config = std::make_shared<DeviceConfig>(StorageSize, STORAGE_BLOCK_SIZE);
    auto wifiConfig = checkConfig(config, &statusLed);
    if(wifiConfig == nullptr)
    {
        statusLed.SetLevel(256);
        return -1;
    }

    auto apMode = !wifiConfig->ssid[0];

    auto service = std::make_shared<ServiceControl>();

    if(checkButtonHeld(PIN_WIFI, &statusLed))
    {
        DBG_PUT("Starting in WIFI Setup mode, as button WIFI button is pressed");
        apMode = true;
    }

    DBG_PUT("Starting the WiFi Scanner...");
    auto wifiConnection = std::make_shared<WifiConnection>(config, apMode, &statusLed);

    // Do an initial WiFi scan before entering AP mode. We don't seem to be able
    // to scan once in AP mode.
    auto wifiScanner = std::make_shared<WifiScanner>(apMode);
    wifiScanner->WaitForScan();

    // Now connect to WiFi or Enable AP mode
    DBG_PRINT("Enabling WiFi in %s mode...\n", apMode ? "AP" : "Client");
    wifiConnection->Start();

    DBG_PUT("Starting the web interface...");
    auto webServer = std::make_shared<WebServer>(config, wifiConnection, &statusLed);
    webServer->Start();
    auto configService = std::make_shared<ConfigService>(config, webServer, wifiScanner, service);

    if(apMode)
    {
        runSetupMode(webServer, service);
    }
    else
    {
        runServiceMode(webServer, config, wifiConnection, animationRunner, service, &statusLed);
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

bool checkButtonHeld(int inputPin, StatusLed *led)
{
    int a = 0;
    for(a = 0; !gpio_get(inputPin) && a < 100; a++)
    {
        if(a == 0)
            led->Pulse(1024, 2048, 256);
        doPollingSleep(50);
    }
    led->TurnOff();
    return a == 100;
}

const WifiConfig *checkConfig(
    std::shared_ptr<DeviceConfig> config,
    StatusLed *redLed)
{
    DBG_PUT("Checking Device config...");
    
    auto needReset = config->GetWifiConfig() == nullptr;
    if(needReset)
        DBG_PUT("WiFi Config not found.");
    else
    {
        needReset = checkButtonHeld(PIN_RESET, redLed);
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
    std::shared_ptr<ServiceControl> service,
    StatusLed *mqttLed)
{

    auto mqttClient = std::make_shared<MqttClient>(config, wifiConnection, "miku/status", "online", "offline", mqttLed);

    mqttClient->Start();
    // Subscribe by wildcard to reduce overhead
    //mqttClient->SubscribeTopic("miku/cmd");
    

    ServiceStatus statusApi(webServer, mqttClient, false);
    PatternList patterns(webServer, config, animationRunner);

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