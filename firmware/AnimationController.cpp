#include "AnimationController.h"
#include "debug.h"

#include "animations/PatternSequenceAnimation.h"
#include "animations/WelcomeAnimation.h"
#include "animations/RandomDropsAnimation.h"
#include "animations/MikuPartCycleAnimation.h"
#include "animations/SolidMikuAnimation.h"
#include "animations/PixelMapperAnimation.h"
#include "animations/PulsingMikuAnimation.h"
#include <algorithm>
#include "animations/MikuSweepAnimation.h"
#include "animations/PingAnimation.h"

AnimationController::AnimationController(std::shared_ptr<WebServer> webServer,
                                       std::shared_ptr<DeviceConfig> deviceConfig,
                                       std::shared_ptr<AnimationRunner> animationRunner)
    : _webServer(std::move(webServer)),
    _deviceConfig(std::move(deviceConfig)),
    _animationRunner(std::move(animationRunner)),
    // Register built-in animations
    _animationFactories( {
        {"Solid Miku", []() { return std::make_unique<SolidMikuAnimation>(); }},
        {"Pulsing Miku", []() { return std::make_unique<PulsingMikuAnimation>(); }},
        {"Miku Part Cycle Slow", []() { return std::make_unique<MikuPartCycleAnimation>(1); }},
        {"Miku Part Cycle Fast", []() { return std::make_unique<MikuPartCycleAnimation>(8); }},
        {"Miku Sweep", []() { return std::make_unique<MikuSweepAnimation>(); }},
        {"Welcome Sweep", []() { return std::make_unique<WelcomeAnimation>(PIXEL_COUNT); }},
        {"LED Mapper", []() { return std::make_unique<PixelMapperAnimation>(PIXEL_COUNT, 4); }},
        {"Wifi Ping", []() { return std::make_unique<PingAnimation>(); }},
        {"Random Drops", []() { return std::make_unique<RandomDropsAnimation>(); }},
    })
{

    // Set up web endpoints
    _cgiHandlers.push_back(MakeCgiSubscription<bool>(_webServer, "/api/activatePattern.json", 
        [this](const CgiParams &params) { return ActivatePattern(params); }));
    _cgiHandlers.push_back(MakeCgiSubscription<bool>(_webServer, "/api/activateAnimation.json", 
        [this](const CgiParams &params) { return ActivateAnimation(params); }));

    _ssiHandlers.push_back(SsiSubscription(_webServer, "anims", [this](char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart) { return HandleAnimationsResponse(pcInsert, iInsertLen, tagPart, nextPart); }));
}

bool AnimationController::ActivatePattern(const CgiParams &params)
{
    auto patternId = params.find("id");
    if(patternId == params.end())
        return false;

    uint16_t id = std::stoul(patternId->second);
    auto pattern = _deviceConfig->GetPatternConfig(id);
    if(!pattern)
    {
        DBG_PUT("No such pattern to activate");
        return false; // No such pattern
    }

    _animationRunner->SetAnimation(std::make_shared<PatternSequenceAnimation>(id, _deviceConfig));
    return true;
}

bool AnimationController::ActivateAnimation(const CgiParams &params)
{
    auto animationId = 0;
    auto animationIdParam = params.find("id");
    if(animationIdParam != params.end())
        animationId = std::stoi(animationIdParam->second);
    else
    {
        auto animationName = params.find("name");
        if(animationName == params.end())
            return false;

        const auto &name = animationName->second;
        auto it = std::find_if(_animationFactories.begin(), _animationFactories.end(),
        [&name](const auto& tuple) {
            auto& [animName, _] = tuple;
            return animName == name;
        });
        animationId = std::distance(_animationFactories.begin(), it);
    }

    if(animationId < 0 || animationId >= _animationFactories.size())
    {
        DBG_PRINT("Unknown animation ID: %d", animationId);
        return false;
    }
    
    auto &[_, factory] = _animationFactories[animationId];
    _animationRunner->SetAnimation(std::move(factory()));
    return true;
}

int16_t AnimationController::HandleAnimationsResponse(char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart)
{
    if(tagPart == 0)
    {
        DBG_PRINT("Returning %d patterns\n", _animationFactories.size());
    }

    if(tagPart >= _animationFactories.size())
    {
        return 0; // No more patterns
    }

    const auto &[animationName, _] = _animationFactories[tagPart];

    BufferOutput outputter(pcInsert, iInsertLen);
    outputter.Append("{ \"id\": ");
    outputter.Append((int)tagPart);
    outputter.Append(", \"name\": \"");
    outputter.AppendEscaped(animationName.c_str());
    outputter.Append("\" ");

    if(tagPart + 1 < _animationFactories.size())
    {
        outputter.Append(" },");
        *nextPart = tagPart + 1;
    }
    else
        outputter.Append(" }");
    
    return outputter.BytesWritten();
}