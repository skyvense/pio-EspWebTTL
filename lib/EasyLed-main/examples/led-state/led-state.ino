/*******************************************************************************  
 * 
 *  File:         led-state.ino
 * 
 *  Description:  EasyLed example that demonstrates how to 
 *                save and restore LED state (whether it is on or off).
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


void doOtherLedStuff()
{
    Serial.println("\ndoing other led stuff:");  

    Serial.println("reset");        
    led.reset();
    delay(period);        
    Serial.println("flash 5");   
    led.flash(5);
}


void setup() 
{
    Serial.begin(115200);
    Serial.println("\n\nled-state started");
    Serial.println("\nLED should currently be Off.");
    Serial.println("If On, change activeLevel in the constructor.");
    delay(period);
    Serial.println("\nrunning:\n");  
    Serial.println("on");        
    led.on();
    delay(period);    
}


void loop() 
{
    // Save current LED state.
    Serial.println("saving led state");  
    EasyLed::State savedState = led.getState();
    delay(period);      

    doOtherLedStuff();
    delay(period);

    // Restore saved state.
    Serial.println("\nrestoring led state");    
    led.setState(savedState);
    delay(period);

    Serial.println("\n\nrepeating:\n");        
}