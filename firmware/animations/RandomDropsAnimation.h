#pragma once

#include "IAnimation.h"
#include "Miku.h"
#include "NeoPixelBuffer.h"

class RandomDropsAnimation : public IAnimation
{
public:

    virtual uint32_t DrawFrame(NeoPixelFrame frame, uint32_t frameCounter) override
    {
        auto input = frame.GetLastBuffer();
        auto output = frame.GetBuffer();

        for(auto x = 0; x < PIXEL_COUNT; x++)
        {
            auto left = (x - 1) % PIXEL_COUNT;
            auto right = (x + 1) % PIXEL_COUNT;
            output[x].red = ((int)input[left].red + input[x].red + input[x].red + input[right].red) / 4;
            output[x].green = ((int)input[left].green + input[x].green + input[x].green + input[right].green) / 4;
            output[x].blue = ((int)input[left].blue + input[x].blue + input[x].blue + input[right].blue) / 4;

            if((rand() & 1023) == 100)
            {
                //puts("drip");
                output[x].colour = 0;
                switch(rand() % 3)
                {
                    case 0:
                        output[x].red = 127;
                        break;
                    case 1:
                        output[x].green = 127;
                        break;
                    case 2:
                        output[x].blue = 127;
                        output[x].red = 127;
                        output[x].green = 127;
                        break;
                }
                output[x].red += rand() & 127;
                output[x].green += rand() & 127;
                output[x].blue += rand() & 127;           
            }        
        }

        return 16; // 60 FPS

    }
};
