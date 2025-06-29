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

        auto frameTime = miku_all_parts(frameCounter, &frameData); //miku_part_cycle(frameCounter, &frameData);
        sleep_until(delayed_by_ms(frameStart, frameTime));
        frameCounter++;
    }

    return 0;
}
