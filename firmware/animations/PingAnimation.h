#pragma once

#include "IAnimation.h"
#include "NeoPixelBuffer.h"
#include "Miku.h"

class PingAnimation : public IAnimation
{
public:
    virtual uint32_t DrawFrame(NeoPixelFrame frame, uint32_t frameCounter) override
    {

        long radius = (frameCounter * 2) & 127;

        // Clear the frame
        for(auto i = 0; i < PIXEL_COUNT; i++)
        {
            // Set colour based on distance from center
            auto pos = PixelPositions[i];
            auto dx = pos.x - 64;
            auto dy = pos.y - 64;
            auto dist = abs(dx * dx + dy * dy - (radius * radius));

            if(dist < 16)
            {
                // Within the radius, set to white
                frame.SetPixel(i, neopixel(128, 128, 128));
            }
            else if(dist < 32)
            {
                // Close to the radius, set to blue
                frame.SetPixel(i, neopixel(0, 0, 128));
            }
            else
            {
                // Outside the radius, set to black
                frame.SetPixel(i, neopixel(0, 0, 0));
            }
        }

        return 1000 / 60; // 60 FPS
    }
};
