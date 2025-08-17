#include "PatternSequenceAnimation.h"
#include "debug.h"
#include <algorithm>

PatternSequenceAnimation::PatternSequenceAnimation(uint16_t startPatternId, std::shared_ptr<DeviceConfig> deviceConfig)
    : _deviceConfig(std::move(deviceConfig))
    , _currentPatternId(startPatternId)
    , _currentPattern(nullptr)
    , _nextPattern(nullptr)
    , _transitionStartTime(0)
    , _inTransition(false)
{
    // Load the initial pattern
    _currentPattern = _deviceConfig->GetPatternConfig(_currentPatternId);
    if (_currentPattern && _currentPattern->nextFrameId >= 0) {
        _nextPattern = _deviceConfig->GetPatternConfig(_currentPattern->nextFrameId);
    }
}

uint32_t PatternSequenceAnimation::DrawFrame(NeoPixelFrame frame, uint32_t frameCounter)
{
    if (!_currentPattern) {
        // No valid pattern, output black and wait a bit
        std::fill(frame.GetBuffer(), frame.GetBuffer() + PIXEL_COUNT, neopixel(0,0,0));
        return 1000; // Wait 1 second before trying again
    }

    // We're in a transition
    uint32_t elapsed = 0;
    if(_inTransition)
    {
        elapsed = frameCounter - _transitionStartTime;
        
        if (elapsed >= _transitionDuration) {
            // Transition complete - make next frame current
            _currentPattern = _nextPattern;
            _currentPatternId = _currentPattern->nextFrameId;
            _inTransition = false;
            
            // Load the next pattern in sequence if there is one
            _nextPattern = (_currentPattern->nextFrameId >= 0) ? 
                _deviceConfig->GetPatternConfig(_currentPattern->nextFrameId) : nullptr;
                
            _inTransition = false;
        }
    }

    if (!_inTransition) {
        // Not in transition - just show current frame
        std::transform(_currentPattern->pixels, _currentPattern->pixels + PIXEL_COUNT, frame.GetBuffer(), [](const neopixel &pixel) { return pixel.gammaCorrected(); });

        // If no next frame or no transition time, just keep showing this frame
        if (_currentPattern->nextFrameId < 0 || _currentPattern->transitionTime == 0) {
            return _currentPattern->frameTime;
        }

        // Time to start transition to next frame
        if (_nextPattern) {
            _inTransition = true;
            _transitionStartTime = frameCounter;
            _transitionDuration = _currentPattern->transitionTime / 60;
        }
        
        return _currentPattern->frameTime;
    }


    // Blend between current and next frame with gamma correction
    for (size_t i = 0; i < PIXEL_COUNT; i++) {
        auto currentPixel = _currentPattern->pixels[i].gammaCorrected();
        auto nextPixel = _nextPattern->pixels[i].gammaCorrected();
        
        frame.SetPixel(i, neopixel(
            (currentPixel.red * (_transitionDuration - elapsed) + nextPixel.red * elapsed) / _transitionDuration,
            (currentPixel.green * (_transitionDuration - elapsed) + nextPixel.green * elapsed) / _transitionDuration,
            (currentPixel.blue * (_transitionDuration - elapsed) + nextPixel.blue * elapsed) / _transitionDuration
        ));
    }

    // 60fps during transition 
    return 1000 / 60;
}
