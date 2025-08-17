#pragma once

#include "IAnimation.h"
#include "NeoPixelBuffer.h"

class MikuSweepAnimation : public IAnimation
{
public:
    //MikuSweepAnimation();

    virtual uint32_t DrawFrame(NeoPixelFrame frame, uint32_t frameCounter) override;

private:
};
