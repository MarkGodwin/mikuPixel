
#include "mikuPixel.h"
#include "ColourUtils.h"
#include <cmath>
#include <algorithm>

// HA messages work better with HSB, as RGB is treated like a hue scaled to full brightness
std::tuple<int,int,int> HSBtoRGB(float hue, float sat, int brightness)
{
    float s = std::clamp(sat / 100.0f, 0.0f, 1.0f);
    float v = std::clamp(brightness / 255.0f, 0.0f, 1.0f);

    float C = v * s;                           // chroma
    float X = C * (1 - fabsf(fmodf(hue / 60.0f, 2.0f) - 1));
    float m = v - C;

    float r=0, g=0, b=0;
    if(hue < 60)      { r = C; g = X; b = 0; }
    else if(hue < 120){ r = X; g = C; b = 0; }
    else if(hue < 180){ r = 0; g = C; b = X; }
    else if(hue < 240){ r = 0; g = X; b = C; }
    else if(hue < 300){ r = X; g = 0; b = C; }
    else            { r = C; g = 0; b = X; }

    int R = static_cast<int>(std::round((r + m) * 255));
    int G = static_cast<int>(std::round((g + m) * 255));
    int B_out = static_cast<int>(std::round((b + m) * 255));

    return {R, G, B_out};
}

// Convert RGB -> HSB
std::tuple<float,float,int> RGBtoHSB(int red, int green, int blue)
{
    float r = red / 255.0f;
    float g = green / 255.0f;
    float b = blue / 255.0f;

    float maxC = std::max({r,g,b});
    float minC = std::min({r,g,b});
    float delta = maxC - minC;

    float hue = 0.0f;
    if(delta != 0.0f) {
        if(maxC == r)      hue = 60.0f * fmodf(((g - b) / delta), 6.0f);
        else if(maxC == g) hue = 60.0f * (((b - r) / delta) + 2.0f);
        else if(maxC == b) hue = 60.0f * (((r - g) / delta) + 4.0f);
    }
    if(hue < 0) hue += 360.0f;

    float sat = (maxC == 0 ? 0 : delta / maxC);
    float brightness = maxC;

    return {hue, sat * 100.0f, static_cast<int>(std::round(brightness * 255))};
}