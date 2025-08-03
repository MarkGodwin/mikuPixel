
#include "PixelMapperAnimation.h"

#include "NeoPixelBuffer.h"

uint32_t PixelMapperAnimation::DrawFrame(NeoPixelFrame frame, uint32_t frameCounter)
{
    // Clear the frame buffer
    frame.Clear();

    int currentPixel = (frameCounter) % frame.GetPixelCount();
    frame.SetPixel(currentPixel, neopixel(255, 255, 255));

    return _frameDelay;
}    
