#pragma once

#include <tuple>

// HA messages work better with HSB, as RGB is treated like a hue scaled to full brightness
std::tuple<int,int,int> HSBtoRGB(float hue, float sat, int brightness);

// Convert RGB -> HSB
std::tuple<float,float,int> RGBtoHSB(int red, int green, int blue);
