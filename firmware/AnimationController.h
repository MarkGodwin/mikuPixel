#pragma once

#include "webServer.h"
#include "mqttClient.h"
#include "deviceConfig.h"
#include "AnimationRunner.h"
#include <memory>
#include <list>

class AnimationController
{
public:
    AnimationController(
        std::shared_ptr<WebServer> webServer,
        std::shared_ptr<MqttClient> mqttClient,
        std::shared_ptr<DeviceConfig> deviceConfig,
        std::shared_ptr<AnimationRunner> animationRunner);

    std::vector<std::string> GetAvailableAnimations() const;
private:
    bool ActivatePattern(const CgiParams &params);
    bool ActivateAnimation(const CgiParams &params);

    void OnEffectCommand(const uint8_t *payload, uint32_t length);

    int16_t HandleAnimationsResponse(char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart);

    void StartAnimation(int animationId);

    using AnimationFactory = std::function<std::unique_ptr<IAnimation>()>;

    std::shared_ptr<WebServer> _webServer;
    std::shared_ptr<MqttClient> _mqttClient;
    std::shared_ptr<DeviceConfig> _deviceConfig;
    std::shared_ptr<AnimationRunner> _animationRunner;
    std::list<CgiSubscription> _cgiHandlers;
    std::list<SsiSubscription> _ssiHandlers;
    std::list<MqttSubscription> _mqttHandlers;
    std::vector<std::tuple<std::string, std::string, AnimationFactory>> _animationFactories;
};
