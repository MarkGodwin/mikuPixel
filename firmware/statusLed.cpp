// Copyright (c) 2023 Mark Godwin.
// SPDX-License-Identifier: MIT

#include "debug.h"
#include <stdio.h>
#include "statusLed.h"
#include "hardware/pwm.h"
#include "scheduler.h"


StatusLed::StatusLed(int pin)
:   _pin(pin),
    _pulseTimer([this]() { return DoPulse(); }, 0),
    _mode(0)
{
}

void StatusLed::SwitchMode(uint mode)
{
    if(mode == _mode)
        return;
    if(_mode == 0)
    {
        uint slice = pwm_gpio_to_slice_num(_pin);

        pwm_set_wrap(slice, 4096);
        pwm_set_enabled(slice, false);
    }
    else if(mode == 1)
    {
        gpio_set_function(_pin, GPIO_FUNC_PWM);
        uint slice = pwm_gpio_to_slice_num(_pin);

        pwm_set_wrap(slice, 4096);
        pwm_set_enabled(slice, true);
        
    }
    _mode = mode;
}

void StatusLed::TurnOn()
{
    SetLevel(1024);
}

void StatusLed::TurnOff()
{
    SwitchMode(0);
    _pulseTimer.ResetTimer(0);
}

void StatusLed::SetLevel(uint16_t level)
{
    if(level == 0)
    {
        SwitchMode(0);
    }
    else
    {
        pwm_set_gpio_level(_pin, level);
        SwitchMode(1);
    }
}

void StatusLed::Pulse(int minPulse, int maxPulse, int pulseSpeed)
{
    _currentPulse = minPulse;
    _minPulse = minPulse;
    _maxPulse = maxPulse;
    _pulseSpeed = pulseSpeed;
    SetLevel(minPulse);
    _pulseTimer.ResetTimer(100);
}

uint32_t StatusLed::DoPulse()
{
    _currentPulse += _pulseSpeed;
    if(_currentPulse <= _minPulse)
    {
        _currentPulse = _minPulse;
        _pulseSpeed = -_pulseSpeed;
    }
    else if(_currentPulse >= _maxPulse)
    {
        _currentPulse = _maxPulse;
        _pulseSpeed = -_pulseSpeed;
    }
    SetLevel(_currentPulse);

    return 100;
}