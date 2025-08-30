#include "mikuPixel.h"
#include "AnimationController.h"

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
#include "animations/TrainAnimation.h"
#include "animations/MarqueeAnimation.h"

AnimationController::AnimationController(
    std::shared_ptr<WebServer> webServer,
    std::shared_ptr<MqttClient> mqttClient,
    std::shared_ptr<DeviceConfig> deviceConfig,
    std::shared_ptr<AnimationRunner> animationRunner)
    : _webServer(std::move(webServer)),
    _mqttClient(std::move(mqttClient)),
    _deviceConfig(std::move(deviceConfig)),
    _animationRunner(std::move(animationRunner)),
    // Register built-in animations
    _animationFactories( {
        {"solid", "Solid Miku", []() { return std::make_unique<SolidMikuAnimation>(128); }},
        {"pulsing", "Pulsing Miku", []() { return std::make_unique<PulsingMikuAnimation>(); }},
        {"slowcycle", "Miku Part Cycle Slow", []() { return std::make_unique<MikuPartCycleAnimation>(1); }},
        {"fastcycle", "Miku Part Cycle Fast", []() { return std::make_unique<MikuPartCycleAnimation>(8); }},
        {"sweep", "Miku Sweep", []() { return std::make_unique<MikuSweepAnimation>(); }},
        {"welcome", "Welcome Sweep", []() { return std::make_unique<WelcomeAnimation>(PIXEL_COUNT); }},
        {"mapper", "LED Mapper", []() { return std::make_unique<PixelMapperAnimation>(PIXEL_COUNT, 4); }},
        {"ping", "Wifi Ping", []() { return std::make_unique<PingAnimation>(); }},
        {"drops", "Random Drops", []() { return std::make_unique<RandomDropsAnimation>(); }},
        {"trains", "Trains", []() { return std::make_unique<TrainAnimation>(); }},
        {"marquee", "Marquee", []() { return std::make_unique<MarqueeAnimation>(); }},
    })
{

    // Set up web endpoints
    _cgiHandlers.push_back(MakeCgiSubscription<bool>(_webServer, "/api/activatePattern.json", 
        [this](const CgiParams &params) { return ActivatePattern(params); }));
    _cgiHandlers.push_back(MakeCgiSubscription<bool>(_webServer, "/api/activateAnimation.json", 
        [this](const CgiParams &params) { return ActivateAnimation(params); }));

    _ssiHandlers.push_back(SsiSubscription(_webServer, "anims", [this](char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart) { return HandleAnimationsResponse(pcInsert, iInsertLen, tagPart, nextPart); }));

    DBG_PUT("Registering Animation MQTT handlers\n");
    _mqttHandlers.push_back(MqttSubscription(_mqttClient, string_format("miku/%s/fx/cmd", macAddress), [this](const uint8_t *payload, uint32_t length)
                 { OnEffectCommand(payload, length); })
    );
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
            auto& [shtName, animName, _] = tuple;
            return shtName == name || animName == name;
        });
        animationId = std::distance(_animationFactories.begin(), it);
    }

    if(animationId < 0 || animationId >= _animationFactories.size())
    {
        DBG_PRINT("Unknown animation ID: %d", animationId);
        return false;
    }
    
    auto &[_sn, _dn, factory] = _animationFactories[animationId];
    _animationRunner->SetAnimation(std::move(factory()));
    return true;
}

void AnimationController::OnEffectCommand(const uint8_t *payload, uint32_t length)
{
    // Expecting a string payload like "solid"
    std::string effectName;
    effectName.resize(length);
    memcpy(effectName.data(), payload, length);

    auto it = std::find_if(_animationFactories.begin(), _animationFactories.end(),
        [&effectName](const auto& tuple) {
            auto& [shtName, animName, _] = tuple;
            return shtName == effectName;
        });
    if(it == _animationFactories.end())
    {
        DBG_PRINT("Unknown effect name: %s", effectName.c_str());
        return;
    }

    auto animationId = std::distance(_animationFactories.begin(), it);
    StartAnimation(animationId);
}

void AnimationController::StartAnimation(int animationId)
{
    auto &[_sn, _dn, factory] = _animationFactories[animationId];
    _animationRunner->SetAnimation(std::move(factory()));

    _mqttClient->Publish(string_format("miku/%s/sw/state", macAddress).c_str(), (const uint8_t *)"ON", 2, true);
    _mqttClient->Publish(string_format("miku/%s/fx/state", macAddress).c_str(), (const uint8_t *)_sn.c_str(), _sn.length(), true);
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

    const auto &[shortName, animationName, _] = _animationFactories[tagPart];

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

std::vector<std::string> AnimationController::GetAvailableAnimations() const {
    std::vector<std::string> result;
    result.reserve(_animationFactories.size()); // avoid reallocations

    for (auto const& [name, description, factory] : _animationFactories) {
        result.push_back(name);
    }
    return result;
}
