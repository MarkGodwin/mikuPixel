#pragma once

#include "IAnimation.h"
#include "Miku.h"
#include "NeoPixelBuffer.h"

class SolidMikuAnimation : public IAnimation
{
public:
    SolidMikuAnimation(int fade):_fade(fade) {}

    virtual uint32_t DrawFrame(NeoPixelFrame frame, uint32_t frameCounter) override
    {

        for(auto partIndex = 0; partIndex < sizeof(mikuParts)/sizeof(mikuParts[0]); partIndex++)
        {
            auto part = mikuParts[partIndex];
            for(auto idx = part.index; idx < part.index + part.length; idx++)
            {
                frame.SetPixel(idx, part.colour.fade(_fade).gammaCorrected() );
            }
        }

        return 1000;
    }

private:
    int _fade;
};
