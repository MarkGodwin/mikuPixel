#pragma once

#include "AnimationHelpers.h"
#include "NeoPixelBuffer.h"

#define PIXEL_COUNT 329

#define RIGHT_HAIR_1 0
#define RIGHT_HAIR1_LEN 12

#define RIGHT_HAIRBAND (RIGHT_HAIR_1 + RIGHT_HAIR1_LEN)
#define RIGHT_HAIRBAND_LEN 16

#define RIGHT_HAIR_2 (RIGHT_HAIRBAND + RIGHT_HAIRBAND_LEN)
#define RIGHT_HAIR_2_LEN 42

#define SHIRT (RIGHT_HAIR_2 + RIGHT_HAIR_2_LEN)
#define SHIRT_LEN 82

#define TIE (SHIRT + SHIRT_LEN)
#define TIE_LEN 21

#define FACE (TIE + TIE_LEN)
#define FACE_LEN 16

#define LEFT_HAIRBAND (FACE + FACE_LEN)
#define LEFT_HAIRBAND_LEN 15

#define HAIR (LEFT_HAIRBAND + LEFT_HAIRBAND_LEN)
#define HAIR_LEN 73

#define LEFT_HAIR_1 (HAIR + HAIR_LEN)
#define LEFT_HAIR_1_LEN 10
#define LEFT_HAIR_2 (LEFT_HAIR_1 + LEFT_HAIR_1_LEN)
#define LEFT_HAIR_2_LEN (PIXEL_COUNT - LEFT_HAIR_2)


struct MikuPart
{
    int index;
    int length;
    neopixel colour;

    constexpr MikuPart(int idx, int len, neopixel col) : index(idx), length(len), colour(col) {}
};

constexpr neopixel HairColour{0, 240, 150};
constexpr neopixel HairbandColour{255, 0, 200};
constexpr neopixel ShirtColour{190, 245, 220};
constexpr neopixel TieColour{0, 255, 100};
constexpr neopixel FaceColour{245, 140, 140};

constexpr MikuPart MikuRightHair{ RIGHT_HAIR_1, RIGHT_HAIR1_LEN, HairColour };
constexpr MikuPart MikuRightHair2{ RIGHT_HAIR_2,  RIGHT_HAIR_2_LEN, HairColour };
constexpr MikuPart MikuRightHairband{ RIGHT_HAIRBAND, RIGHT_HAIRBAND_LEN, HairbandColour };
constexpr MikuPart MikuShirt{ SHIRT, SHIRT_LEN, ShirtColour };
constexpr MikuPart MikuTie{ TIE, TIE_LEN,  TieColour};
constexpr MikuPart MikuFace{ FACE, FACE_LEN, FaceColour};
constexpr MikuPart MikuLeftHairband{ LEFT_HAIRBAND,  LEFT_HAIRBAND_LEN, HairbandColour};
constexpr MikuPart MikuHair{ HAIR, HAIR_LEN, HairColour };
constexpr MikuPart MikuLeftHair{ LEFT_HAIR_1, LEFT_HAIR_1_LEN, HairColour };
constexpr MikuPart MikuLeftHair2{ LEFT_HAIR_2, LEFT_HAIR_2_LEN, HairColour };
constexpr MikuPart NullPart{ -1, 0, neopixel(0) };

constexpr MikuPart mikuParts[] = {
    MikuRightHair2,
    MikuRightHair,
    MikuRightHairband,
    MikuHair,
    MikuLeftHairband,
    MikuLeftHair,
    MikuLeftHair2,
    MikuShirt,
    MikuTie,
    MikuFace,
};

constexpr MikuPart MikuAllHair[] = {
    MikuRightHair,
    MikuRightHair2,
    MikuHair,
    MikuLeftHair,
    MikuLeftHair2,
    NullPart
};

constexpr MikuPart MikuBunches[] = {
    MikuRightHair,
    MikuRightHair2,
    MikuLeftHair,
    MikuLeftHair2,
    NullPart
};

constexpr MikuPart MikuHairBands[] = {
    MikuRightHairband,
    MikuLeftHairband,
    NullPart
};

constexpr MikuPart MikuHead[] = {
    MikuFace,
    MikuHair,
    NullPart
};

constexpr MikuPart MikuShirtAndTie[] = {
    MikuShirt,
    MikuTie,
    NullPart
};

constexpr const MikuPart *AggregatedMikuParts[] = {
    MikuHead,
    MikuHairBands,
    MikuBunches,
    MikuShirtAndTie
};

extern PositionMapping const PixelPositions[PIXEL_COUNT];