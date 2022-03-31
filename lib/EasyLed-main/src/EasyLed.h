/*******************************************************************************  
 *
 *  File:         EasyLed.h
 * 
 *  Description:  Header file for EasyLed library.
 * 
 *  Author:       Leonel Lopes Parente 
 * 
 *  Copyright:    Copyright (c) 2019-2021 Leonel Lopes Parente
 *
 *                Permission is hereby granted, free of charge, to anyone 
 *                obtaining a copy of this document and accompanying files to do, 
 *                whatever they want with them without any restriction, including,
 *                but not limited to, copying, modification and redistribution.
 *                The above copyright notice and this permission notice shall be 
 *                included in all copies or substantial portions of the Software.
 * 
 *                The software is provided "as is", without any warranty.
 * 
 *  License:      MIT License. See included LICENSE file.
 * 
 ******************************************************************************/

#pragma once

#ifndef EASY_LED_H_
#define EASY_LED_H_

#include <Arduino.h>

class EasyLed 
{
public:
    enum class State {Off, On};
    enum class ActiveLevel {Low = LOW, High = HIGH};

	EasyLed(
        const uint8_t pin,                      // GPIO pin to which the LED is connected.
        const ActiveLevel activeLevel,          // The logic level when the LED is on.
        const State initialState = State::Off,  // Default is off.
        const uint8_t pinmode = OUTPUT          // Default is OUTPUT, some Arduino cores also
    );                                          // support OUTPUT_OPEN_DRAIN which can be
                                                // optionally specified here.            

	State getState() const;      
    void setState(const State state);

    bool isOn() const;
    bool isOff() const;
    uint8_t pin() const;
    ActiveLevel activeLevel() const;
    State initialState() const;

    void on();
    void off();
    void toggle();
    void reset();
    void flash(
        const uint8_t  count = 2, 
        const uint16_t onTimeMs = 160, 
        const uint16_t offTimeMs = 160, 
        const uint16_t leadOffTimeMs = 200, 
        const uint16_t trailOffTimeMs = 300
    );

private:
    uint8_t pin_;
    ActiveLevel activeLevel_;
	State state_;
    State initialState_;
    
    EasyLed();                                  // Disable parameterless public constructor.
    void forceState(const State state);      
};    

#endif  // EASY_LED_H_