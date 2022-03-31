/*******************************************************************************  
 * 
 *  File:         led-advanced.ino
 * 
 *  Description:  EasyLed example that demonstrates multiple functions. 
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

// If unclear if your LED is active-low or active high, then first try the example
// led-activelevel-tester.ino to find out.

// Set initial state to on (default is off);
EasyLed led(LED_BUILTIN, EasyLed::ActiveLevel::Low, EasyLed::State::On);  //Use this for an active-low LED
// EasyLed led(LED_BUILTIN, EasyLed::ActiveLevel::High, EasyLed::State::On);  //Use this for an active-high LED

int period = 5000;  // Delay period in milliseconds


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


void setup() 
{
    Serial.begin(115200);
    Serial.println("\n\nled-advanced started");
    Serial.println("\nLED should currently be On");
    Serial.println("If Off, change activeLevel in the constructor");
    delay(period);
    Serial.println("\nrunning:");    
    printLedInfo();
    delay(period);   
}


void loop() 
{
    Serial.println("toggle");
    led.toggle();
    delay(period);

    // Default (double) flash while LED is off.
    Serial.println("flash");
    led.flash();
    delay(period);  

    // 3 standard flashes while LED is off.
    Serial.println("flash 3");
    led.flash(3);
    delay(period);  

    // 4 custom (longer) flashes while LED is off.
    Serial.println("flash 4, 400, 200");
    led.flash(4, 400, 200);   
    delay(period);

    Serial.println("on");
    led.on();
    delay(period);

    // Default (double) flash while LED is on.
    Serial.println("flash");
    led.flash();
    delay(period);

    // 3 custom flashes with custom lead off and trail off times while LED is on.
    Serial.println("flash 3, 400, 200, 300, 300");
    led.flash(3, 400, 200, 300, 300);   
    delay(period);

    Serial.println("\nrepeating:");  
}