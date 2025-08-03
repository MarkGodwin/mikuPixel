#pragma once

#include "pico/stdlib.h"
#include "pico/sem.h"
#include <string.h>
#include "hardware/dma.h"
#include "hardware/pio.h"
#include <array>
#include <vector>
#include <cmath>
#include "NeoPixel.h"


class NeoPixelFrame
{
    public:
        NeoPixelFrame(neopixel *backBuffer, const neopixel *frontBuffer, uint32_t pixelCount) :
        _backBuffer(backBuffer),
        _frontBuffer(frontBuffer),
        _pixelCount(pixelCount)
        {
        }

        void SetPixel(uint32_t x, uint32_t colour)
        {
            _backBuffer[x].colour = colour;
        }

        void SetPixel(uint32_t x, neopixel colour)
        {
            _backBuffer[x] = colour;
        }

        neopixel *GetBuffer() {
            return _backBuffer;
        }

        const neopixel *GetLastBuffer() {
            return _frontBuffer;
        }

        uint32_t GetPixelCount() const {
            return _pixelCount;
        }

        void Clear()
        {
            ::memset(_backBuffer, 0, sizeof(neopixel) * _pixelCount);
        }

    private:
        neopixel *_backBuffer;
        const neopixel *_frontBuffer;
        uint32_t _pixelCount;
};

class NeoPixelBuffer
{
    public:
        NeoPixelBuffer(uint32_t dmaChannel, uint32_t irq, PIO pio, uint32_t stateMachine, uint32_t pixelPin, uint32_t pixelCount);
        ~NeoPixelBuffer();

        NeoPixelFrame Swap()
        {
            _frontBuffer.swap(_backBuffer);

            sem_acquire_blocking(&_swapReady);
            dma_channel_set_read_addr(_dmaChannel, _frontBuffer.data(), true);
            return NeoPixelFrame(_backBuffer.data(), _frontBuffer.data(), _pixelCount);
        }


    private:

        static void __isr Dma0CompleteHandler();
        static void __isr Dma1CompleteHandler();

        static int64_t LatchDelayCompleteEntry(alarm_id_t id, void *userData);
        int64_t LatchDelayComplete();

        uint32_t _dmaChannel;
        uint32_t _dmaMask;
        uint32_t _irq;
        PIO _pio;
        uint32_t _stateMachine;
        uint32_t _programOffset;

        semaphore _swapReady;

        uint32_t _pixelCount;
        std::vector<neopixel> _frontBuffer;
        std::vector<neopixel> _backBuffer;
};
