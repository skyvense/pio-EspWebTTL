/*******************************************************************************  
 * 
 *  File:         led-basics.ino
 * 
 *  Description:  EasyLed example that demonstrates basic use of the 
 *                on(), off(), toggle() and flash() functions. 
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

EasyLed led(LED_BUILTIN, EasyLed::ActiveLevel::Low);  //Use this for an active-low LED.
// EasyLed led(LED_BUILTIN, EasyLed::ActiveLevel::High);  //Use this for an active-high LED.

int period = 5000;  // Delay period in milliseconds


void setup() 
{
    Serial.begin(115200);
    Serial.println("\n\nled-basics started");
    Serial.println("\nLED should currently be Off.");
    Serial.println("If On, change activeLevel in the constructor.");
    delay(period);
    Serial.println("\nrunning:");
}


void loop() 
{
    Serial.println("on");
    led.on();
    delay(period);

    Serial.println("toggle");
    led.toggle();
    delay(period);

    Serial.println("flash");
    led.flash();
    delay(period);

    Serial.println("on");
    led.on();
    delay(period); 

    Serial.println("flash");
    led.flash();
    delay(period);

    Serial.println("off");
    led.off();
    delay(period);               

    Serial.println("\nrepeating:");    
}