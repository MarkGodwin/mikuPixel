#pragma once

#include "IAnimation.h"
#include "Miku.h"
#include "NeoPixelBuffer.h"

class MikuPartCycleAnimation : public IAnimation
{
public:
    MikuPartCycleAnimation(uint32_t speed)
    {
        _speed = speed;
    }

    virtual uint32_t DrawFrame(NeoPixelFrame frame, uint32_t frameCounter) override
    {
        memset(frame.GetBuffer(), 0, sizeof(neopixel) * PIXEL_COUNT);

        int fade = (frameCounter * _speed) % 256;
        int partIndex = ((frameCounter * _speed) / 256) % 10;
        auto part = mikuParts[partIndex];

        // Fading out
        for(auto idx = part.index; idx < part.index + part.length; idx++)
        {
            frame.SetPixel(idx, part.colour.fade(255 - fade).gammaCorrected() );
        }

        // Bright bit
        partIndex = (partIndex + 1) % 10;
        part = mikuParts[partIndex];
        for(auto idx = part.index; idx < part.index + part.length; idx++)
        {
            frame.SetPixel(idx, part.colour.gammaCorrected());
        }

        // Bright bit
        partIndex = (partIndex + 1) % 10;
        part = mikuParts[partIndex];
        for(auto idx = part.index; idx < part.index + part.length; idx++)
        {
            frame.SetPixel(idx, part.colour.gammaCorrected());
        }

        // Fading in
        partIndex = (partIndex + 1) % 10;
        part = mikuParts[partIndex];
        for(auto idx = part.index; idx < part.index + part.length; idx++)
        {
            frame.SetPixel(idx, part.colour.fade(fade).gammaCorrected());
        }

        return 16; // 60 FPS

    }

private:
    uint32_t _speed; // Speed of the fade
};
