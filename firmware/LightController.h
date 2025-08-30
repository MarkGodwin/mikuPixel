#pragma once

#include "webServer.h"
#include "mqttClient.h"
#include "deviceConfig.h"
#include "AnimationRunner.h"
#include <memory>
#include <list>

class LightController
{
public:
    LightController(
        std::shared_ptr<WebServer> webServer, 
        std::shared_ptr<MqttClient> mqttClient,
        std::shared_ptr<AnimationRunner> animationRunner);

private:
    bool OnSetRgbCgi(const CgiParams &params);
    bool OnSetBrightnessCgi(const CgiParams &params);

    void OnSwitchCommand(const uint8_t *payload, uint32_t length);
    void OnBrightnessCommand(const uint8_t *payload, uint32_t length);
    void OnHsCommand(const uint8_t *payload, uint32_t length);
    void OnWhiteCommand(const uint8_t *payload, uint32_t length);

    void SetPatternBrightness(int brightness);
    void SetRgb(int r, int g, int b);

    std::tuple<int,int,int> HSBtoRGB(float hue, float sat, int brightness);
    std::tuple<float,float,int> RGBtoHSB(int red, int green, int blue);

    std::shared_ptr<WebServer> _webServer;
    std::shared_ptr<MqttClient> _mqttClient;
    std::shared_ptr<AnimationRunner> _animationRunner;
    std::list<CgiSubscription> _cgiHandlers;
    std::list<SsiSubscription> _ssiHandlers;
    std::list<MqttSubscription> _mqttHandlers;

    bool _on = true;
    int _hue = 0;
    int _saturation = 0; // Fully white
    int _brightness = 127; // Half brightness

};
