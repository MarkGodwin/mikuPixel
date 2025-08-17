#pragma once

#include "IAnimation.h"
#include "deviceConfig.h"
#include <memory>

class PatternSequenceAnimation : public IAnimation
{
public:
    PatternSequenceAnimation(uint16_t startPatternId, std::shared_ptr<DeviceConfig> deviceConfig);

    virtual uint32_t DrawFrame(NeoPixelFrame frame, uint32_t frameCounter) override;

private:
    std::shared_ptr<DeviceConfig> _deviceConfig;
    uint16_t _currentPatternId;
    const PatternConfig* _currentPattern;
    const PatternConfig* _nextPattern;
    uint32_t _transitionStartTime; // Frame counter when the transition started
    uint32_t _transitionDuration; // Duration of the current transition in frames
    bool _inTransition;
};

