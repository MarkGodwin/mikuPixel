#include "mikuPixel.h"
#include "MarqueeAnimation.h"
#include <Miku.h>

uint32_t MarqueeAnimation::DrawFrame(NeoPixelFrame frame, uint32_t frameCounter)
{
    const neopixel magenta(200, 0, 200);
    const neopixel cyan(0, 200, 200);

    auto offset = frameCounter & 31;


    if(frameCounter & 0x800)
    {
        for(auto y = 0; y < 127; y++)
        {
            for(auto x = 0; x < 127; x++)
            {
                GridBuffer[y][x] = ((y + offset) & 0x10) ? magenta : cyan;
            }
        }
    }
    else
    {
        for(auto y = 0; y < 127; y++)
        {
            for(auto x = 0; x < 127; x++)
            {
                GridBuffer[x][y] = ((y + offset) & 0x10) ? magenta : cyan;
            }
        }
    }

    for(auto i = 0; i < PIXEL_COUNT; i++)
    {
        frame.SetPixel(i, GridBuffer[PixelPositions[i].y][PixelPositions[i].x]);
    }

    return 10;

}