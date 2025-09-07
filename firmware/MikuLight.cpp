
#include "mikuPixel.h"
#include "MikuLight.h"
#include <algorithm>

#include "animations/SolidMikuAnimation.h"
#include "animations/SolidColourAnimation.h"
#include "animations/PatternSequenceAnimation.h"
#include "animations/WelcomeAnimation.h"
#include "animations/RandomDropsAnimation.h"
#include "animations/MikuPartCycleAnimation.h"
#include "animations/PixelMapperAnimation.h"
#include "animations/PulsingMikuAnimation.h"
#include "animations/MikuSweepAnimation.h"
#include "animations/PingAnimation.h"
#include "animations/TrainAnimation.h"
#include "animations/MarqueeAnimation.h"

#include "ColourUtils.h"
#include "mqttClient.h"

MikuLight::MikuLight(
    std::shared_ptr<DeviceConfig> deviceConfig,
    std::shared_ptr<AnimationRunner> animationRunner,
    std::shared_ptr<MqttClient> mqttClient)
    : _deviceConfig(std::move(deviceConfig)),
    _animationRunner(std::move(animationRunner)),
    _mqttClient(std::move(mqttClient)),
    _animationNames(),
    _publishTimer([this] () { return PublishMqttState(); }, 0),
    _saveTimer([this] () { return SaveState(); }, 0),
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
    // Build the name list once during construction
    _animationNames.reserve(_animationFactories.size());
    for (auto const& [name, description, factory] : _animationFactories) {
        _animationNames.emplace_back(name, description);
    }

}

void MikuLight::LoadConfig()
{
    auto loadedCfg = _deviceConfig->GetLightConfig();
    if(loadedCfg)
    {
        _lightConfig = *loadedCfg;
        switch(_lightConfig.state)
        {
            case LightState::Off:
                SetMikuBrightness(64); // Default on power-on
                break;
            case LightState::Miku:
                SetMikuBrightness(_lightConfig.brightness);
                break;
            case LightState::Colour:
            {
                auto [r, g, b] = ::HSBtoRGB(_lightConfig.hue, _lightConfig.saturation, _lightConfig.brightness);
                SetRgb(r, g, b);
                break;
            }
            case LightState::Animation:
            {
                StartAnimation(_lightConfig.animationId);
                break;
            }
            case LightState::Pattern:
            {
                ActivatePattern(_lightConfig.patternId);
                break;
            }
        }

        _publishTimer.ResetTimer(0);
        _saveTimer.ResetTimer(0);
    }

}

void MikuLight::SetRgb(int r, int g, int b)
{
    r = std::clamp(r, 0, 255);
    g = std::clamp(g, 0, 255);
    b = std::clamp(b, 0, 255);
    _animationRunner->SetAnimation(std::make_shared<SolidColourAnimation>(neopixel(r, g, b).gammaCorrected()));

    auto [h, s, br] = ::RGBtoHSB(r, g, b);

    _lightConfig.hue = std::round(h * 10.0f) / 10.0f;
    _lightConfig.saturation = std::round(s * 10.0f) / 10.0f;
    _lightConfig.brightness = br;
    _lightConfig.animationId = 0;
    auto brval = std::to_string(br);

    if(r > 0 || g > 0 || b > 0)
    {
        _lightConfig.state = LightState::Colour;
    }
    else
    {
        _lightConfig.state == LightState::Off;
    }

    TriggerStateChanged();
}

bool MikuLight::ActivatePattern(int patternId)
{
    auto pattern = _deviceConfig->GetPatternConfig(patternId);
    if(!pattern)
    {
        DBG_PUT("No such pattern to activate");
        return false; // No such pattern
    }

    _animationRunner->SetAnimation(std::make_shared<PatternSequenceAnimation>(patternId, _deviceConfig));
    _lightConfig.state = LightState::Pattern;
    _lightConfig.patternId = patternId;
    _lightConfig.animationId = 0;

    
    TriggerStateChanged();
    return true;
}

void MikuLight::SetMikuBrightness(int brightness)
{
    brightness = std::clamp(brightness, 0, 255);
    _animationRunner->SetAnimation(std::make_shared<SolidMikuAnimation>(brightness));

    if(brightness > 0)
    {
        _lightConfig.brightness = brightness;
        _lightConfig.saturation = 0.0f;
        _lightConfig.animationId = 0;

        _lightConfig.state = LightState::Miku;
    }
    else
    {
        _lightConfig.state = LightState::Off;
    }

    TriggerStateChanged();
}

bool MikuLight::StartAnimation(int animationId)
{
    if(animationId < 0 || animationId >= _animationFactories.size())
    {
        DBG_PRINT("Unknown animation ID: %d", animationId);
        return false;
    }

    if(_lightConfig.animationId == animationId)
        // Ignore requests to re-set the same current animation (usually "Solid" from HA)
        return true;
    
    auto &[_sn, _dn, factory] = _animationFactories[animationId];
    _animationRunner->SetAnimation(std::move(factory()));
    _lightConfig.state = LightState::Animation;
    _lightConfig.animationId = animationId;

    TriggerStateChanged();
    return true;
}

const std::vector<std::tuple<std::string,std::string>> &MikuLight::GetAvailableAnimations() const {
    return _animationNames;
}

void MikuLight::SetBrightness(int brightness)
{
    if(_lightConfig.saturation == 0.0f)
    {
        SetMikuBrightness(brightness);
    }
    else
    {
        auto [r, g, b] = HSBtoRGB(_lightConfig.hue, _lightConfig.saturation, brightness);
        SetRgb(r, g, b);
    }
}

void MikuLight::SetHueAndSaturation(float hue, float saturation)
{
    if(saturation == 0.0f)
    {
        _lightConfig.hue = hue;
        _lightConfig.saturation = saturation;
        SetMikuBrightness(_lightConfig.brightness);
    }
    else
    {
        auto [r, g, b] = HSBtoRGB(hue, saturation, _lightConfig.brightness);
        SetRgb(r, g, b);
    }
}

bool MikuLight::SwitchOn()
{
    if(_lightConfig.state == LightState::Off)
    {
        // Restore the previous colour/animation
        if(_lightConfig.animationId > 0)
            StartAnimation(_lightConfig.animationId);
        else if(_lightConfig.saturation == 0.0f)
            SetMikuBrightness(_lightConfig.brightness);
        else
        {
            auto [r, g, b] = HSBtoRGB(_lightConfig.hue, _lightConfig.saturation, _lightConfig.brightness);
            SetRgb(r, g, b);
        }
    }
    return true;
}

bool MikuLight::SwitchOff()
{
    SetMikuBrightness(0);
    return true;
}

int MikuLight::GetAnimationByName(const std::string &name) const
{
    auto it = std::find_if(_animationFactories.begin(), _animationFactories.end(),
        [&name](const auto& tuple) {
            auto& [shortName, longName, _] = tuple;
            return shortName == name || longName == name;
        });
    if(it == _animationFactories.end())
    {
        DBG_PRINT("Unknown effect name: %s", name.c_str());
        return -1;
    }

    auto animationId = std::distance(_animationFactories.begin(), it);
    return animationId;
}

uint32_t MikuLight::PublishMqttState()
{
    if(!_mqttClient->IsEnabled() )
        return 0;

    switch(_lightConfig.state)
    {
        case LightState::Off:
            _mqttClient->Publish(string_format("miku/%s/sw/state", macAddress).c_str(), (const uint8_t *)"OFF", 3, true);
            break;

        case LightState::Colour:
        {
            auto br = string_format("%.0f", _lightConfig.brightness);
            auto hs = string_format("%.1f,%.1f", _lightConfig.hue, _lightConfig.saturation);
            _mqttClient->Publish(string_format("miku/%s/br/state", macAddress).c_str(), (const uint8_t *)br.data(), br.length(), true);
            _mqttClient->Publish(string_format("miku/%s/hs/state", macAddress).c_str(), (const uint8_t *)hs.data(), hs.length(), true);
            _mqttClient->Publish(string_format("miku/%s/sw/state", macAddress).c_str(), (const uint8_t *)"ON", 2, true);
            _mqttClient->Publish(string_format("miku/%s/fx/state", macAddress).c_str(), (const uint8_t *)"solid", 5, true);
            break;
        }
        case LightState::Miku:
        {
            auto br = string_format("%.0f", _lightConfig.brightness);
            auto hs = string_format("%.1f,0", _lightConfig.hue);
            _mqttClient->Publish(string_format("miku/%s/br/state", macAddress).c_str(), (const uint8_t *)br.data(), br.length(), true);
            _mqttClient->Publish(string_format("miku/%s/hs/state", macAddress).c_str(), (const uint8_t *)hs.data(), hs.length(), true);
            _mqttClient->Publish(string_format("miku/%s/sw/state", macAddress).c_str(), (const uint8_t *)"ON", 2, true);
            _mqttClient->Publish(string_format("miku/%s/fx/state", macAddress).c_str(), (const uint8_t *)"solid", 5, true);
            break;
        }
        case LightState::Animation:
        {
            auto anim = std::get<0>(_animationNames[_lightConfig.animationId]);
            _mqttClient->Publish(string_format("miku/%s/sw/state", macAddress).c_str(), (const uint8_t *)"ON", 2, true);
            _mqttClient->Publish(string_format("miku/%s/fx/state", macAddress).c_str(), (const uint8_t *)anim.c_str(), anim.length(), true);
            break;
        }
        case LightState::Pattern:
        {
            // No MQTT for this...
            _mqttClient->Publish(string_format("miku/%s/sw/state", macAddress).c_str(), (const uint8_t *)"ON", 2, true);
            _mqttClient->Publish(string_format("miku/%s/fx/state", macAddress).c_str(), (const uint8_t *)"solid", 5, true);
            break;
        }
    }

    return 0;
}

uint32_t MikuLight::SaveState()
{
    _deviceConfig->SaveLightConfig(&_lightConfig);
    return 0;
}