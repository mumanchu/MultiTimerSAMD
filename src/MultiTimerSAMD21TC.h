#pragma once

// SAMD21 Hardware Timer Interrupt Library for TC3..TC7 Timer/Counters
// muman.ch, 2026.03.15
// https://github.com/mumanchu/MultiTimerSAMD

#include <Arduino.h>
#include "MumanchuDebug.h"

#ifdef _SAMD21_

class MultiTimerSAMD21TC
{
public:
	TcCount16* tc;		// this TC timer's APB base address

	MultiTimerSAMD21TC(Tc* tc) : tc((TcCount16*)tc) { }

	bool begin(float frequency, void (*timerCallback)());
	bool beginChannel(uint channel, float frequency, void (*timerCallback)());

	void enable()  { tc->CTRLA.bit.ENABLE = 1; while (tc->STATUS.bit.SYNCBUSY); }
	void disable() { tc->CTRLA.bit.ENABLE = 0; while (tc->STATUS.bit.SYNCBUSY); }

	bool interruptPending(uint channel = 0) {
		return (tc->INTFLAG.reg & (channel ? TC_INTFLAG_MC1 : TC_INTFLAG_MC0)) != 0; 
	}

	// Only declare the data if we need it
	//#undef TIMER_SAMD21_CH1_DEFINED
	#ifdef USING_TIMER_SAMD21_TC3
		static uint compareValueTC3CH0;
		static void (*timerCallbackTC3CH0)();
		#ifdef USING_TIMER_SAMD21_TC3_CH1
			//#define TIMER_SAMD21_CH1_DEFINED
			static uint compareValueTC3CH1;
			static void (*timerCallbackTC3CH1)();
		#endif
	#endif
	#ifdef USING_TIMER_SAMD21_TC4
		static uint compareValueTC4CH0;
		static void (*timerCallbackTC4CH0)();
		#ifdef USING_TIMER_SAMD21_TC4_CH1
			//#define TIMER_SAMD21_CH1_DEFINED
			static uint compareValueTC4CH1;
			static void (*timerCallbackTC4CH1)();
		#endif
	#endif
	#ifdef USING_TIMER_SAMD21_TC5
		static uint compareValueTC5CH0;
		static void (*timerCallbackTC5CH0)();
		#ifdef USING_TIMER_SAMD21_TC5_CH1
			//#define TIMER_SAMD21_CH1_DEFINED
			static uint compareValueTC5CH1;
			static void (*timerCallbackTC5CH1)();
		#endif
	#endif
	#if defined(USING_TIMER_SAMD21_TC6)
		#ifndef TC6
			#error TC6 NOT PRESENT
		#else
			static uint compareValueTC6CH0;
			static void (*timerCallbackTC6CH0)();
			#ifdef USING_TIMER_SAMD21_TC6_CH1
				//#define TIMER_SAMD21_CH1_DEFINED
				static uint compareValueTC6CH1;
				static void (*timerCallbackTC6CH1)();
			#endif
		#endif
	#endif
	#if defined(USING_TIMER_SAMD21_TC7)
		#ifndef TC7
			#error TC7 NOT PRESENT
		#else
			static uint compareValueTC7CH0;
			static void (*timerCallbackTC7CH0)();
			#ifdef USING_TIMER_SAMD21_TC7_CH1
				//#define TIMER_SAMD21_CH1_DEFINED
				static uint compareValueTC7CH1;
				static void (*timerCallbackTC7CH1)();
			#endif
		#endif
	#endif

protected:
	bool calculatePrescalerAndCompareValue(float frequency,
		uint* prescaler, uint* compareValue);
	static const ushort prescalers[8];
};

// Only declare the data if we need it
#ifdef USING_TIMER_SAMD21_TC3
	uint MultiTimerSAMD21TC::compareValueTC3CH0;
	void (*MultiTimerSAMD21TC::timerCallbackTC3CH0)() = NULL;
	#ifdef USING_TIMER_SAMD21_TC3_CH1
		uint MultiTimerSAMD21TC::compareValueTC3CH1;
		void (*MultiTimerSAMD21TC::timerCallbackTC3CH1)() = NULL;
	#endif
#endif
#ifdef USING_TIMER_SAMD21_TC4
	uint MultiTimerSAMD21TC::compareValueTC4CH0;
	void (*MultiTimerSAMD21TC::timerCallbackTC4CH0)() = NULL;
	#ifdef USING_TIMER_SAMD21_TC4_CH1
		uint MultiTimerSAMD21TC::compareValueTC4CH1;
		void (*MultiTimerSAMD21TC::timerCallbackTC4CH1)() = NULL;
	#endif
#endif
#ifdef USING_TIMER_SAMD21_TC5
	uint MultiTimerSAMD21TC::compareValueTC5CH0;
	void (*MultiTimerSAMD21TC::timerCallbackTC5CH0)() = NULL;
	#ifdef USING_TIMER_SAMD21_TC5_CH1
		uint MultiTimerSAMD21TC::compareValueTC5CH1;
		void (*MultiTimerSAMD21TC::timerCallbackTC5CH1)() = NULL;
	#endif
#endif
#if defined(USING_TIMER_SAMD21_TC6) && defined(TC6)
	uint MultiTimerSAMD21TC::compareValueTC6CH0;
	void (*MultiTimerSAMD21TC::timerCallbackTC6CH0)() = NULL;
	#ifdef USING_TIMER_SAMD21_TC6_CH1
		uint MultiTimerSAMD21TC::compareValueTC6CH1;
		void (*MultiTimerSAMD21TC::timerCallbackTC6CH1)() = NULL;
	#endif
#endif
#if defined(USING_TIMER_SAMD21_TC7) && defined(TC7)
	uint MultiTimerSAMD21TC::compareValueTC7CH0;
	void (*MultiTimerSAMD21TC::timerCallbackTC7CH0)() = NULL;
	#ifdef USING_TIMER_SAMD21_TC7_CH1
		uint MultiTimerSAMD21TC::compareValueTC7CH1;
		void (*MultiTimerSAMD21TC::timerCallbackTC7CH1)() = NULL;
	#endif
#endif


// Configure the timer, channel 0
// note: call enable() to start the timer after the optional beginChannel1()
bool MultiTimerSAMD21TC::begin(float frequency, void (*timerCallback)())
{
	ASSERT(timerCallback != NULL);

	// calculate the prescaler and compare values for the desired frequency
	uint prescaler;
	uint compareValue;
	if (!calculatePrescalerAndCompareValue(frequency, &prescaler, &compareValue)) {
		LOGERROR("unsupported frequency");
		return false;
	}

	uint gclkid;			// generic clock multiplexer id
	IRQn_Type irqn;			// interrupt request number, all channels
	uint pmapbcmask;		// power manager APB clock enable mask

	// only generate the code if we need it
	if (0) { }
	#ifdef USING_TIMER_SAMD21_TC3
	else if (tc == (TcCount16*)TC3) {
		compareValueTC3CH0 = compareValue;
		timerCallbackTC3CH0 = timerCallback;
		gclkid = GCM_TCC2_TC3;
		irqn = TC3_IRQn;
		pmapbcmask = PM_APBCMASK_TC3;
	}
	#endif
	#ifdef USING_TIMER_SAMD21_TC4
	else if (tc == (TcCount16*)TC4) {
		compareValueTC4CH0 = compareValue;
		timerCallbackTC4CH0 = timerCallback;
		gclkid = GCM_TC4_TC5;
		irqn = TC4_IRQn;
		pmapbcmask = PM_APBCMASK_TC4;
	}
	#endif
	#ifdef USING_TIMER_SAMD21_TC5
	else if (tc == (TcCount16*)TC5) {
		compareValueTC5CH0 = compareValue;
		timerCallbackTC5CH0 = timerCallback;
		gclkid = GCM_TC4_TC5;
		irqn = TC5_IRQn;
		pmapbcmask = PM_APBCMASK_TC5;
	}
	#endif
	#if defined(USING_TIMER_SAMD21_TC6) && defined(TC6)
	else if (tc == (TcCount16*)TC6) {
		compareValueTC6CH0 = compareValue;
		timerCallbackTC6CH0 = timerCallback;
		gclkid = GCM_TC6_TC7;
		irqn = TC6_IRQn;
		pmapbcmask = PM_APBCMASK_TC6;
	}
	#endif
	#if defined(USING_TIMER_SAMD21_TC7) && defined(TC7)
	else if (tc == (TcCount16*)TC7) {
		compareValueTC7CH0 = compareValue;
		timerCallbackTC7CH0 = timerCallback;
		gclkid = GCM_TC6_TC7;
		irqn = TC7_IRQn;
		pmapbcmask = PM_APBCMASK_TC7;
	}
	#endif
	else {
		LOGERROR("missing #define USING_TIMER_SAMD21_TCx or TCx not present");
		return false;
	}

	// connect GCLK0 to the timer's general clock multiplexer id (gcmid)
	// GCLK0 is the CPU main clock, SAMD21 = 48MHz = F_CPU
	REG_GCLK_CLKCTRL = (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(gclkid));
	while (GCLK->STATUS.bit.SYNCBUSY);
	
	// enable (unmask) the peripheral interface clock using the power management peripheral
	// (there are 3 mask registers, A/B/C, the APB'C'MASK register has the timer mask bits)
	PM->APBCMASK.reg |= pmapbcmask;

	// disable the timer
	// "The TCC should be disabled before the TCC is reset to avoid undefined behavior"
	disable();

	// reset timer to defaults
	tc->CTRLA.bit.SWRST = 1;
	while (tc->CTRLA.bit.SWRST);

	// set 16-bit mode + normal freq mode (not synchronised)
	tc->CTRLA.reg |= (TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_NFRQ);

	// prescaler (not synchronised)
	tc->CTRLA.bit.PRESCALER = prescaler;

	// set compare value, channel 0
	tc->CC[0].reg = compareValue;
	while (tc->STATUS.bit.SYNCBUSY);

	// enable compare interrupt, channel 0
	tc->INTENSET.reg = TC_INTENSET_MC0;

	// enable the NVIC interrupt, all channels
	NVIC_ClearPendingIRQ(irqn);
	//NVIC_SetPriority(irqn, 0); // advanced: set the interrupt priority (default = 0)
	NVIC_EnableIRQ(irqn);

	// note: timer is still disabled

	return true;
}

// Only if we need channel 1
//#ifdef TIMER_SAMD21_CH1_DEFINED

// Configure timer channel 1 for a different frequency and interrupt
// the frequency must use the same prescaler as channel 0 
// note : 'channel' must always be 1, the TC timers only have two channels 0 and 1 
bool MultiTimerSAMD21TC::beginChannel(uint channel, float frequency, void (*timerCallback)())
{
	ASSERT(channel == 1 && timerCallback != NULL);

	uint currentPrescaler = tc->CTRLA.bit.PRESCALER;
	float t = (float)F_CPU / frequency;		// F_CPU == SystemCoreClock
	uint compareValue = (uint)(t / MultiTimerSAMD21TC::prescalers[currentPrescaler]);
	if (compareValue > 65536) {
		LOGERROR("unsupported frequency");
		return false;
	}

	// only generate the code if we need it
	if (0) {}
	#if defined(USING_TIMER_SAMD21_TC3) && defined(USING_TIMER_SAMD21_TC3_CH1)
	else if (tc == (TcCount16*)TC3) {
		compareValueTC3CH1 = compareValue;
		timerCallbackTC3CH1 = timerCallback;
	}
	#endif
	#if defined(USING_TIMER_SAMD21_TC4) && defined(USING_TIMER_SAMD21_TC4_CH1)
	else if (tc == (TcCount16*)TC4) {
		compareValueTC4CH1 = compareValue;
		timerCallbackTC4CH1 = timerCallback;
	}
	#endif
	#if defined(USING_TIMER_SAMD21_TC5) && defined(USING_TIMER_SAMD21_TC5_CH1)
	else if (tc == (TcCount16*)TC5) {
		compareValueTC5CH1 = compareValue;
		timerCallbackTC5CH1 = timerCallback;
	}
	#endif
	#if defined(USING_TIMER_SAMD21_TC6) && defined(USING_TIMER_SAMD21_TC6_CH1) && defined(TC6)
	else if (tc == (TcCount16*)TC6) {
		compareValueTC6CH1 = compareValue;
		timerCallbackTC6CH1 = timerCallback;
	}
	#endif
	#if defined(USING_TIMER_SAMD21_TC7) && defined(USING_TIMER_SAMD21_TC7_CH1) && defined(TC7)
	else if (tc == (TcCount16*)TC7) {
		compareValueTC7CH1 = compareValue;
		timerCallbackTC7CH1 = timerCallback;
	}
	#endif
	else {
		LOGERROR("missing #define USING_TIMER_SAMD21_TCx[_CHx] or TCx not present");
		return false;
	}

	// set initial compare value, channel 1
	tc->CC[1].reg = compareValue;
	while (tc->STATUS.bit.SYNCBUSY);

	// enable compare interrupt, channel 1
	tc->INTENSET.reg = TC_INTENSET_MC1;

	return true;
}
//#endif


// Calculate the prescaler and compare values
// returns false if the timer cannot be configured for this frequency
// (assumes GCLK0, the MCU clock frequency F_CPU)
bool MultiTimerSAMD21TC::calculatePrescalerAndCompareValue(float frequency,
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
const ushort MultiTimerSAMD21TC::prescalers[8] = { 1, 2, 4, 8, 16, 64, 256, 1024 };


// Interrupt handlers, TCx_Handler()
// these have fixed names according to the timer number
// they override the "weak" definitions which call the Dummy_Handler() alias
// the same handler is called for both compare channels, 0 and 1

#ifdef USING_TIMER_SAMD21_TC3
void TC3_Handler() 
{ 
	TcCount16* tc = (TcCount16*)TC3;
	uint intflags = tc->INTFLAG.reg;

	// channel 0 compare
	if (intflags & TC_INTFLAG_MC0) {
		// clear the interrupt flag
		tc->INTFLAG.reg = TC_INTFLAG_MC0;
		// update the counter with the next compare value, roll-over is ok
		tc->CC[0].bit.CC += MultiTimerSAMD21TC::compareValueTC3CH0;
		// call the user's timer interrupt handler
		if (MultiTimerSAMD21TC::timerCallbackTC3CH0 != NULL)
			MultiTimerSAMD21TC::timerCallbackTC3CH0();
	}

	#ifdef USING_TIMER_SAMD21_TC3_CH1
	// channel 1 compare
	if (intflags & TC_INTFLAG_MC1) {
		tc->INTFLAG.reg = TC_INTFLAG_MC1;
		tc->CC[1].bit.CC += MultiTimerSAMD21TC::compareValueTC3CH1;
		if (MultiTimerSAMD21TC::timerCallbackTC3CH1 != NULL)
			MultiTimerSAMD21TC::timerCallbackTC3CH1();
	}
	#endif
}
#endif

#ifdef USING_TIMER_SAMD21_TC4
void TC4_Handler() 
{
	TcCount16* tc = (TcCount16*)TC4;
	uint intflags = tc->INTFLAG.reg;
	if (intflags & TC_INTFLAG_MC0) {
		tc->INTFLAG.reg = TC_INTFLAG_MC0;
		tc->CC[0].bit.CC += MultiTimerSAMD21TC::compareValueTC4CH0;
		if (MultiTimerSAMD21TC::timerCallbackTC4CH0 != NULL)
			MultiTimerSAMD21TC::timerCallbackTC4CH0();
	}
	#ifdef USING_TIMER_SAMD21_TC4_CH1
	if (intflags & TC_INTFLAG_MC1) {
		tc->INTFLAG.reg = TC_INTFLAG_MC1;
		tc->CC[1].bit.CC += MultiTimerSAMD21TC::compareValueTC4CH1;
		if (MultiTimerSAMD21TC::timerCallbackTC4CH1 != NULL)
			MultiTimerSAMD21TC::timerCallbackTC4CH1();
	}
	#endif
}
#endif

#ifdef USING_TIMER_SAMD21_TC5
void TC5_Handler() 
{
	TcCount16* tc = (TcCount16*)TC5;
	uint intflags = tc->INTFLAG.reg;
	if (intflags & TC_INTFLAG_MC0) {
		tc->INTFLAG.reg = TC_INTFLAG_MC0;
		tc->CC[0].bit.CC += MultiTimerSAMD21TC::compareValueTC5CH0;
		if (MultiTimerSAMD21TC::timerCallbackTC5CH0 != NULL)
			MultiTimerSAMD21TC::timerCallbackTC5CH0();
	}
	#ifdef USING_TIMER_SAMD21_TC5_CH1
	if (intflags & TC_INTFLAG_MC1) {
		tc->INTFLAG.reg = TC_INTFLAG_MC1;
		tc->CC[1].bit.CC += MultiTimerSAMD21TC::compareValueTC5CH1;
		if (MultiTimerSAMD21TC::timerCallbackTC5CH1 != NULL)
			MultiTimerSAMD21TC::timerCallbackTC5CH1();
	}
	#endif
}
#endif

#if defined(USING_TIMER_SAMD21_TC6) && defined(TC6)
void TC6_Handler() 
{
	TcCount16* tc = (TcCount16*)TC6;
	uint intflags = tc->INTFLAG.reg;
	if (intflags & TC_INTFLAG_MC0) {
		tc->INTFLAG.reg = TC_INTFLAG_MC0;
		tc->CC[0].bit.CC += MultiTimerSAMD21TC::compareValueTC6CH0;
		if (MultiTimerSAMD21TC::timerCallbackTC6CH0 != NULL)
			MultiTimerSAMD21TC::timerCallbackTC6CH0();
	}
	#ifdef USING_TIMER_SAMD21_TC6_CH1
	if (intflags & TC_INTFLAG_MC1) {
		tc->INTFLAG.reg = TC_INTFLAG_MC1;
		tc->CC[1].bit.CC += MultiTimerSAMD21TC::compareValueTC6CH1;
		if (MultiTimerSAMD21TC::timerCallbackTC6CH1 != NULL)
			MultiTimerSAMD21TC::timerCallbackTC6CH1();
	}
	#endif
}
#endif

#if defined(USING_TIMER_SAMD21_TC7) && defined(TC7)
void TC7_Handler() 
{ 
	TcCount16* tc = (TcCount16*)TC7;
	uint intflags = tc->INTFLAG.reg;
	if (intflags & TC_INTFLAG_MC0) {
		tc->INTFLAG.reg = TC_INTFLAG_MC0;
		tc->CC[0].bit.CC += MultiTimerSAMD21TC::compareValueTC7CH0;
		if (MultiTimerSAMD21TC::timerCallbackTC7CH0 != NULL)
			MultiTimerSAMD21TC::timerCallbackTC7CH0();
	}
	#ifdef USING_TIMER_SAMD21_TC7_CH1
	if (intflags & TC_INTFLAG_MC1) {
		tc->INTFLAG.reg = TC_INTFLAG_MC1;
		tc->CC[1].bit.CC += MultiTimerSAMD21TC::compareValueTC7CH1;
		if (MultiTimerSAMD21TC::timerCallbackTC7CH1 != NULL)
			MultiTimerSAMD21TC::timerCallbackTC7CH1();
	}
	#endif
}
#endif

#else
	#error THIS LIBRARY IS ONLY FOR SAMD21 DEVICES

#endif // #ifdef _SAMD21_

