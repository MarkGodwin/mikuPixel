#pragma once

#include "pico/stdlib.h"

class NeoPixelFrame;

class IAnimation
{
public:
    virtual uint32_t DrawFrame(NeoPixelFrame frame, uint32_t frameCounter) = 0;
};
