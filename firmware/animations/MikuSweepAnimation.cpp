
#include "MikuSweepAnimation.h"
#include "Miku.h"
#include "NeoPixelBuffer.h"
#include <cmath>

// Sweep types and their direction vectors
static const struct {
    int32_t dx;
    int32_t dy;
} sweepDirections[] = {
    {1, 0},   // Right
    {0, 1},   // Up
    {-1, 0},  // Left
    {0, -1},  // Down
    {1, 1},   // Diagonal up-right
    {-1, 1},  // Diagonal up-left
    {-1, -1}, // Diagonal down-left
    {1, -1}   // Diagonal down-right
};

uint32_t MikuSweepAnimation::DrawFrame(NeoPixelFrame frame, uint32_t frameCounter)
{

    // Calculate sweep parameters
    constexpr int32_t sweepInterval = 120;  // Frames between sweep starts
    constexpr int32_t sweepWidth = 32;     // Width of the sweep in pixels
    constexpr int32_t sweepDuration = 60;  // How long each sweep takes

    int32_t sweepPhase = frameCounter % sweepInterval;
    if (sweepPhase >= sweepDuration) {
        // No active sweep, just return the base frame
        for(auto partIndex = 0; partIndex < sizeof(mikuParts)/sizeof(mikuParts[0]); partIndex++)
        {
            auto part = mikuParts[partIndex];
            for(auto idx = part.index; idx < part.index + part.length; idx++)
            {
                frame.SetPixel(idx, part.colour.fade(128).gammaCorrected());
            }
        }

        return 1000 / 60;
    }

    // Select sweep direction based on which sweep we're on
    uint32_t sweepIndex = (frameCounter / sweepInterval) % (sizeof(sweepDirections) / sizeof(sweepDirections[0]));
    auto direction = sweepDirections[sweepIndex];

    // Unit direction vector of sweep
    int32_t dx = direction.dx;
    int32_t dy = direction.dy;

    // Progress along sweep direction (negative → positive)
    int32_t progress = (sweepPhase - (sweepDuration / 2)) * (256 + sweepWidth) / sweepDuration;

    // Offset of pixels along direction of sweep
    int32_t cx = progress * dx;
    int32_t cy = progress * dy;

    // For each LED, calculate intensity according to its distance from the sweep center
    for(auto partIndex = 0; partIndex < sizeof(mikuParts)/sizeof(mikuParts[0]); partIndex++)
    {
        auto part = mikuParts[partIndex];
        for(auto idx = part.index; idx < part.index + part.length; idx++)
        {
            // Get the LED's position
            auto pos = PixelPositions[idx];
            
            // Calculate distance from sweep center line
            // Vector from line center to point
            int32_t vx = (int32_t)pos.x - cx;
            int32_t vy = (int32_t)pos.y - cy;

            int32_t dist = vx * dx + vy * dy;
            if (dist < 0)
                dist = -dist;

            // If LED is within sweep width, brighten it
            if (dist < sweepWidth) {
                // Calculate intensity based on distance from center of sweep
                uint8_t intensity = 128 - (dist * 128 / sweepWidth);
                
                frame.SetPixel(idx, part.colour.fade(127 + intensity).gammaCorrected());
            }
            else
                frame.SetPixel(idx, part.colour.fade(128).gammaCorrected());
        }
    }

    return 1000 / 60;  // Run at 60fps for smooth animation
}