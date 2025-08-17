#pragma once

#include "IAnimation.h"

class WelcomeAnimation : public IAnimation
{
public:
    WelcomeAnimation(uint32_t pixelCount)
    :   _pixelCount(pixelCount)
    {
    }

    virtual uint32_t DrawFrame(NeoPixelFrame frame, uint32_t frameCounter) override;

    private:
        uint32_t _pixelCount;  // Number of pixels in the animation
};
