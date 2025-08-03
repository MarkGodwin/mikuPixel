
#include "AnimationRunner.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/flash.h"
#include "debug.h"

class SpinLock
{
public:
    SpinLock(spin_lock_t *lock): _lock(lock)
    {
        _irq = spin_lock_blocking(_lock);
    }
    ~SpinLock()
    {
        spin_unlock(_lock, _irq);
    }

private:
    spin_lock_t *_lock;
    int32_t _irq;
};

static AnimationRunner *_theRunner = nullptr;

void AnimationRunner::SetAnimation(std::shared_ptr<IAnimation> animation)
{
    SpinLock lock(_setLock);
    __compiler_memory_barrier();
    if(_stopRequested)
        return;
    _newAnimation = std::move(animation);
}

void AnimationRunner::Shutdown()
{
    SpinLock lock(_setLock);
    __compiler_memory_barrier();
    _stopRequested = true;
}

void AnimationRunner::Start()
{
    _theRunner = this;
    multicore_launch_core1([]() {
        // Make sure the other core can fiddle with flash without us screwing everything up
        if(!flash_safe_execute_core_init())
        {
            // Unsafe to continue this thread...
            DBG_PUT("Flash init failed");
            return;
        }

        _theRunner->Worker();

        // Don't actually end the command thread, as we want to write to flash
        // and if we stop this thread, the flash write may fail because it can't
        // synchronise
        while(true)
            sleep_ms(1000);

        //flash_safe_execute_core_deinit();
    });
}

void AnimationRunner::Worker()
{
    uint32_t frameCounter = 0;
    uint32_t lastFrameDelay = 0;

    std::shared_ptr<IAnimation> currentAnimation;

    while(true)
    {
        {
            SpinLock lock(_setLock);
            __compiler_memory_barrier();
            if(_stopRequested)
                break;

            if(_newAnimation)
            {
                currentAnimation = std::move(_newAnimation);
                frameCounter = 0;
                lastFrameDelay = 0;
            }
            __compiler_memory_barrier();
        }

        auto frameStart = get_absolute_time();

        // Show the previous frame
        auto frame = _pixels->Swap(); 
        // Draw the current animation frame
        auto frameDelay = currentAnimation ? currentAnimation->DrawFrame(frame, frameCounter) : 1000;
        frameCounter++;
        // Wait until the previous frame has been shown for the required time
        sleep_until(delayed_by_ms(frameStart, lastFrameDelay));
        lastFrameDelay = frameDelay;
    }
}
