#pragma once

#include "IAnimation.h"

class PixelMapperAnimation : public IAnimation
{
public:
    PixelMapperAnimation(uint32_t pixelCount, uint32_t frameRate)
    :   _pixelCount(pixelCount),
        _frameDelay(1000/frameRate)
    {
    }

    virtual uint32_t DrawFrame(NeoPixelFrame frame, uint32_t frameCounter) override;

    private:
        uint32_t _pixelCount;  // Number of pixels in the animation
        uint32_t _frameDelay;  // Milliseconds per frame
};
