<!-- README.md, muman.ch 2026.03.28 -->


# Multichannel Timer Interrupt Library for SAMD21 and SAMD51 processors

Here is yet another timer interrupt library for SAMD21 and SAMD51 processors. But this $${\color{green}mumanchu}$$ library is unique in that it can use one or more Timer/Counters (TC or TCC), and can use the timer's "channels" (CH0..CH5) to produce multiple interrupts at different frequencies, each calling a different interrupt handler (ISR).

This library was orginally developed as part of a non-blocking Stepper Motor Library, which controls up to 4 motors using S-Curve acceleration/deceleration curves (this library will be released soon). The Stepper Motor library has timer support for almost all common microcontrollers/MCUs, via a cross-platform timer class with a common API (Application Programming Interface). A universal timer library for other MCUs will be released soon too.


> [!NOTE]
> This is my first Arduino Library and my first github project. I think this README text is too long. It's aimed at beginners or intermediate makers, which is why it contains so many details. Maybe I'll move most of it to the muman.ch website. I also wanted to use MattLabs (which I have used for my projects, just for fun), but someone else has already used it. So now it's $${\color{green}mumanchu}$$, which comes from the name of my old `muman.ch` website. 


### Advantages of this library

- Uses one or more 'Timer/Counters' (TC0..TC7) or 'Timer/Counters for Control' (TCC0..TCC4). The number of available timer/counters depends on the processor type. But normally you'll only need one.
- Uses a timer's _channels_ (CH0..CH5) to generate multiple interrupts at different frequencies from a single timer, depending on the number of channels the timer has.
- Easy to implement, with just a single `#include` file.
- Very small code size. Only generates code if it's actually needed. Uses `#define` statements to define the code to be generated for a specific timer.
- Optional `DEBUG` mode which adds additional validation and writes error messages to `Serial` to help with implementation and debugging. The debug code can be excluded from the release version once it's been tested.
- Unique `PrintUsedTimers()` function so you can see which timers are already used by existing code and other libraries.


### Table of contents

- [Supported Boards](#supported-boards)
- [Do you really need a hardware interrupt?](#really-need-interrupt)
- [What is a 'Timer/Counter' (TC) and a 'Timer/Counter for Control' (TCC)?](#what-is-tc-tcc)
- [Using the Library](#using-the-library)
- [Example Sketch](#example-sketch)
- [The `#include` Files](#include-files)
- [Using `#defines` to control code generation](#using-defines)
- [The `#define` Symbols](#the-define-symbols)
- [Library API (Application Programming Interface)](#library-api)
- [Debugging Features](#debugging-features)
- [A simple scheduler which doesn't need a timer interrupt](#simple-scheduler)
- [A simple switch debouncer which doesn't need a timer interrupt](#simple-debouncer)
- [Technical Notes (wear your Anorak)](#technical-notes)
	- [Interrupt handler (ISR) restrictions and best practices](#interrupt-restrictions)
	- [How to determine which timers are already used](#which-timers-are-used)
	- [Code in the SAMD's `delay.c`](#delay-dot-c)
	- [Pointers](#pointers)
- [Reference Documents](#reference-documents)
- [New libraries for SAMD21/51, STM32, ESP32 and AVR processors](#coming-soon)
- [Matt's Blog](#matts-blog)
- [License and Disclaimer](#license-and-disclaimer)
- [Revision History](#revision-history)
<br/>


<a name="supported-boards"></a>

## Supported Boards

The library has been tested on these boards, but it should work with all SAMD21/51 boards that use the Arduino SAMD boards package. Note that the SAMD21's have no FPU (no floating point support in hardware).

**Arduino Zero (ATSAMD21G18A)**\
This 48MHz/256KB/32KB board has excellent debugging features with its built-in EDBG debugger. 

**Arduino Nano 33 IoT (ATSAMD21G18A)**\
48MHz/256KB/32KB

**Arduino MKR WiFi 1010 (ATSAMD21G18A)**\
48MHz/256KB/32KB

**Adafruit Metro M4 Express (ATSAMD51J19A)**\
This superbly fast 120MHz/512KB/192KB board also has an FPU, but you can't debug the program unless you have an Atmel-ICE or Segger J-Link. 


<a name="really-need-interrupt"></a>

## Do you really need a hardware timer interrupt?

If all you need is a simple scheduler to run some code at approximate time intervals, then you may not need to use hardware timer interrupts. See the [Simple scheduler](#simple-scheduler) example below.

The same is true for debouncing switches, where a call to `millis()` or `micros()` is usually sufficient. See the [Simple debouncer](#simple-debouncer) example below.

The code in an interrupt handler has many limitations, see [Interrupt Handler Restrictions](#interrupt-restrictions). 

**_This means that a hardware timer interrupt may not always be the best solution._**

This timer library was developed to provide STEP pulses to a stepper motor controller. For this application, a timer generates regular interrupts at (say) 25kHz, and the interrupt handler decides whether it is time to toggle a motor's STEP pin or not, according to the acceleration/deceleration curves. This means the motor control is _completely independent_ of the main program, being controlled only by the interrupt handler. The main program just polls to find out if the motor movement has been completed before setting up and starting the next movement.

Very often an interrupt handler will just read or write a digital pin or two, which is always fast and safe (and for that you may need the $${\color{green}mumanchu}$$ "Universal Fast Digital IO Library" - coming soon :-)


<a name="what-is-tc-tcc"></a>

## What is a 'Timer/Counter' (TC) and a 'Timer/Counter for Control' (TCC)?

SAMDs contain two types of timer/counters, TC and TCC. 

TC is a "general-purpose Timer/Counter", configurable as 8, 16 or 24 bits. This library always runs TCs as 16-bit timers. 

TCC is a "Timer/Counter for Control" with many additional features for waveform generation, frequency generation and pulse width modulation. You can read all about the complexities of Timers and Counters in the [Data Sheets](#reference-documents).

For our purposes, you can use either a TC or a TCC to generate timer interrupts. `#include` the include file according to the timer/counter type (and the SAMD type) you want to use, i.e. `MultiTimerSAMD21TC.h` or `MultiTimerSAMD21TCC.h`, or both.

Which one you choose depends on which TCs or TCCs are used by other features. See [How to determine which timers are already used](#which-timers-are-used).



<a name="using-the-library"></a>

## Using the Library

The library may not be visible in the online Arduino IDE Library Manager, but you can install it from the github ZIP file using the Arduino IDE, then open the example sketch.

1. Download the ZIP file from https://github.com/mumanchu/MultiTimerSAMD with "\<\> Code / Download ZIP".

2. Open the Arduino IDE. Use "Sketch / Include Library > Add ZIP Library..." and choose the ZIP file you just downloaded, `MultiTimerSAMD-main.zip`. This installs the MultiTimerSAMD library in the folder 'C:\Users\<user-name>\Documents\Arduino\libraries\MultiTimerSAMD', and it can now be used from the Arduino IDE. (You may not see it in the Arduino Library Manager.)

3. Now you can open the `MultiTimerSAMDExample1` sketch. Do "File / Examples" to open the list of installed examples. Scroll down into the 'Examples from Custom Libraries' section and find 'MultiTimerSAMD'. Select 'MultiTimerSAMDExample1'.

4. Connect your SAMD21 or SAMD51 board, e.g. Arduino Zero (recommended because of its built-in EDBG debugger), Arduino Nano 33 IoT, Arduino MKR 10x0, Adafruit Metro M4, etc. Select the board and port in the Arduino IDE.

5. Start the "Tools / Serial Monitor" and set the baudate to '115200'. 

6. Press the -> button to build and upload the example sketch. 

7. In the Serial Monitor, you should see the four timer interrupt frequencies displayed every second. 

8. You may want to save the example sketch with "File / Save As...", so it will open again if you close and re-open the Arduino IDE.

> [!TIP]
> Before you start using the MultiTimer library, it's a good idea to find out which timers are used by the other libraries you are using. To do this, see [How to determine which timers are already used](#which-timers-are-used).\
TC5 is used by the Arduino `tone()` function, so do not use TC5 if you use `tone()`.


<a name="example-sketch"></a>

## Example Sketch

The example sketch `MultiTimerSAMDExample1.ino` should run on all SAMD21 or SAMD51 boards. The relevant code is selected by the `#ifdef _SAMD21_` and `#ifdef _SAMD51_` statements.

<details>
<summary>Click to expand the code for the example sketch</summary>

### MultiTimerSAMDExample1.ino

```cpp
/////////////////////////////////////////////////////////
// SAMD MultiTimer Library Example Sketch
// MultiTimerSAMDExample1.ino
// Copyright (C) 2026, muman.ch
// https://github.com/mumanchu/MultiTimerSAMD
/*
This example uses two timers, TC3 and TCC2.
By default, timer channel 0 (CH0) is used to generate an interrupt.
The example also uses channel 1 (CH1) of each timer to generate an 
additional interrupt.
So four interrupts are generated, at 1000, 2000, 3000 and 4000Hz.
Each has its own interrupt handler.
The interrupts are counted and displayed every second.

Use the Serial Monitor to display the output from Serial.

This example has been tested on an Arduino Zero (SAMD21) and an 
Adafruit Metro M4 Express (SAMD51). You can use the Arduino IDE, 
or Microsoft Visual Studio 2022 with the Visual Micro extension.
*/


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// In DEBUG mode, detect and log errors
#include "MumanchuDebug.h"

#ifdef DEBUG
// if using a debugger, disable all GCC compiler optimisations
#pragma GCC optimize ("-O0")

// Shared error logging function
void LogError(const char* msg, const char* filePath, uint line)
{
	char buf[256];
	const char* fname = strrchr(filePath, '\\');
	fname = fname ? fname + 1 : filePath;
	sprintf(buf, "ERROR: %s : %s(%u)", msg, fname, line);
	Serial.println(buf);
	Serial.flush();
}
#endif
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// Different code is generated for the SAMD21 and SAMD51
#if defined(_SAMD21_)

// Define the timers and channels to be used
// this controls what code is generated
#define USING_TIMER_SAMD21_TC3
#define USING_TIMER_SAMD21_TC3_CH1
#include "MultiTimerSAMD21TC.h"		// this generates the code for the TC timers
#define USING_TIMER_SAMD21_TCC2
#define USING_TIMER_SAMD21_TCC2_CH1
#include "MultiTimerSAMD21TCC.h"	// this generates the code for the TCC timers

// Construction
MultiTimerSAMD21TC timerTC3(TC3);
MultiTimerSAMD21TCC timerTCC2(TCC2);

#elif defined(_SAMD51_)

// Define the timers and channels to be used
// this controls what code is generated
#define USING_TIMER_SAMD51_TC3
#define USING_TIMER_SAMD51_TC3_CH1
#include "MultiTimerSAMD51TC.h"
#define USING_TIMER_SAMD51_TCC2
#define USING_TIMER_SAMD51_TCC2_CH1
#include "MultiTimerSAMD51TCC.h"

// Construct the timers
MultiTimerSAMD51TC timerTC3(TC3);
MultiTimerSAMD51TCC timerTCC2(TCC2);

#endif

// Timer callback functions
// these just count the interrupts so you know its working
uint counter0 = 0, counter1 = 0, counter2 = 0, counter3 = 0;
void timerCallback0() { ++counter0; }
void timerCallback1() { ++counter1; }
void timerCallback2() { ++counter2; }
void timerCallback3() { ++counter3; }


void setup() 
{
	Serial.begin(115200);

	// delay to give you time to open the serial monitor
	delay(5000);

	Serial.println("\n\rStarted...\n\r");
	Serial.flush();

	pinMode(LED_BUILTIN, OUTPUT);

	// initialize the timers and channels
	// the API is the same for both SAMD21 and SAMD51
	
	// TC3 CH0
	if (!timerTC3.begin(1000, timerCallback0)) {
		LOGERROR("timerTC3.begin() failed");
		while (1) yield();
	}

	// TC3 CH1
	if (!timerTC3.beginChannel(1, 2000, timerCallback1)) {
		LOGERROR("timerTC3.beginChannel() failed");
		while (1) yield();
	}

	// TCC2 CH0
	if (!timerTCC2.begin(3000, timerCallback2)) {
		LOGERROR("timerTCC2.begin() failed");
		while (1) yield();
	}

	// TCC2 CH1
	if (!timerTCC2.beginChannel(1, 4000, timerCallback3)) {
		LOGERROR("timerTCC2.beginChannel() failed");
		while (1) yield();
	}

	// start the timers
	timerTC3.enable();
	timerTCC2.enable();
}

// For flashing the LED
bool ledState = true;

void loop() 
{
	ulong t = micros();

	// Simple scheduler
	static ulong t1 = 0;
	if (t1 == 0)		// delay at startup, else patch out for immediate first call
		t1 = t;
	if (t - t1 >= 1000000) {	// elapsed time
		t1 = t;

		// every second, take a snapshot of the interrupt counters and clear them
		noInterrupts();
		uint counter0s = counter0;
		uint counter1s = counter1;
		uint counter2s = counter2;
		uint counter3s = counter3;
		counter0 = 0;
		counter1 = 0;
		counter2 = 0;
		counter3 = 0;
		interrupts();

		// print the counter values, these are the frequencies in Hz
		char buf[256];
		sprintf(buf, "counter0=%u", counter0s);
		Serial.println(buf);
		sprintf(buf, "counter1=%u", counter1s);
		Serial.println(buf);
		sprintf(buf, "counter2=%u", counter2s);
		Serial.println(buf);
		sprintf(buf, "counter3=%u", counter3s);
		Serial.println(buf);
		Serial.println("");
		Serial.flush();

		// toggle the led so you know it's running
		digitalWrite(LED_BUILTIN, ledState);
		ledState = !ledState;
	}
}
```
</details>


<a name="include-files"></a>

### The `#include` Files

The MultiTimer include files contain the class definitions AND code, so they can only be included in **ONE** file in your sketch (.ino or .cpp), else you will get 'multidefined' errors. It is good practice to keep the code for a particular feature in just one file, either your main `.ino` file or a separate `Timers.cpp` file (for example). If you do this then you do not need separate `.h` and `.cpp` files. You can use inheritance or encapsulation to extend the features.

Each include file is for a particular SAMD device (SAMD21 or SAMD51) and a particular timer type, TC or TCC.

These are the library's include files:
```cpp
MultiTimerSAMD21TC.h
MultiTimerSAMD21TCC.h
MultiTimerSAMD51TC.h
MultiTimerSAMD51TCC.h
```

> [!NOTE]
> The file `MultiTimerSAMD.h` should not be used. It's there only to satisfy the Arduino library requirements that the include file must have the same name as the library.


<a name="using-defines"></a>

### Using `#defines` to control code generation

`#define` is a _preprocessor directive_. It is processed by the compiler before it compiles the code. The MultiTimer library uses `#define` to create symbols that define which timers and channels will be used, and to control the code that is generated. It does not generate any code that is not needed.

The main reason for this is because of the way the system's interrupt handlers (ISRs) are declared. The interrupt handlers have fixed names, like `TC3_Handler`. So if a function called `TC3_Handler` exists, then it is this function that is called for the timer TC3 interrupt. This function is not be created if the timer is not used, so you must tell the compiler that this function should be created. That's where the `#defines` come in. 

Each timer, and each of the timer's channels, has an associated symbol which must be defined so the library uses that timer and channel. If it's not defined then you will get an error, either at build time or at run time.

For example, define `USING_TIMER_SAMD21_TC3` to use timer/counter TC3 channel 0. This also creates the code and interrupt handler for the timer/counter. If you also want to use channel 1 of that timer/counter, then also define `USING_TIMER_SAMD21_TC3_CH1`.

Define the symbols immediately before the `#include` statement, as shown in the [Example sketch](#example-sketch). 

***Only define the symbols for the timers and channels that you need!***

If the timer does not exist (not all SAMD devices have all the timers), then you will get a compiler error or `begin()` will fail whith a `LOGERROR()` message that tells you what's wrong.

<details>
<summary>Click to expand the list of symbols that can be defined</summary>


<a name="the-define-symbols"></a>

### The `#define` Symbols
```
#if defined(_SAMD21_)

// For MultiTimerSAMD21TC.h
// the TC Timer/Counters have 2 channels
#define USING_TIMER_SAMD21_TC3
#define USING_TIMER_SAMD21_TC3_CH1
#define USING_TIMER_SAMD21_TC4
#define USING_TIMER_SAMD21_TC4_CH1
#define USING_TIMER_SAMD21_TC5
#define USING_TIMER_SAMD21_TC5_CH1
#define USING_TIMER_SAMD21_TC6
#define USING_TIMER_SAMD21_TC6_CH1
#define USING_TIMER_SAMD21_TC7
#define USING_TIMER_SAMD21_TC7_CH1

// For MultiTimerSAMD21TCC.h
// TCC0 has 4 channels, the rest have 2
#define USING_TIMER_SAMD21_TCC0
#define USING_TIMER_SAMD21_TCC0_CH1
#define USING_TIMER_SAMD21_TCC0_CH2
#define USING_TIMER_SAMD21_TCC0_CH3
#define USING_TIMER_SAMD21_TCC1
#define USING_TIMER_SAMD21_TCC1_CH1
#define USING_TIMER_SAMD21_TCC2
#define USING_TIMER_SAMD21_TCC2_CH1

#elif defined(_SAMD51_)

// For MultiTimerSAMD51TC.h
// the TC Timer/Counters have 2 channels
#define USING_TIMER_SAMD51_TC0
#define USING_TIMER_SAMD51_TC0_CH1
#define USING_TIMER_SAMD51_TC1
#define USING_TIMER_SAMD51_TC1_CH1
#define USING_TIMER_SAMD51_TC2
#define USING_TIMER_SAMD51_TC2_CH1
#define USING_TIMER_SAMD51_TC3
#define USING_TIMER_SAMD51_TC3_CH1
// the following are only present on certain SAMD51 models
#define USING_TIMER_SAMD51_TC4
#define USING_TIMER_SAMD51_TC4_CH1
#define USING_TIMER_SAMD51_TC5
#define USING_TIMER_SAMD51_TC5_CH1
#define USING_TIMER_SAMD51_TC6
#define USING_TIMER_SAMD51_TC6_CH1
#define USING_TIMER_SAMD51_TC7
#define USING_TIMER_SAMD51_TC7_CH1

// For MultiTimerSAMD51TCC.h
// Channels, TCC0=6, TCC1 = 4, TCC2 = 4, TCC3 = 2, TCC4 = 2
#define USING_TIMER_SAMD51_TCC0
#define USING_TIMER_SAMD51_TCC0_CH1
#define USING_TIMER_SAMD51_TCC0_CH2
#define USING_TIMER_SAMD51_TCC0_CH3
#define USING_TIMER_SAMD51_TCC0_CH4
#define USING_TIMER_SAMD51_TCC0_CH5
#define USING_TIMER_SAMD51_TCC1
#define USING_TIMER_SAMD51_TCC1_CH1
#define USING_TIMER_SAMD51_TCC1_CH2
#define USING_TIMER_SAMD51_TCC1_CH3
#define USING_TIMER_SAMD51_TCC2
#define USING_TIMER_SAMD51_TCC2_CH1
#define USING_TIMER_SAMD51_TCC2_CH2
// the following are only present on certain SAMD51 models
#define USING_TIMER_SAMD51_TCC3
#define USING_TIMER_SAMD51_TCC3_CH1
#define USING_TIMER_SAMD51_TCC3
#define USING_TIMER_SAMD51_TCC3_CH1

#endif
```
</details>

In `setup()` you must call the `begin()` or `beginChannel()` handlers for each timer and each channel, as in the [Example sketch](#example-sketch). Each timer and channel needs its own callback handler to handle the interrupt. Call `enable()` to start the timer.

 
> [!WARNING]
> If the interrupt handler (`TC0_Handler` etc) is not created, then the default handler `Default_Handler()` is called. This puts the MCU into an infinite loop _with interrupts disabled_! After this, it can be very difficult to download a corrected program. The downloads (or uploads, as Arduino call them) always fail. This happened to me regularly with the _Adafruit Metro M4 Express_ board. It was necessary to mess about with the reset button and multiple download attempts until it eventually worked again. If you have a board with built-in debugging (Arduino Zero), or if you are using a fancy Atmel-ICE or J-Link debugger, then it's not a problem because you can hit Stop or Break to regain control.


<a name="library-api"></a>

## Library API (Application Programming Interface)

The API is the same for all the timer classes: For the two counter/timer types (TC and TCC) and for different microcontrollers (SAMD21/51, STM32, ESP32 and AVR processors).

**`MultiTimerSAMD21TC(Tc* tc); MultiTimerSAMD21TCC(Tcc* tcc);`**\
**`MultiTimerSAMD51TC(Tc* tc); MultiTimerSAMD51TCC(Tcc* tcc);`**\
Constructs the `MultiTimer` object. Add this after the `#defines` and the `#includes`, supplying the TCx or TCCx timer object. TC0 and TCC0 (etc) are already pointers, so you don't need the `&`. (See [Pointers](#pointers) if you are uncertain about pointers.)
```cpp
#define USING_TIMER_SAMD21_TC0
#define USING_TIMER_SAMD21_TCC1
#include "MultiTimerSAMD21TC.h"
#include "MultiTimerSAMD21TCC.h"
MultiTimerSAMD21TC timer0(TC0);
MultiTimerSAMD21TC timer1(TCC1);
```

**`bool begin(float frequency, void (*timerCallback)());`**\
The initializer. Call this in `setup()`. It initializes the timer channel 0 (MC0) for the given frequency, and supplies the callback function. It returns `false` if something fails, and a error message is output on `Serial`. See [Debug features](#debug-features) below.
```cpp
if (!timer0.begin(1000, timer0Channel0Callback) {
	// fatal error
}
```

**`bool beginChannel(uint channel, float frequency, void (*timerCallback)());`**\
It you want to use the timer's additional channels, use the CHx `#defines`, then call this from `setup()` for each channel, after calling `begin()`. It returns 'false' if something's wrong.

```cpp
if (!timer0.begin(1000, timer0Channel0Callback) {
	// fatal error
}
if (!timer0.beginChannel(1, 2000, timer0Channel1Callback)) {
	// fatal error
}
```

**`void enable();`**\
Enables and starts the timer. Interrupts will begin. Always call this after `begin()`, otherwise the timer will remained stopped. If `disable()` was used, the timer will continue from where it was disabled.

**`void disable();`**\
Disables (stops) the timer. Interrupts will be paused.

**`bool interruptPending(uint channel = 0);`**\
Call this to see if another interrupt has occurred for this timer and the given channel. This means your interrupt handler is taking too long! The default channel (if not supplied) is CH0.



<a name="debugging-features"></a>

## Debugging Features

Debugging without a hardware debugger is very difficult. You can only use `Serial.print()`. To help out, all the $${\color{green}mumanchu}$$ libraries use `ASSERT()` and `LOGERROR()` macros. 

If `DEBUG` is defined, these macros will output error messages to the `Serial` channel. If `DEBUG` is not defined, no code is generated and they will do nothing.

`ASSERT()` checks that a boolean expression is true, and if not it outputs an error message and returns `false`. `ASSERT()` is designed to be used _only_ in functions that return `bool`, where `true` = success and `false` = failure. 

`LOGERROR()` displays a message along with the source file name and line number where the error occurred. You might use this at the end of a `switch()` statement if the value can't be processed.

These macros are not defined in the library itself because they are used by all the $${\color{green}mumanchu}$$ libraries. They reside in a common include file called `MumanchuDebug.h`. They share a common function called `LogError()`, which is usually defined in the main `.ino` file. 

Almost all library methods return `true` or `false` to indicate success or failure. It is important to always check the return values of the `begin()` methods. After debugging, the return values of other methods can be ignored unless it is something that could fail, like a communications request.

> [!TIP]
> You can modify `LogError()` to do anything you want. If you want to use `Serial` for something else, you can modify `LogError()` to use `Serial1`, for example. 

The `MumanchuDebug.h` file

```cpp
#pragma once
////////////////////////////////////////////////////////////////
// MumanchuDebug.h
// Macro definitions for debugging and logging
// Copyright (C) 2026, muman.ch

// Comment this out for Release mode
#define DEBUG

#ifdef DEBUG
extern void LogError(const char* msg, const char* filePath, uint line);
#define LOGERROR(msg) { LogError(msg, __FILE__, __LINE__); }
#define ASSERT(b) { if(!(b)) { LOGERROR("ASSERT failed, " #b); return false; } }
#else
#define LOGERROR(msg)
#define ASSERT(b)
#endif
```

Put this code at the start of the main `.ino` file. It contains the shared `LogError()` method, and an optional `#pragma` to disable all optimisation if you are using a hardware debugger. 

```cpp
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// In DEBUG mode, detect and log errors
#include "MumanchuDebug.h"

#ifdef DEBUG
// if using a debugger, disable all GCC compiler optimisations
#pragma GCC optimize ("-O0")

// Shared error logging function
void LogError(const char* msg, const char* filePath, uint line)
{
	char buf[256];
	const char* fname = strrchr(filePath, '\\');
	fname = fname ? fname + 1 : filePath;
	sprintf(buf, "ERROR: %s : %s(%u)", msg, fname, line);
	Serial.println(buf);
	Serial.flush();
}
#endif
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
```


<a name="simple-scheduler"></a>

## A simple scheduler which doesn't need a timer interrupt

If you don't need precise timing, then a simple `loop()` scheduler using `millis()` for the timing will be sufficient. Many of the Arduino timer libraries do it this way. 

Because interrupts are not used, this has the _huge advantage_ that there are no restrictions on what can be done in the scheduler handlers. The only restriction is that the combined worst-case processing time of all the handlers should not be longer than the shortest scheduler period.

A simple scheduler like this is fine for many applications. I have used this simple pattern in many projects, combining it with an interrupt handler or two. There's often no need for a fancy and complicated "real time operating system". This does the same thing.

```cpp
// Simple Scheduler

void ourHandler1() { }
void ourHandler2() { }

void loop() 
{
	unsigned long t = millis();		// get elapsed milliseconds since start-up

	// ourHandler1 scheduler
	static unsigned long t1 = 0;	// ms for elapsed time calculation
	//if (t1 == 0)					// delay at startup, else patch out for immediate first call
	//	t1 = t;
	if (t - t1 >= 300) {			// every 300ms, call ourHandler1
		t1 = t;						// save time of last call
		ourHandler1();
	}
	
	// ourHandler2 scheduler
	static unsigned long t2 = 0;	// ms for elapsed time calculation
	if (t2 == 0)					// delay at startup, else patch out for immediate first call
		t2 = t;
	if (t - t2 >= 1000) {			// every 1000ms, call ourHandler2
		t2 = t;						// save time of last call
		ourHandler2();
	}
	
	// add as many schedulers as you need...

}
```
This works for periods of up to 49.71 days (4'294'967'295 milliseconds) - the 32-bit millisecond counter will overflow _more than once_ after that. A single overflow is not a problem because the elapsed time substraction will roll over and still be correct.

Some people use a hardware timer to set a flag, and then poll the flag in `loop()`. But that's doing the same thing as the code above, so it does not make much sense to use a hardware timer. And `millis()` _already_ uses an accurate hardware timer.

Note that a `static` value (`t1` and `t2`) is initialised once, and its value is retained between calls to `loop()`. If it is declared inside `poop()`, as in the example above, then it can't be accessed as a global variable.


<a name="simple-debouncer"></a>

## A simple switch debouncer which doesn't need a timer interrupt

A switch input can either be polled or can generate an interrupt. In both cases an input from a mechanical switch (relay, microswitch, push-button, etc) should always be _debounced_. This is because the contacts of a mechanical switch usually "bounce" (vibrate) and produce multiple open/close signals before settling to the correct state. Timers are often used for debouncing. 

But you don't need to use a hardware timer, `millis()` or `micros()` is fine, _even in an interrupt handler_. 

Note: On some architectures `millis()` and `micros()` should be avoided in interrupt handlers because interrupts are disabled in the handler, and these functions rely on interrupts for their accuracy. But if the interrupt does not occur, all it does is cause the reading to sometimes be 1 millisecond behind. This makes _no difference_ to a debounce procedure which usually needs around 5 to 10 milliseconds, so ± 1 millisecond is irrelevant.

However, with the SAMD Arduino platform you _can_ call these functions in an interrupt handler because they are 'interrupt safe'. See [Technical Notes](#technical-notes) below. 

Here's a simple debouncer which can be used either from an input interrupt handler or from a polled input. Just create a `Debouncer2` object for each input to be debounced, supplying the number of microseconds for the debounce timer in the constructor or with `begin()`. Call `stateChanged()` regularly from `loop()` or the interrupt handler, supplying it with the latest input state. `stateChanged()` returns `true` if the debounced state has changed, and the current state is returned in `*debouncedState`.

<details>
<summary>Click to expand the debouncer code</summary>

### Debouncer2.h

```cpp
#pragma once

// Debouncer using a microsecond timer
// muman.ch, 2019.03.14
/*
Use this for debouncing a boolean value such as a switch or input state.
Unlike some other debouncers, this code registers activation (closing)
immediately. It is only the deactivation (opening) that is delayed by 
the timer.
*/

class Debouncer2
{
private:
	unsigned long usDelay;
	bool firstCall;
	bool wasActive;
	unsigned long usTimer;

public:
	Debouncer2() { }
	Debouncer2(unsigned long usDelay);

	void begin(unsigned long usDelay);
	bool stateChanged(bool currentState, bool* debouncedState);
};

// If you use the constructor, then you don't need to call begin()
Debouncer2::Debouncer2(unsigned long usDelay)
{
	begin(usDelay);
}

void Debouncer2::begin(unsigned long usDelay)
{
	this->usDelay = usDelay;
	firstCall = true;
}

// Returns 'true' if the debounced state has changed, with the state in
// 'debouncedState'. 'currentState' and 'debouncedState' are active high.
// Detects activation immediately, deactivation after the delay.
bool Debouncer2::stateChanged(bool currentState, bool* debouncedState)
{
	// register current state as changed on first call
	if (firstCall) {
		firstCall = false;
		wasActive = !currentState;
	}

	// newly activated
	if (currentState && !wasActive) {
		// register as active immediately
		*debouncedState = true;
		wasActive = true;
		return true;
	}

	// was active, still active or active again
	if (currentState && wasActive) {
		*debouncedState = true;
		usTimer = 0;	// restart deactivation timer
		return false;
	}
	*debouncedState = wasActive;

	// was active, now inactive
	if (wasActive && !currentState) {
		// start deactivation timer
		if (usTimer == 0) {
			usTimer = micros();
			return false;
		}
		// or check for deactivation timeout
		if (micros() - usTimer >= usDelay) {
			// timed out, set inactive
			wasActive = false;
			*debouncedState = false;
			usTimer = 0;
			return true;
		}
	}

	// state unchanged
	return false;
}
```
</details>


<a name="technical-notes"></a>

## Technical Notes (put on your Anorak before reading)

Joke of the week: "The IDE's of March"


<a name="interrupt-restrictions"></a>

### Interrupt handler (ISR) restrictions and best practices

_Interrupts are disabled during an interrupt service routine (ISR)._ This means that any functions which use interrupts themselves _will not work_. In particular, you must _not_ call any communications functions, `Serial`, `Wire`, `SPI`, WiFi, Bluetooth, etc. Calling any of these will usually hang your program. Remember that a timer interrupt can occur at _any point_ in your program.

Despite what many say, with the SAMD devices you _can_ use `millis()` and `micros()` in an ISR because these are 'interrupt compatible'. See [Technical notes](#technical-notes) for details.

You can also use `delayMicroseconds()` because this is just a small software loop. `delay()` will also work, but delaying for milliseconds in an ISR is a _not_ a good idea.

Interrupt handlers must be as fast as possible, using the smallest possible code. They should not contain any significant loops or delays. For example, if an ISR is called every 100 milliseconds and it takes 50 milliseconds to process (way too long), then it's using 50% of the processing time! Disabling interrupts for 50ms would also cause received serial data to be lost (buffer overruns), and may cause transmission errors too if the receiver is time-dependent. 

Therefore, the worst-case ISR time often depends on the communications overhead, taking into account the size of the receive buffers and the data rates.

> [!TIP]
> You can solve communications problems by reducing the priority of the timer interrupts so communications interrupts are handled first. See the commented out `//NVIC_SetPriority(irqn, 0); // advanced: set the interrupt priority (default = 0)` lines in the source code. But this means that your timer interrupt may be delayed by the communications, damn!


If you have an oscilloscope, you can measure the ISR time by toggling an output at the start and end of the handler - but bear in mind that `digitalWrite()` itself can take many microseconds (depending on the speed of your MCU). Using the $${\color{green}mumanchu}$$ "Fast Digital IO Library" (coming soon) will help you here. 

Global variables that are referenced by an interrupt handler should be declared as `volatile` to disable the optimizer so they are not cached in a CPU register.

Variables shared by the main program and the interrupt handler should be protected by disabling interrupts when they are accessed by the main program, so the value is not modified by the ISR at the same moment it is accessed by the main program. For example,

```cpp
volatile int valueUpdatedByISR;

loop() 
{
	// disable interrupts while taking a snapshot of valueUpdatedByISR
	noInterrupts();
	int temp = valueUpdatedByISR;
	// you may want to reset it too, to indicate its been read
	valueUpdateByISR = 0;
	interrupts();
	
	// now use the snapshot value
	if (temp != 0) { }
}
```


<a name="which-timers-are-used"></a>

### How to determine which timers are already used

I looked very hard, but nobody who has written a timer interrupt library has described how to do this. They just say something like, "make sure the timer is not already used", which doesn't help much.

But maybe there is a way...

The file 'cortex_handlers.c' contains the default timer interrupt handler definitions. The fixed-name handlers are defined like this:
```cpp
void TCC0_Handler     (void) __attribute__ ((weak, alias("Dummy_Handler")));
```
This means if `TCC0_Handler` is not defined elsewhere, it will call the alias `Dummy_Handler` (which is an infinite loop with interrupts disabled!)

But if `TCC0_Handler` has been defined elsewhere, it will override the `weak` alias, and it will no longer have the same address as `Dummy_Handler`.

So if a handler does not have the same address as `Dummy_Handler`, then you can assume the timer has been used because somebody has defined the interrupt handler.

The code to do this is below. Before you start using the MultiTimerSAMD library, you can call this from your sketch and use it to display a list of the timers used by the current program. The library has this code in `PrintUsedTimers.h`.

<details>
<summary>Click to expand the code</summary>

### PrintUsedTimers.h
```cpp
#pragma once
// Prints a list of used and unused TCC and TC timers
// muman.ch, 2026

// The default interrupt handlers, from cortex_handlers.c
extern "C" {
	void Dummy_Handler();
	void TCC0_Handler(void);
	void TCC1_Handler(void);
	void TCC2_Handler(void);
	void TCC3_Handler(void);
	void TC0_Handler(void);
	void TC1_Handler(void);
	void TC2_Handler(void);
	void TC3_Handler(void);
	void TC4_Handler(void);
	void TC5_Handler(void);
	void TC6_Handler(void);
	void TC7_Handler(void);
}

#define IS_TIMER_USED(timer) \
	if (timer##_Handler == Dummy_Handler) \
		stream.println( #timer " used"); \
	else \
		stream.println( #timer " not used");

void printUsedTimers(Stream& stream)
{
	IS_TIMER_USED(TCC0);
	IS_TIMER_USED(TCC1);
	IS_TIMER_USED(TCC2);
	#ifdef TCC3
	IS_TIMER_USED(TCC3);
	#endif
	#ifdef TC0
	IS_TIMER_USED(TC0);
	#endif
	#ifdef TC1
	IS_TIMER_USED(TC1);
	#endif
	#ifdef TC2
	IS_TIMER_USED(TC2);
	#endif
	IS_TIMER_USED(TC3);
	IS_TIMER_USED(TC4);
	IS_TIMER_USED(TC5);
	#ifdef TC6
	IS_TIMER_USED(TC6);
	#endif
	#ifdef TC7
	IS_TIMER_USED(TC7);
	#endif
	stream.println("");
	stream.flush();
}
```
</details>


<a name="delay-dot-c"></a>

### Code in the SAMD's `delay.c`

`delayMicroseconds()` is a simple loop, looping a number of times according to the processor speed. This means you can call it from an ISR with interrupts disabled.
But if interrupts are _enabled_, and an interrupt occurs _during the loop_, then it will delay far longer. I'm not sure that's a good idea.

```asm
//r3 = usec * (VARIANT_MCK / 1000000) / 3;
loop:
	subs r3, #1        // 1 Core cycle
	bne.n loop         // 1 Core cycle + 1 if branch is taken
```    

For `millis()`, I noticed this interesting comment. Does this mean it may not work if interrupts are _enabled_?

```cpp
unsigned long millis( void )
{
	// todo: ensure no interrupts
	return _ulTickCount ;
}
```

The code for `micros()` has a nice comment about how it works. It accesses the SAMD's `SysTick` register directly, and also checks the 'interrupt pending flag'. So it _does_ work if interrupts are disabled.

```cpp
// Interrupt-compatible version of micros
// Theory: repeatedly take readings of SysTick counter, millis counter and SysTick interrupt pending flag.
// When it appears that millis counter and pending is stable and SysTick hasn't rolled over, use these
// values to calculate micros. If there is a pending SysTick, add one to the millis counter in the calculation.
unsigned long micros( void ) { ... }
```


<a name="pointers"></a>

### Pointers, `&` and `*`

<details>
<summary>This library uses pointers. Click to expand the text if you are a beginner and want to learn about pointers.</summary>

###  

The Arduino Library Style Guide states, "_Don't assume knowledge of pointers. Beginners with C find this the biggest roadblock, and get very confused by `&` and `*`_".

I know the feeling, but the problem is that many things _cannot_ be done without using pointers. Even the `TC` and `TCC` Timer/Counter objects are pointers. The name of an array is pointer, the name of a function is a pointer, `void (*interruptCallback)()` is a pointer to a function. But it's really quite easy, so I'll try to confuse you...

A 'pointer' is just a memory address. 

`&` means _address-of_ or _pointer-to_. 

`*` means _contents-of-the-address_ or _what-the-pointer-points-to_. 

That's it.

```cpp
int data;
int* pointerToData = &data;		// &data is the memory address of 'data', so pointerToData is given the address of 'data'
								// int* means 'pointer to integer'
*pointerToData = 123;			// *pointerToData is what the pointer points to, so 'data' is set to 123
``` 
Pointers should have the same data type as the data they point to. 

However, there are also `void` pointers, `void*`, which means that the data type it points to is unknown. Void pointers can point to anything.

Before you can access the data it points to, `void` pointers must know the data type, e.g. `*(int*)voidPointer = 1;` tells the compiler that `voidPointer` is _a pointer to_ an `int`. This is known as 'casting' the pointer to the correct data type.

Pointers can even point to nothing. A `NULL` pointer, with the value 0, is an unassigned pointer. If you try to access what it points to, your program will crash! NULL pointers are one of the major causes of serious and hard-to-find bugs, so they must be avoided at all costs! It's a good idea to use `ASSERT(pointer != NULL)` wherever there may be a NULL pointer. And ALWAYS validate the pointers returned by `malloc()` (or don't use it at all, see below). 

We can also have _pointers-to-pointers_. `int** pp` is a _pointer-to-a-pointer-to_ an `int`. Or you might have a pointer to an array of pointers to functions which return pointers to structures containing pointers to...  but we won't go there.
 
```cpp
int i = 0; 
int* p = &i; 		// p is the address of i, so p points to i
int** pp = &p; 		// pp is the address of p, so pp points to p which points to i
**pp = 123;			// the sets the-contents-of-the-contents-of pp to 123, so i = 123
```

`&` can also mean _reference to_ (if it's on the left-hand side of the assignment or is a function parameter). A reference is really a hidden pointer, except you don't need to mess about with `*` and `&`. Unlike pointers, references can only be assigned at runtime.

```cpp
int i = 0;
int& iref = i;		// iref is a reference to i
iref = 123;			// what iref references is set to 123, so i = 123
```


### Tip: Avoid using `malloc()` and `free()`

In embedded programs it can be a bad idea to use `malloc()` and `free()` because memory allocation at runtime is slow, has a high overhead (perhaps 16 bytes added to each allocation), and a small amount of RAM 'heap' memory (i.e. if you only have 32KB RAM) can quickly become fragmented. You must also check for `NULL` after _every_ call, in case you've run out of heap memory. ('Heap' memory is whatever RAM is left after all the static data and the runtime stack has been allocated.)

For the same reason, avoid using `String` and use character arrays instead, `char buf[10];` or `const char* s = "hello";` etc. Using `const` means the text is stored in flash memory, not RAM.

I have had nasty problems with fragmentation and running out of heap memory in the past - the program will hang or crash with a `NULL` reference, and without a debugger you will have no idea what happened. It is much more efficient (and much safer) to use static data, data in a class that's constructed at startup, or data that's initialised during `setup()` with the usual `begin()` call where you can check the return value. 

Sometimes you _must_ use `malloc()`, because the amount of data you need may not be known at build time. In this case, use `malloc()` in the `begin()` method, _never_ in the constructor (where handling fatal errors is very difficult), and always check the return value for `NULL` (not enough RAM), so you can handle the fatal error instead of crashing.
</details>


> [!IMPORTANT]
> Few! It's getting hot. Maybe it's time to take off our Anoraks.


<a name="reference-documents"></a>

## Reference documents

**SAMD21 Data Sheet**\
[SAM-D21-DA1-Family-Data-Sheet-DS40001882.pdf](https://ww1.microchip.com/downloads/aemDocuments/documents/MCU32/ProductDocuments/DataSheets/SAM-D21-DA1-Family-Data-Sheet-DS40001882.pdf)
[Alternative](https://content.arduino.cc/assets/mkr-microchip_samd21_family_full_datasheet-ds40001882d.pdf)

**SAMD51 Data Sheet**\
[SAM-D5x-E5x-Family-Data-Sheet-DS60001507.pdf](https://ww1.microchip.com/downloads/aemDocuments/documents/MCU32/ProductDocuments/DataSheets/SAM-D5x-E5x-Family-Data-Sheet-DS60001507.pdf)


**Visual Micro extension for Microsoft Visual Studio**\
https://www.visualmicro.com/
\
https://visualstudio.microsoft.com/vs/community/

Visual Micro is a great alternative to the Arduino IDE. Microsoft Visual Studio Community Edition (MSVC) is free. It has very powerful debug and cross-referencing features. Ideal if you want to develop large programs. I like it because it can be used for developing desktop and mobile applications too, so you only need one IDE for all platforms.

The Visual Micro extension can be installed from MSVC (Extensions / Manage Extensions) and is free to try out for 49 days. After that it costs just $20 a year, or $64 for a lifetime license. (But I'm not sure who's liftime that is.)


<a name="coming-soon"></a>

## New libraries for SAMD21/51, STM32, ESP32 and AVR processors

**_Coming Soon!_**

Now that I've got the hang of this github business, I'll be releasing some more libraries...

- MultiTimer Libraries for STM32, ESP32 and AVR processors
- Stepper Motor Control Library
- TMC2209 Motor Controller Library
- Universal Fast Digital IO Library
- AS3935 Lightning Sensor Library (with desktop app)
- and there are many more...


<a name="matts-blog"></a>

## Matt's Blog

You can find more software and examples here...\
[https://muman.ch/muman/index.htm?muman-matts-blog.htm](https://muman.ch/muman/index.htm?muman-matts-blog.htm)


<a name="license-and-disclaimer"></a>

## License and Disclaimer

Released under the BSD license, see [LICENSE](LICENSE). Or should I use the MIT license? Or the GNU? I don't know.

If you use this code, please acknowledge the author. Don't forget, according to Dante Alighieri (Dante's Inferno), plagiarism is one of the deadly sins and you'll end up in hell (Eighth circle, Bolgia 10).
<br/>


## The Additional $${\color{green}mumanchu}$$ Disclaimer :grinning:

This so-called _software_* is used entirely at your own risk. Neither the authors nor the distributors will accept any responsibility or liability for any physical or mental damage, injuries or deaths, of any persons, pets or relatives, living or already dead, or for any loss or accumulation of data, or any other possible effect that may, or may not, result from the use, non-use, misuse or abuse of this software.

ALL material, including this Disclaimer, is supplied "AS IS" without warranty of quality or accuracy of ANY KIND. (But we use it a lot and it seems OK to us.)

Links to third-party sites do not constitute sponsorship, endorsement or approval of these sites or the contents of these sites.

By using this software, you accept that you have accepted the terms of this Disclaimer. Even if you haven't read it.

As with all modern software, the error messages have been carefully designed to insult your so-called "intelligence" ;-)

_\* This software is subject to change without noticing._
<br/>

> [!TIP]
> If you do find any problems or have any (polite) suggestions for improvements, please send an email to `info@muman.ch` and we'll see what we can do.


<a name="revision-history"></a>

## Revision History

| Date       | Version  | Description |
|:---------- |:---------|:----------- |
| 2026.03.29 | 1.0.0	| The first version! |



