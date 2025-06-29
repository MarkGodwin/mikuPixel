#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/sem.h"
#include "hardware/dma.h"
#include "neopixel.pio.h"
#include <string.h>

#include "NeoPixelBuffer.h"
#include <stdlib.h>

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

#define DMA_CHANNEL 0
#define PIXEL_PIN 2

struct PositionMapping
{
    uint8_t x;
    uint8_t y;
};

PositionMapping PixelPositions[] = {
    { 101, 42 }, 
{ 101, 45 }, 
{ 101, 48 }, 
{ 100, 51 }, 
{ 100, 54 }, 
{ 99, 57 }, 
{ 99, 60 }, 
{ 98, 63 }, 
{ 97, 66 }, 
{ 96, 69 }, 
{ 95, 72 }, 
{ 95, 74 }, 
{ 96, 95 }, 
{ 97, 97 }, 
{ 97, 100 }, 
{ 96, 102 }, 
{ 95, 105 }, 
{ 93, 108 }, 
{ 92, 110 }, 
{ 90, 112 }, 
{ 88, 113 }, 
{ 86, 113 }, 
{ 85, 110 }, 
{ 86, 107 }, 
{ 88, 105 }, 
{ 89, 103 }, 
{ 90, 100 }, 
{ 91, 98 }, 
{ 98, 110 }, 
{ 101, 109 }, 
{ 103, 107 }, 
{ 104, 104 }, 
{ 105, 101 }, 
{ 106, 99 }, 
{ 107, 95 }, 
{ 108, 93 }, 
{ 108, 90 }, 
{ 109, 86 }, 
{ 110, 84 }, 
{ 111, 81 }, 
{ 112, 78 }, 
{ 113, 75 }, 
{ 114, 72 }, 
{ 116, 69 }, 
{ 118, 67 }, 
{ 119, 64 }, 
{ 120, 61 }, 
{ 122, 59 }, 
{ 124, 56 }, 
{ 125, 53 }, 
{ 126, 50 }, 
{ 127, 47 }, 
{ 127, 44 }, 
{ 126, 41 }, 
{ 124, 40 }, 
{ 121, 40 }, 
{ 119, 43 }, 
{ 117, 45 }, 
{ 116, 46 }, 
{ 113, 47 }, 
{ 111, 45 }, 
{ 110, 43 }, 
{ 110, 41 }, 
{ 110, 38 }, 
{ 110, 35 }, 
{ 111, 32 }, 
{ 110, 28 }, 
{ 109, 25 }, 
{ 107, 24 }, 
{ 105, 24 }, 
{ 82, 35 }, 
{ 84, 33 }, 
{ 86, 31 }, 
{ 88, 29 }, 
{ 90, 26 }, 
{ 93, 25 }, 
{ 96, 25 }, 
{ 98, 27 }, 
{ 100, 29 }, 
{ 101, 32 }, 
{ 100, 35 }, 
{ 98, 37 }, 
{ 95, 38 }, 
{ 93, 40 }, 
{ 90, 41 }, 
{ 88, 43 }, 
{ 85, 46 }, 
{ 83, 48 }, 
{ 81, 50 }, 
{ 79, 52 }, 
{ 77, 52 }, 
{ 75, 50 }, 
{ 76, 48 }, 
{ 76, 45 }, 
{ 77, 43 }, 
{ 77, 39 }, 
{ 78, 36 }, 
{ 79, 33 }, 
{ 81, 30 }, 
{ 83, 27 }, 
{ 84, 25 }, 
{ 85, 22 }, 
{ 85, 19 }, 
{ 84, 17 }, 
{ 82, 15 }, 
{ 79, 14 }, 
{ 76, 13 }, 
{ 73, 13 }, 
{ 70, 15 }, 
{ 68, 16 }, 
{ 66, 18 }, 
{ 64, 18 }, 
{ 62, 16 }, 
{ 60, 14 }, 
{ 57, 13 }, 
{ 55, 13 }, 
{ 52, 14 }, 
{ 49, 15 }, 
{ 47, 16 }, 
{ 45, 18 }, 
{ 45, 21 }, 
{ 45, 24 }, 
{ 47, 26 }, 
{ 48, 29 }, 
{ 50, 32 }, 
{ 51, 35 }, 
{ 51, 38 }, 
{ 52, 41 }, 
{ 52, 44 }, 
{ 53, 47 }, 
{ 53, 50 }, 
{ 52, 52 }, 
{ 49, 51 }, 
{ 47, 49 }, 
{ 45, 47 }, 
{ 43, 45 }, 
{ 41, 42 }, 
{ 39, 40 }, 
{ 36, 39 }, 
{ 34, 37 }, 
{ 32, 35 }, 
{ 29, 33 }, 
{ 29, 30 }, 
{ 30, 28 }, 
{ 31, 26 }, 
{ 34, 24 }, 
{ 37, 23 }, 
{ 40, 25 }, 
{ 41, 27 }, 
{ 43, 29 }, 
{ 45, 32 }, 
{ 47, 34 }, 
{ 60, 52 }, 
{ 61, 50 }, 
{ 62, 47 }, 
{ 62, 44 }, 
{ 62, 41 }, 
{ 61, 38 }, 
{ 60, 35 }, 
{ 60, 32 }, 
{ 61, 29 }, 
{ 62, 27 }, 
{ 65, 26 }, 
{ 68, 27 }, 
{ 69, 30 }, 
{ 69, 33 }, 
{ 68, 36 }, 
{ 68, 39 }, 
{ 67, 42 }, 
{ 66, 45 }, 
{ 67, 48 }, 
{ 67, 50 }, 
{ 69, 53 }, 
{ 83, 67 }, 
{ 82, 66 }, 
{ 80, 63 }, 
{ 78, 61 }, 
{ 75, 60 }, 
{ 72, 59 }, 
{ 69, 59 }, 
{ 65, 58 }, 
{ 63, 58 }, 
{ 60, 58 }, 
{ 56, 59 }, 
{ 53, 59 }, 
{ 51, 60 }, 
{ 48, 62 }, 
{ 46, 64 }, 
{ 45, 66 }, 
{ 29, 94 }, 
{ 28, 97 }, 
{ 29, 99 }, 
{ 29, 102 }, 
{ 30, 105 }, 
{ 32, 107 }, 
{ 33, 110 }, 
{ 35, 112 }, 
{ 38, 113 }, 
{ 40, 112 }, 
{ 40, 109 }, 
{ 39, 106 }, 
{ 38, 104 }, 
{ 36, 102 }, 
{ 35, 99 }, 
{ 41, 72 }, 
{ 39, 70 }, 
{ 37, 72 }, 
{ 35, 74 }, 
{ 34, 77 }, 
{ 34, 80 }, 
{ 34, 83 }, 
{ 34, 86 }, 
{ 34, 89 }, 
{ 35, 92 }, 
{ 36, 95 }, 
{ 37, 97 }, 
{ 39, 100 }, 
{ 40, 102 }, 
{ 42, 105 }, 
{ 44, 106 }, 
{ 47, 109 }, 
{ 49, 110 }, 
{ 52, 111 }, 
{ 55, 112 }, 
{ 57, 113 }, 
{ 61, 114 }, 
{ 64, 114 }, 
{ 68, 114 }, 
{ 70, 113 }, 
{ 73, 113 }, 
{ 76, 111 }, 
{ 79, 110 }, 
{ 81, 109 }, 
{ 84, 106 }, 
{ 85, 104 }, 
{ 87, 102 }, 
{ 89, 99 }, 
{ 90, 96 }, 
{ 91, 93 }, 
{ 92, 91 }, 
{ 93, 88 }, 
{ 93, 85 }, 
{ 93, 82 }, 
{ 92, 79 }, 
{ 92, 76 }, 
{ 91, 74 }, 
{ 89, 72 }, 
{ 86, 72 }, 
{ 85, 74 }, 
{ 83, 77 }, 
{ 82, 79 }, 
{ 81, 82 }, 
{ 80, 85 }, 
{ 78, 87 }, 
{ 77, 90 }, 
{ 75, 92 }, 
{ 72, 93 }, 
{ 70, 93 }, 
{ 69, 91 }, 
{ 68, 89 }, 
{ 69, 87 }, 
{ 69, 84 }, 
{ 68, 81 }, 
{ 66, 81 }, 
{ 63, 82 }, 
{ 62, 84 }, 
{ 60, 87 }, 
{ 59, 90 }, 
{ 58, 92 }, 
{ 57, 93 }, 
{ 54, 94 }, 
{ 52, 93 }, 
{ 50, 91 }, 
{ 48, 88 }, 
{ 47, 85 }, 
{ 45, 83 }, 
{ 44, 80 }, 
{ 31, 67 }, 
{ 31, 65 }, 
{ 30, 61 }, 
{ 29, 59 }, 
{ 28, 55 }, 
{ 28, 53 }, 
{ 28, 50 }, 
{ 27, 46 }, 
{ 27, 43 }, 
{ 27, 41 }, 
{ 25, 23 }, 
{ 22, 23 }, 
{ 20, 24 }, 
{ 18, 27 }, 
{ 17, 30 }, 
{ 17, 33 }, 
{ 17, 36 }, 
{ 16, 39 }, 
{ 15, 41 }, 
{ 13, 42 }, 
{ 10, 41 }, 
{ 9, 39 }, 
{ 7, 37 }, 
{ 5, 35 }, 
{ 3, 35 }, 
{ 1, 37 }, 
{ 0, 40 }, 
{ 1, 43 }, 
{ 1, 47 }, 
{ 2, 49 }, 
{ 4, 52 }, 
{ 5, 55 }, 
{ 6, 58 }, 
{ 8, 60 }, 
{ 9, 63 }, 
{ 11, 66 }, 
{ 12, 68 }, 
{ 13, 71 }, 
{ 14, 74 }, 
{ 15, 77 }, 
{ 16, 80 }, 
{ 17, 83 }, 
{ 17, 86 }, 
{ 18, 89 }, 
{ 18, 91 }, 
{ 19, 95 }, 
{ 20, 97 }, 
{ 20, 100 }, 
{ 22, 103 }, 
{ 23, 106 }, 
{ 26, 108 }, 
{ 27, 109 }, 
};

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

static uint32_t miku_all_parts(uint32_t frame, NeoPixelFrame *neopixels)
{

    for(auto partIndex = 0; partIndex < sizeof(mikuParts)/sizeof(mikuParts[0]); partIndex++)
    {
        auto part = mikuParts[partIndex];
        for(auto idx = part.index; idx < part.index + part.length; idx++)
        {
            neopixels->SetPixel(idx, part.colour.fade(128).gammaCorrected() );
        }
    }

    return 1000;
}


static uint32_t miku_part_cycle(uint32_t frame, NeoPixelFrame *neopixels)
{
    memset(neopixels->GetBuffer(), 0, sizeof(neopixel) * PIXEL_COUNT);

    int fade = frame % 256;
    int partIndex = (frame / 256) % 10;
    auto part = mikuParts[partIndex];

    // Fading out
    for(auto idx = part.index; idx < part.index + part.length; idx++)
    {
        neopixels->SetPixel(idx, part.colour.fade(255 - fade).gammaCorrected() );
    }

    // Bright bit
    partIndex = (partIndex + 1) % 10;
    part = mikuParts[partIndex];
    for(auto idx = part.index; idx < part.index + part.length; idx++)
    {
        neopixels->SetPixel(idx, part.colour.gammaCorrected());
    }

    // Bright bit
    partIndex = (partIndex + 1) % 10;
    part = mikuParts[partIndex];
    for(auto idx = part.index; idx < part.index + part.length; idx++)
    {
        neopixels->SetPixel(idx, part.colour.gammaCorrected());
    }

    // Fading in
    partIndex = (partIndex + 1) % 10;
    part = mikuParts[partIndex];
    for(auto idx = part.index; idx < part.index + part.length; idx++)
    {
        neopixels->SetPixel(idx, part.colour.fade(fade).gammaCorrected());
    }

    return 0;
}

static uint32_t ring_spin(uint32_t frame, NeoPixelFrame *neopixels)
{
    static uint8_t intensity = 255;
    static uint8_t colIndex = 0;

    memset(neopixels->GetBuffer(), 0, sizeof(neopixel) * PIXEL_COUNT);

    neopixel col{0};

    auto idx = frame % PIXEL_COUNT;

    neopixels->SetPixel(idx, 0xFFFFFF00);
    neopixels->SetPixel((idx + 50) % PIXEL_COUNT, 0x0000FF00);
    neopixels->SetPixel((idx + 100) % PIXEL_COUNT, 0xFF000000);
    neopixels->SetPixel((idx + 150) % PIXEL_COUNT, 0x00FF0000);
    neopixels->SetPixel((idx + 200) % PIXEL_COUNT, 0x0000FF00);
    neopixels->SetPixel((idx + 250) % PIXEL_COUNT, 0xFF000000);
    neopixels->SetPixel((idx + 300) % PIXEL_COUNT, 0x00FF0000);

    return 500;
}

uint32_t blurDrops(uint32_t frame, NeoPixelFrame *neopixels)
{
    auto input = neopixels->GetLastBuffer();
    auto output = neopixels->GetBuffer();


    for(auto x = 0; x < PIXEL_COUNT; x++)
    {
        auto left = (x - 1) % PIXEL_COUNT;
        auto right = (x + 1) % PIXEL_COUNT;
        output[x].red = ((int)input[left].red + input[x].red + input[x].red + input[right].red) / 4;
        output[x].green = ((int)input[left].green + input[x].green + input[x].green + input[right].green) / 4;
        output[x].blue = ((int)input[left].blue + input[x].blue + input[x].blue + input[right].blue) / 4;
        /*if(output[x].red < 1)
            output[x].red = 0;
        else
            output[x].red -= 1;
        if(output[x].green < 1)
            output[x].green = 0;
        else
            output[x].green -= 1;
        if(output[x].blue < 1)
            output[x].blue = 0;
        else
            output[x].blue -= 1;*/

        if((rand() & 1023) == 100)
        {
            //puts("drip");
            output[x].colour = 0;
            switch(rand() % 3)
            {
                case 0:
                    output[x].red = 127;
                    break;
                case 1:
                    output[x].green = 127;
                    break;
                case 2:
                    output[x].blue = 127;
                    output[x].red = 127;
                    output[x].green = 127;
                    break;
            }
            output[x].red += rand() & 127;
            output[x].green += rand() & 127;
            output[x].blue += rand() & 127;           
        }        
    }



    return 10;
}

neopixel _pattern[127][127];

uint32_t pattern_scroll(uint32_t frame, NeoPixelFrame *neopixels)
{
    const neopixel magenta(200, 0, 200);
    const neopixel cyan(0, 200, 200);

    auto offset = frame & 31;


    if(frame & 0x800)
    {
        for(auto y = 0; y < 127; y++)
        {
            for(auto x = 0; x < 127; x++)
            {
                _pattern[y][x] = ((y + offset) & 0x10) ? magenta : cyan;
            }
        }
    }
    else
    {
        for(auto y = 0; y < 127; y++)
        {
            for(auto x = 0; x < 127; x++)
            {
                _pattern[x][y] = ((y + offset) & 0x10) ? magenta : cyan;
            }
        }
    }

    for(auto i = 0; i < PIXEL_COUNT; i++)
    {
        neopixels->SetPixel(i, _pattern[PixelPositions[i].y][PixelPositions[i].x]);
    }

    return 10;
}


uint32_t location_registration(uint32_t frame, NeoPixelFrame *neopixels)
{
    memset(neopixels->GetBuffer(), 0, sizeof(neopixel) * PIXEL_COUNT);
    int currentPixel = (frame) % PIXEL_COUNT;
    neopixels->SetPixel(currentPixel, neopixel(255, 255, 255));

    return 500;
}


int main()
{
    //stdio_init_all();

    //sleep_ms(8000);

    //puts("Hello, world!");

    NeoPixelBuffer neopixels(DMA_CHANNEL, DMA_IRQ_0, pio0, 0, PIXEL_PIN, PIXEL_COUNT);

    //puts("Program Init");

    uint32_t frameCounter = 0;
    while(true)
    {
        auto frameStart = get_absolute_time();
        auto frameData = neopixels.Swap();

        auto frameTime = pattern_scroll(frameCounter, &frameData); //  miku_all_parts(frameCounter, &frameData); //miku_part_cycle(frameCounter, &frameData);
        sleep_until(delayed_by_ms(frameStart, frameTime));
        frameCounter++;
    }

    return 0;
}
