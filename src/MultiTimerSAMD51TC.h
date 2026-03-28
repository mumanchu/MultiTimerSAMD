#pragma once

// SAMD51 Hardware Timer Interrupt Library for TCx Timer Counter
// Copyright (C) muman.ch, 2026.03.15
// All rights reversed
// https://github.com/mumanchu/MultiTimerSAMD

#include <Arduino.h>
#include "MumanchuDebug.h"

#ifdef _SAMD51_

class MultiTimerSAMD51TC
{
public:
	// Static pointers to timer objects for access by interrupt handlers
	static MultiTimerSAMD51TC* timers[TC_INST_NUM];
	static void timerInterruptHandler(uint t);
	TcCount16* tc;				// 16-bit timer base address
	int tcNumber = -1;			// 0..7 for TC0..TC7

	// Two channels, 0 and 1
	uint compareValueCh0 = 0;				// compare value, channel 0
	void (*timerCallbackCh0)() = NULL;		// timer callback, channel 0
	uint compareValueCh1 = 0;				// compare value, channel 1
	void (*timerCallbackCh1)() = NULL;		// timer callback, channel 1

public:
	MultiTimerSAMD51TC(Tc* tc) : tc((TcCount16*)tc) { }

	bool begin(float frequency, void (*timerCallback)());
	bool beginChannel(uint channel, float frequency, void (*timerCallback)());

	void enable()  { tc->CTRLA.bit.ENABLE = 1; while (tc->SYNCBUSY.bit.ENABLE); }
	void disable() { tc->CTRLA.bit.ENABLE = 0; while (tc->SYNCBUSY.bit.ENABLE); }

	bool interruptPending(uint channel = 0) {
		return (tc->INTFLAG.reg & (channel ? TC_INTFLAG_MC1 : TC_INTFLAG_MC0)) != 0;
	}

protected:
	bool calculatePrescalerAndCompareValue(float frequency,
		uint* prescaler, uint* compareValue);
	static const ushort prescalers[8];
};

// Static pointers to timer objects for access by interrupt handlers
MultiTimerSAMD51TC* MultiTimerSAMD51TC::timers[TC_INST_NUM] = { NULL };


// Configure the timer, channel 0
// note: call enable() to start the timer after the optional beginChannel1()
bool MultiTimerSAMD51TC::begin(float frequency, void (*timerCallback)())
{
	ASSERT(timerCallback != NULL);
	ASSERT(sizeof(uint) == 4);

	uint gclkid;			// generic clock multiplexer id
	IRQn_Type irqn;			// interrupt request number
	
	// up to 8 x TC timers, TC0..TC7
	if (0) { }
	#ifdef USING_TIMER_SAMD51_TC0
	else if (tc == (TcCount16*)TC0) {
		tcNumber = 0;
		gclkid = TC0_GCLK_ID;
		irqn = TC0_IRQn;
	}
	#endif
	#ifdef USING_TIMER_SAMD51_TC1
	else if (tc == (TcCount16*)TC1) {
		tcNumber = 1;
		gclkid = TC1_GCLK_ID;
		irqn = TC1_IRQn;
	}
	#endif
	#ifdef USING_TIMER_SAMD51_TC2
	else if (tc == (TcCount16*)TC2) {
		tcNumber = 2;
		gclkid = TC2_GCLK_ID;
		irqn = TC2_IRQn;
	}
	#endif
	#ifdef USING_TIMER_SAMD51_TC3
	else if (tc == (TcCount16*)TC3) {
		tcNumber = 3;
		gclkid = TC3_GCLK_ID;
		irqn = TC3_IRQn;
	}
	#endif
	#if defined(USING_TIMER_SAMD51_TC4) && defined(TC4)
	else if (tc == (TcCount16*)TC4) {
		tcNumber = 4;
		gclkid = TC4_GCLK_ID;
		irqn = TC4_IRQn;
	}
	#endif
	#if defined(USING_TIMER_SAMD51_TC5) && defined(TC5)
	else if (tc == (TcCount16*)TC5) {
		tcNumber = 5;
		gclkid = TC5_GCLK_ID;
		irqn = TC5_IRQn;
	}
	#endif
	#if defined(USING_TIMER_SAMD51_TC6) && defined(TC6)
	else if (tc == (TcCount16*)TC6) {
		tcNumber = 6;
		gclkid = TC6_GCLK_ID;
		irqnCh0 = TC6_IRQn;
	}
	#endif
	#if defined(USING_TIMER_SAMD51_TC7) && defined(TC7)
	else if (tc == (TcCount16*)TC7) {
		tcNumber = 7;
		gclkid = TC7_GCLK_ID;
		irqnCh0 = TC7_IRQn;
	}
	#endif
	else {
		LOGERROR("missing #define USING_TIMER_SAMD51_TCx or TCx not present");
		return false;
	}

	// calculate the prescaler and compare values for the desired frequency
	uint prescaler;
	if (!calculatePrescalerAndCompareValue(frequency, &prescaler, &compareValueCh0)) {
		LOGERROR("unsupported frequency");
		return false;
	}

	// store static pointer to this timer object for access by interrupt handler
	if (timers[tcNumber] != NULL) {
		LOGERROR("timer already defined");
		return false;
	}
	timers[tcNumber] = this;

	// set the user's interrupt handler, channel 0
	this->timerCallbackCh0 = timerCallback;

	// peripheral channel control clock
	// connect GCLK0 to the timer's general clock multiplexer id (gcmid)
	// GCLK0 is the CPU main clock, SAMD51 = 120MHz = F_CPU
	GCLK->PCHCTRL[gclkid].reg = (GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN_GCLK0);
	while (GCLK->SYNCBUSY.reg);

	// disable the timer
	// "The TC should be disabled before the TC is reset in order to avoid undefined behavior"
	disable();

	// reset timer to defaults
	tc->CTRLA.bit.SWRST = 1;
	while (tc->SYNCBUSY.bit.SWRST);

	// 16-bit mode + normal frequency mode + prescaler (not synchronised)
	tc->CTRLA.bit.MODE = TC_CTRLA_MODE_COUNT16;
	tc->WAVE.bit.WAVEGEN = TC_WAVE_WAVEGEN_NFRQ;
	tc->CTRLA.bit.PRESCALER = prescaler;

	// set compare value, channel 0
	tc->CC[0].reg = compareValueCh0;
	while (tc->SYNCBUSY.bit.CC0);

	// enable compare interrupt, channel 0
	tc->INTENSET.bit.MC0 = 1;

	// enable the NVIC interrupt, all channels
	NVIC_ClearPendingIRQ(irqn);
	//NVIC_SetPriority(irqn, 0); // advanced: set the interrupt priority (default = 0)
	NVIC_EnableIRQ(irqn);

	// note: timer is still disabled

	return true;
}

// Configure timer channel 1 for a different frequency and interrupt
// the frequency must use the same prescaler as channel 0
// note : 'channel' must always be 1, the TC timers only have two channels 0 and 1 
bool MultiTimerSAMD51TC::beginChannel(uint channel, float frequency, void (*timerCallback)())
{
	ASSERT(channel == 1 && timerCallback != NULL);

	if (tcNumber == -1) {
		LOGERROR("begin() not called");
		return false;
	}
	uint currentPrescaler = tc->CTRLA.bit.PRESCALER;
	float t = (float)F_CPU / frequency;	// F_CPU == SystemCoreClock
	uint compareValue = (uint)(t / MultiTimerSAMD51TC::prescalers[currentPrescaler]);
	if (compareValue > 65536) {			// 16 bits
		LOGERROR("unsupported frequency");
		return false;
	}
	compareValueCh1 = compareValue;
	timerCallbackCh1 = timerCallback;

	// set channel 1 compare value
	tc->CC[1].reg = compareValue;
	while (tc->SYNCBUSY.bit.CC1);

	// enable channel 1 compare interrupt
	tc->INTENSET.reg = TC_INTENSET_MC1;

	return true;
}


// Calculate the prescaler and compare values
// returns false if timer cannot be configured for this frequency
// (assumes GCLK0, the MCU clock frequency F_CPU)
bool MultiTimerSAMD51TC::calculatePrescalerAndCompareValue(float frequency,
	uint* prescaler, uint* compareValue)
{
	*prescaler = 0;
	*compareValue = 0;

	float t = (float)F_CPU / frequency;	// F_CPU == SystemCoreClock
	for (uint i = 0; i < 8; ++i) {
		uint count = (uint)(t / prescalers[i]);
		if (count <= 65536) {			// 16 bits
			*prescaler = i;
			*compareValue = count;
			return true;
		}
	}
	return false;
}
const ushort MultiTimerSAMD51TC::prescalers[8] = { 1, 2, 4, 8, 16, 64, 256, 1024 };


// Interrupt handlers, TCx_Handler()
// these have fixed names according to the timer
// they override the "weak" definitions which call Dummy_Handler()

//? these seem to be present but there's no handler in cortex_handlers.c ???
#ifdef USING_TIMER_SAMD51_TC0
void TC0_Handler() { MultiTimerSAMD51TC::timerInterruptHandler(0); }
#endif
#ifdef USING_TIMER_SAMD51_TC1
void TC1_Handler() { MultiTimerSAMD51TC::timerInterruptHandler(1); }
#endif
#ifdef USING_TIMER_SAMD51_TC2
void TC2_Handler() { MultiTimerSAMD51TC::timerInterruptHandler(2); }
#endif
#ifdef USING_TIMER_SAMD51_TC3
void TC3_Handler() { MultiTimerSAMD51TC::timerInterruptHandler(3); }
#endif
#ifdef USING_TIMER_SAMD51_TC4
#ifndef TC4
#error TC4 NOT PRESENT
#else
void TC4_Handler() { MultiTimerSAMD51TC::timerInterruptHandler(4); }
#endif
#endif
#ifdef USING_TIMER_SAMD51_TC5
#ifndef TC5
#error TC5 NOT PRESENT
#else
void TC5_Handler() { MultiTimerSAMD51TC::timerInterruptHandler(5); }
#endif
#endif
#ifdef USING_TIMER_SAMD51_TC6
#ifndef TC6
#error TC6 NOT PRESENT
#else
void TC6_Handler() { MultiTimerSAMD51TC::timerInterruptHandler(6); }
#endif
#endif
#ifdef USING_TIMER_SAMD51_TC7
#ifndef TC7
#error TC7 NOT PRESENT
#else
void TC7_Handler() { MultiTimerSAMD51TC::timerInterruptHandler(7); }
#endif
#endif

// Internal interrupt handler for channels 0 and 1
// updates the compare value, calls the user's handler and clears the interrupt flags
void MultiTimerSAMD51TC::timerInterruptHandler(uint t)
{
	MultiTimerSAMD51TC* thisTimer = MultiTimerSAMD51TC::timers[t];
	#ifdef DEBUG
	if (thisTimer == NULL || thisTimer->tc == NULL || thisTimer->tcNumber != t)
		return;
	#endif
	TcCount16* tc = thisTimer->tc;

	uint intflags = tc->INTFLAG.reg;

	// channel 0 compare
	if (intflags & TC_INTFLAG_MC0) {
		// clear the interrupt flag
		tc->INTFLAG.reg = TC_INTFLAG_MC0;
		// update the counter with the next compare value, roll-over is ok
		tc->CC[0].bit.CC += thisTimer->compareValueCh0;
		// call the user's timer interrupt handler
		if (thisTimer->timerCallbackCh0 != NULL)
			thisTimer->timerCallbackCh0();
	}

	// channel 1 compare
	if (intflags & TC_INTFLAG_MC1) {
		tc->INTFLAG.reg = TC_INTFLAG_MC1;
		tc->CC[1].bit.CC += thisTimer->compareValueCh1;
		if (thisTimer->timerCallbackCh1 != NULL)
			thisTimer->timerCallbackCh1();
	}
}

#else
	#error THIS LIBRARY IS ONLY FOR SAMD51 DEVICES

#endif // #ifdef _SAMD51_

