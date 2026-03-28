/////////////////////////////////////////////////////////
// SAMD MultiTimer Library Example Sketch
// MultiTimerSAMDExample1.ino
// Copyright (C) 2026.03.27, muman.ch
// email: info@muman.ch
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

// Construct the timers
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

#include "printUsedTimers.h"

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



