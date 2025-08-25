#include "IAnimation.h"

#include "NeoPixelBuffer.h"

class MarqueeAnimation : public IAnimation
{
public:
    virtual uint32_t DrawFrame(NeoPixelFrame frame, uint32_t frameCounter) override;
};
