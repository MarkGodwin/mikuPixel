#pragma once

#include "webServer.h"
#include "mqttClient.h"
#include "deviceConfig.h"
#include "AnimationRunner.h"
#include <memory>
#include <list>

class MikuLight;

/// @brief Network controller for remote miku light control via web & mqtt APIs
class LightController
{
public:
    LightController(
        std::shared_ptr<MikuLight> mikuLight,
        std::shared_ptr<WebServer> webServer, 
        std::shared_ptr<MqttClient> mqttClient);
private:
    bool OnSetRgbCgi(const CgiParams &params);
    bool OnSetBrightnessCgi(const CgiParams &params);
    bool ActivatePatternCgi(const CgiParams &params);
    bool ActivateAnimationCgi(const CgiParams &params);

    void OnSwitchCommand(const uint8_t *payload, uint32_t length);
    void OnBrightnessCommand(const uint8_t *payload, uint32_t length);
    void OnHsCommand(const uint8_t *payload, uint32_t length);
    void OnWhiteCommand(const uint8_t *payload, uint32_t length);

    std::tuple<int,int,int> HSBtoRGB(float hue, float sat, int brightness);
    std::tuple<float,float,int> RGBtoHSB(int red, int green, int blue);

    void OnEffectCommand(const uint8_t *payload, uint32_t length);

    int16_t HandleAnimationsResponse(char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart);

    std::shared_ptr<MikuLight> _mikuLight;
    std::shared_ptr<WebServer> _webServer;
    std::shared_ptr<MqttClient> _mqttClient;
    std::list<CgiSubscription> _cgiHandlers;
    std::list<SsiSubscription> _ssiHandlers;
    std::list<MqttSubscription> _mqttHandlers;


};
