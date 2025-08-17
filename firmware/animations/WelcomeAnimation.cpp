
#include "WelcomeAnimation.h"

#include "NeoPixelBuffer.h"
#include "Miku.h"

uint32_t WelcomeAnimation::DrawFrame(NeoPixelFrame frame, uint32_t frameCounter)
{
    for(uint32_t i = 0; i < _pixelCount; ++i)
    {
        int col = (PixelPositions[i].y + frameCounter * 4) & 255;
        col -= (255 - 32);
        if(col < 0)
            col = 0;
        else
            col = (32 - col) * 4;
        frame.SetPixel(i, neopixel(0, col, 0).gammaCorrected());
    }
    //int currentPixel = (frameCounter) % frame.GetPixelCount();
    //int nextPixel = (frameCounter+1) % frame.GetPixelCount();
    //frame.SetPixel(currentPixel, neopixel(0, _greenLevel, 0));
    //frame.SetPixel(nextPixel, neopixel(0, _greenLevel++, 0));

    return 1000 / 60; // 60 FPS
}    
