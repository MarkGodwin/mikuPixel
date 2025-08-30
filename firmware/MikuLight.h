#pragma once

#include "deviceConfig.h"
#include "AnimationRunner.h"
#include <functional>
#include "scheduler.h"

class MqttClient;

class MikuLight
{
public:
    MikuLight(
        std::shared_ptr<DeviceConfig> deviceConfig,
        std::shared_ptr<AnimationRunner> animationRunner,
        std::shared_ptr<MqttClient> mqttClient
    );

    const std::vector<std::tuple<std::string,std::string>> &GetAvailableAnimations() const;

    void LoadConfig();
    void SetMikuBrightness(int brightness);
    void SetRgb(int r, int g, int b);

    void SetHueAndSaturation(float hue, float saturation);
    void SetBrightness(int brightness); // Special brightness shared between HSV and steady miku colours
    bool StartAnimation(int animationId);
    bool ActivatePattern(int patternId);
    bool SwitchOn();
    bool SwitchOff();

    int GetAnimationByName(const std::string &name) const;
private:

    uint32_t PublishMqttState();
    uint32_t SaveState();
    void TriggerStateChanged()
    {
        _publishTimer.ResetTimer(250);
        _saveTimer.ResetTimer(60000);
    }
    

    std::shared_ptr<AnimationRunner> _animationRunner;
    std::shared_ptr<DeviceConfig> _deviceConfig;
    std::shared_ptr<MqttClient> _mqttClient;
    ScheduledTimer _publishTimer;
    ScheduledTimer _saveTimer; // Only save state after 60 seconds, to avoid flash wear

    using AnimationFactory = std::function<std::unique_ptr<IAnimation>()>;
    std::vector<std::tuple<std::string, std::string, AnimationFactory>> _animationFactories;
    std::vector<std::tuple<std::string, std::string>> _animationNames;

    LightConfig _lightConfig = { LightState::Miku, 0.0f, 0.0f, 127, 0, 0 };

};