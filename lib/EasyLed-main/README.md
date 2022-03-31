# EasyLed  

[![GitHub release](https://img.shields.io/github/release/lnlp/EasyLed.svg)](https://github.com/lnlp/EasyLed/releases/latest) [![GitHub commits](https://img.shields.io/github/commits-since/lnlp/EasyLed/v1.1.0.svg)](https://github.com/lnlp/EasyLed/compare/v1.1.0...main)

Arduino library for controlling standard LEDs in an easy way.

EasyLed provides simple logical methods like `on()`, `off()`, `toggle()`, `flash()`, `isOn()`, `isOff()` and `reset()`.<br>
This library will help to write cleaner code that is easy to read and understand.

A LED can be switched On by simply using the statement **`led.on()`**. When someone reads that statement it is instantly clear what it does: switch the LED On. With EasyLed there is no need to use `pinMode()`, `digitalWrite()`, `HIGH` and `LOW` anymore.

Without EasyLed you will have to use a statement like `digitalWrite(LED_PIN, LOW)` to switch a LED On (if active-low). When reading that statement it does not become clear if it will switch the LED On or Off. This makes it more difficult to read and understand the code. You need to remember whether `HIGH` or `LOW` means the LED is On or Off. When both active-low *and* active-high LEDs are used that will make the code even more difficult to read and understand.

LEDs are defined as instance of the `EasyLed` class. The constructor needs two parameters: `pin`, which is the GPIO pin the LED is connected to and `activeLevel` which indicates whether the LED is active-low or active-high. Enum value `EasyLed::ActiveLevel::Low` represents active-low and `EasyLed::ActiveLevel::High` represents active-high.

For On and Off states similar enum values are used: `EasyLed::State::On` means the LED is On and `EasyLed::State::Off` means it is Off. Enum values are used for state because neither `LOW` nor `HIGH` uniquely identify if a LED is On or Off.

Optionally a third parameter `initialState` can be specified to set the initial state to On (default is Off).
The fourth parameter `pinmode` is also optional and sets the pin as output. This should normally not be changed
_(except when there is reason to change it to OUTPUT_OPEN_DRAIN for architectures that support it)_.

#### Active-low and active-high explained  
LEDs are switched On and Off by programming the state of a GPIO pin. Setting the GPIO pin to a LOW level can turn a LED either On or Off. If the LED is On when the GPIO pin level is LOW this is called active-low. If the LED is On when the pin level is HIGH this is called active-high. Whether a LED is active-low or active-high depends on how the LED is physically connected. Both types are used in practice.

### Constructor

```cpp
EasyLed(
    const uint8_t pin,                      // GPIO pin to which the LED is connected.
    const ActiveLevel activeLevel,          // The logic level when the LED is on.
    const State initialState = State::Off,  // Optional, default is off.
    const uint8_t pinmode = OUTPUT          // Optional, shall normally not be changed.      
```   


### Enumeration types

```cpp
enum class State {Off, On};
enum class ActiveLevel {Low = LOW, High = HIGH};
```


### Functions

```cpp
void on()                                   // Switch LED on

void off()                                  // Switch LED off

void toggle()                               // Toggle LED state.

void reset()                                // Reset LED state to initialState.

void flash(                                 // Flash LED (all parameters are optional)
    const uint8_t  count = 2,               // Number of flashes
    const uint16_t onTimeMs = 160,          // on-time duration in milliseconds
    const uint16_t offTimeMs = 160,         // off-time duration in milliseconds
    const uint16_t leadOffTimeMs = 200,     // off-time duration before first flash
    const uint16_t trailOffTimeMs = 300     // off-time duration after last flash
                                            // lead and trail time are only used when LED is on
)

bool isOn()                                 // Returns true if LED is on, false otherwise

bool isOff()                                // Returns true if LED is off, false otherwise

EasyLed::State getState()                   // Returns the current state (On or Off)

void setState(EasyLed::State state)         // Sets the current state

uint8_t pin()                               // Returns GPIO pin number as specified in constructor

EasyLed::ActiveLevel activeLevel()          // Returns active-level as specified in constructor

EasyLed::State initialState()               // Returns initialState as specified in constructor
```

### Included examples

- `led-basics.ino` - Demonstrates basic use
- `led-advanced.ino` - Demonstrates more advanced use
- `led-state.ino` - Demonstrates how to save and restore state
- `led-activelevel-tester.ino` - Helps determine if LED is active-low or active-high


### Simple code example

```cpp
#include "EasyLed.h"

EasyLed led(LED_BUILTIN, EasyLed::ActiveLevel::Low);

void setup() 
{
    ...
}  

void loop() 
{ 
    readSensors();

    led.on();
    transmitSensorData();
    led.off();

    ...
}
```
