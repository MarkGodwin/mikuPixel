#pragma once

#include "IAnimation.h"

class EditableImage : public IAnimation
{
public:
    EditableImage(const neopixel *pixels, uint32_t pixelCount):
        _pixels(pixels, pixels + pixelCount)
    {
    }

    bool SetPixel(uint32_t index, neopixel colour)
    {
        if (index >= _pixels.size())
        {
            return false;
        }
        _pixels[index] = colour;
        return true;
    }

    const std::vector<neopixel>& GetPixels() const
    {
        return _pixels;
    }

    virtual uint32_t DrawFrame(NeoPixelFrame frame, uint32_t frameCounter)
    {
        std::copy(_pixels.begin(), _pixels.end(), frame.GetBuffer());
        return 1000 / 30; // 30 FPS, so edits are visible
    }
private:

    std::vector<neopixel> _pixels;
};
