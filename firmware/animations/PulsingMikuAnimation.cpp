
#include "PulsingMikuAnimation.h"
#include "debug.h"
#include "NeoPixelBuffer.h"

PulsingMikuAnimation::PulsingMikuAnimation()
{
    // Initialize pulse parts
    for (size_t i = 0; i < std::size(pulseParts); ++i) {
        pulseParts[i] = 64;
    }
}

uint32_t PulsingMikuAnimation::DrawFrame(NeoPixelFrame frame, uint32_t frameCounter)
{
    // Randomly, approximately every 4 seconds, pulse a part
    if((random() & 255) == 0)
    {
        // Find a random part to pulse
        size_t partIndex = random() % std::size(AggregatedMikuParts);
        pulseParts[partIndex] = 512; // Reset the pulse value for this part
    }

    for(auto i = 0; i < std::size(AggregatedMikuParts); ++i)
    {
        auto part = AggregatedMikuParts[i];

        while(part->length > 0)
        {
            neopixel color = part->colour;
            if(pulseParts[i] > 256)
                color = part->colour.blend(neopixel(255, 255, 255), pulseParts[i] - 256).gammaCorrected();
            else
                color = part->colour.fade(pulseParts[i]).gammaCorrected();

            for(auto idx = part->index; idx < part->index + part->length; idx++)
                frame.SetPixel(idx, color);
            part++;
        }

        if(pulseParts[i] > 384 )
            pulseParts[i] -= 4;
        else if(pulseParts[i] > 64 )
            pulseParts[i] -= 16;
    }

    
    return 1000/60;
}