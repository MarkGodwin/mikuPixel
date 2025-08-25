#pragma once

#include "AnimationHelpers.h"
#include "NeoPixelBuffer.h"
#include <optional>

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
extern neopixel GridBuffer[127][127]; // For intermediate drawing before pixel mapping

struct Connection {
    int track;
    bool start; // True if connection is at start of track, false if at end
};

constexpr Connection NullConnection{ -1, false };

struct TrackPart {
    int start;
    int end;
    std::array<Connection, 3> startConnections;
    std::array<Connection, 3> endConnections;

    bool IsAtEnd(int position, int direction) const
    {
        return (direction > 0 && position == end) || (direction < 0 && position == start);
    }
    const Connection* GetConnections(bool atEnd) const
    {
        return atEnd ? endConnections.data() : startConnections.data();
    }
};


inline constexpr auto MikuTracks = std::to_array<TrackPart>({
    TrackPart { 0, 11, { Connection { 8, true }, Connection { 7, false }, NullConnection }, { Connection { 1, false }, NullConnection } },
    TrackPart { 239, 243, { Connection { 2, true },Connection { 5, false }, NullConnection }, { Connection { 0, false },Connection { 17, true }, NullConnection } },
    TrackPart { 12, 17, { Connection { 5, false },Connection { 1, true}, NullConnection }, { Connection { 4, true },Connection { 3, true}, NullConnection } },
    TrackPart { 28, 69, { Connection { 2, false },Connection { 4, true}, NullConnection }, { Connection { 16, false },Connection { 7, true}, NullConnection } },
    TrackPart { 18, 27, { Connection { 2, false },Connection { 3, true}, NullConnection }, { Connection { 5, true },Connection { 6, false}, NullConnection } },
    TrackPart { 237, 238, { Connection { 4, false },Connection { 6, false}, NullConnection }, { Connection { 2, true },Connection { 1, true}, NullConnection } },
    TrackPart { 215, 236, { Connection { 24, false },Connection { 23, false}, NullConnection }, { Connection { 5, true },Connection { 4, false}, NullConnection } },
    TrackPart { 78, 80, { Connection { 3, false },Connection { 16, false}, NullConnection }, { Connection { 0, true },Connection { 8, true}, NullConnection } },
    TrackPart { 81, 90, { Connection { 0, true },Connection { 7, false}, NullConnection }, { Connection { 9, true },Connection { 22, false}, NullConnection } },
    TrackPart { 91, 95, { Connection { 8, false },Connection { 22, false}, NullConnection }, { Connection { 16, true },Connection { 10, true}, NullConnection } },
    TrackPart { 96, 110, { Connection { 9, false}, NullConnection }, { Connection { 10, false },Connection { 20, false}, NullConnection } },
    TrackPart { 111, 125, { Connection { 10, false },Connection { 22, true}, NullConnection }, { Connection { 12, true}, NullConnection } },
    TrackPart { 126, 130, { Connection { 11, false },Connection { 15, false}, NullConnection }, { Connection { 13, true },Connection { 20, true}, NullConnection } },
    TrackPart { 131, 139, { Connection { 12, false },Connection { 20, true}, NullConnection }, { Connection { 14, true },Connection { 29, false}, NullConnection } },
    TrackPart { 140, 143, { Connection { 13, false },Connection { 29, false}, NullConnection }, { Connection { 15, true },Connection { 28, true}, NullConnection } },
    TrackPart { 144, 151, { Connection { 14, false },Connection { 28, true}, NullConnection }, { Connection { 12, true}, NullConnection } },
    TrackPart { 70, 77, { Connection { 9, false}, NullConnection }, { Connection { 3, false },Connection { 7, true}, NullConnection } },
    TrackPart { 244, 246, { Connection { 1, false}, NullConnection }, { Connection { 27, true}, NullConnection } },
    TrackPart { 173, 177, { Connection { 27, true}, NullConnection }, { Connection { 22, false },Connection { 19, true}, NullConnection } },
    TrackPart { 178, 182, { Connection { 18, false}, NullConnection }, { Connection { 21, true}, NullConnection } },
    TrackPart { 152, 162, { Connection { 13, true },Connection { 21, true}, NullConnection }, { Connection { 22, true },Connection { 10, false}, NullConnection } },
    TrackPart { 183, 188, { Connection { 20, true },Connection { 19, false}, NullConnection }, { Connection { 27, false}, NullConnection } },
    TrackPart { 163, 172, { Connection { 11, true },Connection { 20, false}, NullConnection }, { Connection { 8, false },Connection { 18, false}, NullConnection } },
    TrackPart { 213, 214, { Connection { 24, false },Connection { 6, true}, NullConnection }, { Connection { 30, false },Connection { 25, true}, NullConnection } },
    TrackPart { 195, 203, { Connection { 25, false },Connection { 28, false}, NullConnection }, { Connection { 6, true },Connection { 23, false}, NullConnection } },
    TrackPart { 189, 194, { Connection { 30, false },Connection { 23, true}, NullConnection }, { Connection { 28, false },Connection { 24, true}, NullConnection } },
    TrackPart { 204, 207, { Connection { 27, false}, NullConnection }, { Connection { 30, true}, NullConnection } },
    TrackPart { 247, 276, { Connection { 18, true },Connection { 17, false}, NullConnection }, { Connection { 26, true },Connection { 21, false}, NullConnection } },
    TrackPart { 287, 328, { Connection { 14, false },Connection { 15, true}, NullConnection }, { Connection { 25, false },Connection { 24, true}, NullConnection } },
    TrackPart { 277, 286, { Connection { 30, true}, NullConnection }, { Connection { 14, true },Connection { 13, false}, NullConnection } },
    TrackPart { 208, 212, { Connection { 26, false },Connection { 29, true}, NullConnection }, { Connection { 23, true },Connection { 25, true}, NullConnection } },
});

