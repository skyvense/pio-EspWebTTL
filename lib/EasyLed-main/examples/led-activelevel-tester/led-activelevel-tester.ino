/*******************************************************************************  
 * 
 *  File:         led-activelevel-tester.ino
 * 
 *  Description:  EasyLed example to help determine if a LED is active-low 
 *                or active-high. 
 * 
 *                When active-low the LED is on when the pin level is low,
 *                when active-high the LED is on when the pin level is high. 
 *                The activelevel depends on hardware and board used.
 *                The correct value for activelevel must be specified in 
 *                the constructor. If incorrect, on and off will be reversed.
 * 
 *  Author:       Leonel Lopes Parente
 * 
 *  Copyright:    Copyright (c) 2019-2021 Leonel Lopes Parente
 * 
 *  License:      MIT License. See included LICENSE file.
 * 
 ******************************************************************************/

#include "EasyLed.h"

// It is assumed that your board has a builtin LED connected to pin LED_BUILTIN.
// If not, replace LED_BUILTIN with the correct pin number and/or connect an external LED.

EasyLed led(LED_BUILTIN, EasyLed::ActiveLevel::Low, EasyLed::State::Off);  //Use this for an active-low LED
// EasyLed led(LED_BUILTIN, EasyLed::ActiveLevel::High, EasyLed::State::Off);  //Use this for an active-high LED

int period = 8000;  // Delay period in milliseconds


void printLedInfo()
{
    Serial.println("\nObtained information:");     
    Serial.print("LED is currently ");  
    Serial.println(led.isOn() ? "On" : "Off");
    Serial.print("Is connected to pin ");
    Serial.println(led.pin());
    Serial.print("Activelevel is active-");
    Serial.println(led.activeLevel() == EasyLed::ActiveLevel::High ? "high" : "low"); 
    Serial.print("Initial state is ");
    Serial.println(led.initialState() == EasyLed::State::On ? "On" : "Off");     
    Serial.println(); 
}


String getString(EasyLed::ActiveLevel level)
{
    return (level == EasyLed::ActiveLevel::High ? String("high") : String("low"));
}

String getString(EasyLed::State state)
{
    return (state == EasyLed::State::On ? String("On") : String("Off"));
}

EasyLed::ActiveLevel getInverse(EasyLed::ActiveLevel level)
{
    return (level == EasyLed::ActiveLevel::High ? EasyLed::ActiveLevel::Low : EasyLed::ActiveLevel::High);
}

EasyLed::State getInverse(EasyLed::State state)
{
    return (state == EasyLed::State::On ? EasyLed::State::Off : EasyLed::State::On);
}


void setup() 
{
    Serial.begin(115200);
    Serial.println("\n\nled-activelevel-tester started");
    Serial.println("\nLED should currently be Off");
    delay(period);
    printLedInfo();
    delay(period); 
}


EasyLed::ActiveLevel activeLevel = led.activeLevel();

void loop() 
{ 
    EasyLed::State targetState = getInverse(led.getState());

    Serial.print("\nSwitching LED ");        
    Serial.println(getString(targetState));
    led.toggle();

    Serial.print("\nIf the LED is ");
    Serial.print(getString(targetState));
    Serial.print(" then the LED is active-");
    Serial.println(getString(activeLevel));
    Serial.println("The activeLevel parameter for the constructor is correct");
    
    Serial.print("\nIf the LED is ");
    Serial.print(getString(getInverse(targetState)));    
    Serial.print(" then the LED is active-");
    Serial.println(getString(getInverse(activeLevel)));
    Serial.print("The activeLevel parameter for the constructor should be changed to active-"); 
    Serial.println(getString(getInverse(activeLevel)));
    Serial.println();

    delay(period);
}