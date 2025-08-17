#pragma once

#include "webServer.h"
#include "deviceConfig.h"
#include "AnimationRunner.h"
#include <memory>
#include <list>

class AnimationController
{
public:
    AnimationController(std::shared_ptr<WebServer> webServer, 
                       std::shared_ptr<DeviceConfig> deviceConfig,
                       std::shared_ptr<AnimationRunner> animationRunner);

private:
    bool ActivatePattern(const CgiParams &params);
    bool ActivateAnimation(const CgiParams &params);

    int16_t HandleAnimationsResponse(char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart);

    using AnimationFactory = std::function<std::unique_ptr<IAnimation>()>;

    std::shared_ptr<WebServer> _webServer;
    std::shared_ptr<DeviceConfig> _deviceConfig;
    std::shared_ptr<AnimationRunner> _animationRunner;
    std::list<CgiSubscription> _cgiHandlers;
    std::list<SsiSubscription> _ssiHandlers;
    std::vector<std::tuple<std::string, AnimationFactory>> _animationFactories;
};
