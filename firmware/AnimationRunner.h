#pragma once

#include <memory>
#include "pico/sem.h"
#include "NeoPixelBuffer.h"
#include "IAnimation.h"

class IAnimation;

class AnimationRunner
{
public:
    AnimationRunner(std::unique_ptr<NeoPixelBuffer> pixels) :
        _pixels(std::move(pixels)),
        _stopRequested(false)
    {
        _lockNum = spin_lock_claim_unused(true);
        _setLock = spin_lock_init(_lockNum);
    }

    void Start();

    void Shutdown();

    void SetAnimation(std::shared_ptr<IAnimation> animation);

    AnimationRunner(const AnimationRunner&) = delete;
    AnimationRunner& operator=(const AnimationRunner&) = delete;
private:

    void Worker();


    std::unique_ptr<NeoPixelBuffer> _pixels;

    std::shared_ptr<IAnimation> _newAnimation;

    int _lockNum;
    spin_lock_t *_setLock;
    volatile bool _stopRequested = false;

};

