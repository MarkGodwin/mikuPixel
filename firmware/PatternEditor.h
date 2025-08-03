#pragma once

#include <memory>
#include "deviceConfig.h"
#include "webServer.h"
#include "EditableImage.h"

class AnimationRunner;
class EditableImage;

class PatternEditor
{
    public:
        PatternEditor(
            uint16_t patternId,
            const neopixel *pixels,
            uint32_t pixelCount,
            AnimationRunner *animationRunner,
            std::shared_ptr<WebServer> webServer);

        // Disable copy constructor and assignment operator
        PatternEditor(const PatternEditor &) = delete;
        PatternEditor &operator=(const PatternEditor &) = delete;

        // Enable move constructor and assignment operator
        PatternEditor(PatternEditor &&) = default;
        PatternEditor &operator=(PatternEditor &&) = default;

        ~PatternEditor() = default;

        uint16_t GetPatternId() const
        {
            return _patternId;
        }

        const std::vector<neopixel> & GetPixels() const
        {
            return _editableImage->GetPixels();
        }

    private:
        bool OnSetLed(const CgiParams &params);

        CgiSubscription _setLedSubscription;
        std::shared_ptr<EditableImage> _editableImage;
        uint16_t _patternId;

};

