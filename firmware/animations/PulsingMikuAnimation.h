#pragma once

#include "IAnimation.h"
#include "Miku.h"

class PulsingMikuAnimation : public IAnimation
{
public:
    PulsingMikuAnimation();

    virtual uint32_t DrawFrame(NeoPixelFrame frame, uint32_t frameCounter) override;

private:
    uint32_t pulseParts[std::size(AggregatedMikuParts)];
};
