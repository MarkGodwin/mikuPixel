#pragma once

#include "IAnimation.h"
#include "Miku.h"
#include "NeoPixelBuffer.h"

class SolidColourAnimation : public IAnimation
{
public:
    SolidColourAnimation(neopixel colour): _colour(colour) {}

    virtual uint32_t DrawFrame(NeoPixelFrame frame, uint32_t frameCounter) override
    {
        for(auto i = 0; i < PIXEL_COUNT; i++)
        {
            frame.SetPixel(i, _colour);
        }

        return 1000;
    }

private:
    neopixel _colour;
};

