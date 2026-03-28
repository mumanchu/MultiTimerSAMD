#pragma once

// SAMD51 Hardware Timer Interrupt Library for TCCx Timer Counter for Control
// Copyright (C) muman.ch, 2026.03.15
// All rights reversed
// https://github.com/mumanch/MultiTimerSAMD

#include <Arduino.h>
#include "MattLabsLibDebug.h"


#ifdef _SAMD51_

class MultiTimerSAMD51TCC
{
public:
	Tcc* tcc;				// this timer's APB base address
	IRQn_Type irqnCh0;		// interrupt request number for channel 0
	int numberOfChannels = -1;

	MultiTimerSAMD51TCC(Tcc* tcc) : tcc(tcc) { }

	bool begin(float frequency, void (*timerCallback)());
	bool beginChannel(uint channel, float frequency, void (*timerCallback)());

	bool interruptPending(uint channel = 0) {
		return (tcc->INTFLAG.reg & (TCC_INTFLAG_MC0 << (channel & 0x07))) != 0;
	}
	void enable()  { tcc->CTRLA.bit.ENABLE = 1; while (tcc->SYNCBUSY.bit.ENABLE); }
	void disable() { tcc->CTRLA.bit.ENABLE = 0; while (tcc->SYNCBUSY.bit.ENABLE); }

	// Only declare the data if it is needed

	#undef TIMER_SAMD51_TCC_DEFINED
	#ifdef USING_TIMER_SAMD51_TCC0
		#define TIMER_SAMD51_TCC_DEFINED
		static uint compareValueTCC0CH0;
		static void (*timerCallbackTCC0CH0)();
		#ifdef USING_TIMER_SAMD51_TCC0_CH1
			static uint compareValueTCC0CH1;
			static void (*timerCallbackTCC0CH1)();
		#endif
		#ifdef USING_TIMER_SAMD51_TCC0_CH2
			static uint compareValueTCC0CH2;
			static void (*timerCallbackTCC0CH2)();
		#endif
		#ifdef USING_TIMER_SAMD51_TCC0_CH3
			static uint compareValueTCC0CH3;
			static void (*timerCallbackTCC0CH3)();
		#endif
		#ifdef USING_TIMER_SAMD51_TCC0_CH4
			static uint compareValueTCC0CH4;
			static void (*timerCallbackTCC0CH4)();
		#endif
		#ifdef USING_TIMER_SAMD51_TCC0_CH5
			static uint compareValueTCC0CH5;
			static void (*timerCallbackTCC0CH5)();
		#endif
	#endif

	#ifdef USING_TIMER_SAMD51_TCC1
		#define TIMER_SAMD51_TCC_DEFINED
		static uint compareValueTCC1CH0;
		static void (*timerCallbackTCC1CH0)();
		#ifdef USING_TIMER_SAMD51_TCC1_CH1
			static uint compareValueTCC1CH1;
			static void (*timerCallbackTCC1CH1)();
		#endif
		#ifdef USING_TIMER_SAMD51_TCC1_CH2
			static uint compareValueTCC1CH2;
			static void (*timerCallbackTCC1CH2)();
		#endif
		#ifdef USING_TIMER_SAMD51_TCC1_CH3
			static uint compareValueTCC1CH3;
			static void (*timerCallbackTCC1CH3)();
		#endif
	#endif

	#ifdef USING_TIMER_SAMD51_TCC2
		#define TIMER_SAMD51_TCC_DEFINED
		static uint compareValueTCC2CH0;
		static void (*timerCallbackTCC2CH0)();
		#ifdef USING_TIMER_SAMD51_TCC2_CH1
			static uint compareValueTCC2CH1;
			static void (*timerCallbackTCC2CH1)();
		#endif
		#ifdef USING_TIMER_SAMD51_TCC2_CH2
			static uint compareValueTCC2CH2;
			static void (*timerCallbackTCC2CH2)();
		#endif
	#endif

	#ifdef USING_TIMER_SAMD51_TCC3
		#ifndef TCC3
			#error TCC3 NOT PRESENT
		#else
			#define TIMER_SAMD51_TCC_DEFINED
			static uint compareValueTCC3CH0;
			static void (*timerCallbackTCC3CH0)();
			#ifdef USING_TIMER_SAMD51_TCC3_CH1
				static uint compareValueTCC3CH1;
				static void (*timerCallbackTCC3CH1)();
			#endif
		#endif
	#endif

	#ifdef USING_TIMER_SAMD51_TCC4
		#ifndef TCC4
			#error TCC4 NOT PRESENT
		#else
			#define TIMER_SAMD51_TCC_DEFINED
			static uint compareValueTCC4CH0;
			static void (*timerCallbackTCC4CH0)();
			#ifdef USING_TIMER_SAMD51_TCC4_CH1
				static uint compareValueTCC4CH1;
				static void (*timerCallbackTCC4CH1)();
			#endif
		#endif
	#endif

protected:
	bool calculatePrescalerAndCompareValue(float frequency,
		uint* prescaler, uint* compareValue);
	static const ushort prescalers[8];
};


// Configure the timer, channel 0
bool MultiTimerSAMD51TCC::begin(float frequency, void (*timerCallback)())
{
	ASSERT(timerCallback != NULL);
	ASSERT(sizeof(uint) == 4);

	// calculate the prescaler and compare values for the desired frequency
	uint prescaler;
	uint compareValue;
	if (!calculatePrescalerAndCompareValue(frequency, &prescaler, &compareValue)) {
		LOGERROR("unsupported frequency");
		return false;
	}

	uint gclkid;			// generic clock index

	// only generate the code if we need it
	if (0) { }
	#ifdef USING_TIMER_SAMD51_TCC0
	else if (tcc == TCC0) {
		numberOfChannels = TCC0_CC_NUM;
		compareValueTCC0CH0 = compareValue;
		timerCallbackTCC0CH0 = timerCallback;
		gclkid = TCC0_GCLK_ID;
		irqnCh0 = TCC0_1_IRQn;
	}
	#endif
	#ifdef USING_TIMER_SAMD51_TCC1
	else if (tcc == TCC1) {
		numberOfChannels = TCC1_CC_NUM;
		compareValueTCC1CH0 = compareValue;
		timerCallbackTCC1CH0 = timerCallback;
		gclkid = TCC1_GCLK_ID;
		irqnCh0 = TCC1_1_IRQn;
	}
	#endif
	#ifdef USING_TIMER_SAMD51_TCC2
	else if (tcc == TCC2) {
		numberOfChannels = TCC2_CC_NUM;
		compareValueTCC2CH0 = compareValue;
		timerCallbackTCC2CH0 = timerCallback;
		gclkid = TCC2_GCLK_ID;
		irqnCh0 = TCC2_1_IRQn;
	}
	#endif
	#if defined(USING_TIMER_SAMD51_TCC3) && defined(TCC3)
	else if (tcc == TCC3) {
		numberOfChannels = TCC3_CC_NUM;
		compareValueTCC3CH0 = compareValue;
		timerCallbackTCC3CH0 = timerCallback;
		gclkid = TCC3_GCLK_ID;
		irqnCh0 = TCC3_1_IRQn;
	}
	#endif
	#if defined(USING_TIMER_SAMD51_TCC4) && defined(TCC4)
	else if (tcc == TCC4) {
		numberOfChannels = TCC4_CC_NUM;
		compareValueTCC4CH0 = compareValue;
		timerCallbackTCC4CH0 = timerCallback;
		gclkid = TCC4_GCLK_ID;
		irqnCh0 = TCC4_1_IRQn;
	}
	#endif
	else {
		LOGERROR("missing #define USING_TIMER_SAMD51_TCCx ?");
		return false;
	}

	//TODO clock may be disabled to reduce power consumption? e.g. for TCC0
	//MCLK->APBBMASK.reg |= MCLK_APBBMASK_TCC0;

	// peripheral channel control clock
	// connect GCLK0 to the timer's general clock multiplexer id (gcmid)
	// GCLK0 is the CPU main clock, SAMD51 = 120MHz = F_CPU
	GCLK->PCHCTRL[gclkid].reg = (GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN_GCLK0);
	while (GCLK->SYNCBUSY.reg);

	// disable the timer
	// "The TCC should be disabled before the TCC is reset to avoid undefined behavior"
	disable();

	// reset timer to defaults
	tcc->CTRLA.bit.SWRST = 1;
	while (tcc->SYNCBUSY.bit.SWRST);

	// normal freq mode (not synchronised)
	tcc->WAVE.bit.WAVEGEN = TCC_WAVE_WAVEGEN_NFRQ;

	// prescaler (not synchronised)
	tcc->CTRLA.bit.PRESCALER = prescaler;

	// set compare value, channel 0
	tcc->CC[0].reg = compareValue;
	while (tcc->SYNCBUSY.bit.CC0);

	// enable compare interrupt, channel 0
	tcc->INTENSET.reg = TCC_INTENSET_MC0;

	// enable the NVIC interrupt, channel 0
	NVIC_ClearPendingIRQ(irqnCh0);
	//NVIC_SetPriority(irqnCh0, 0); // advanced: set the interrupt priority (default = 0)
	NVIC_EnableIRQ(irqnCh0);

	// note: timer is still disabled

	return true;
}


// Initialise a channel with a different frequency and callback
// TCC0 = 6 channels, 0..5
// TCC1 = 4 channels, 0..3
// TCC2 = 3 channels, 0..2 
// TCC3 = 2 channels, 0..1 (not all SAMD51 devices have TCC3)
// TCC4 = 2 channels, 0..1 (not all SAMD51 devices have TCC4)
bool MultiTimerSAMD51TCC::beginChannel(uint channel, float frequency, void (*timerCallback)())
{
	ASSERT(timerCallback != NULL);

	if (numberOfChannels == -1) {
		LOGERROR("begin() not called");
		return false;
	}
	if (channel == 0 || channel >= numberOfChannels) {
		LOGERROR("invalid channel");
		return false;
	}

	// can this frequency be supported by the shared prescaler?
	// prescaler must be the same, but compare values may be different
	uint currentPrescaler = tcc->CTRLA.bit.PRESCALER;
	float t = (float)F_CPU / frequency;		// F_CPU == SystemCoreClock
	uint compareValue = (uint)(t / MultiTimerSAMD51TCC::prescalers[currentPrescaler]);
	if (compareValue > 65536) {					// 16 bits
		LOGERROR("unsupported frequency");
		return false;
	}

	// initialise the channel's static compareValue and timerCallback values
	// these are used by the interrupt handler
	// as usual, only generate the code if we need it

	if (0) { }	// (so we can use else if)
	#ifdef USING_TIMER_SAMD51_TCC0
	else if (tcc == TCC0) {
		switch (channel) {
		#ifdef USING_TIMER_SAMD51_TCC0_CH1
		case 1:
			compareValueTCC0CH1 = compareValue;
			timerCallbackTCC0CH1 = timerCallback;
			break;
		#endif
		#ifdef USING_TIMER_SAMD51_TCC0_CH2
		case 2:
			compareValueTCC0CH2 = compareValue;
			timerCallbackTCC0CH2 = timerCallback;
			break;
		#endif
		#ifdef USING_TIMER_SAMD51_TCC0_CH3
		case 3:
			compareValueTCC0CH3 = compareValue;
			timerCallbackTCC0CH3 = timerCallback;
			break;
		#endif
		#ifdef USING_TIMER_SAMD51_TCC0_CH4
		case 4:
			compareValueTCC0CH4 = compareValue;
			timerCallbackTCC0CH4 = timerCallback;
			break;
		#endif
		#ifdef USING_TIMER_SAMD51_TCC0_CH5
		case 5:
			compareValueTCC0CH5 = compareValue;
			timerCallbackTCC0CH5 = timerCallback;
			break;
		#endif
		default:
			LOGERROR("missing #define USING_TIMER_SAMD51_TCC0_CHx");
			return false;
		}
	}
	#endif
	#ifdef USING_TIMER_SAMD51_TCC1
	else if (tcc == TCC1) {
		switch (channel) {
		#ifdef USING_TIMER_SAMD51_TCC1_CH1
		case 1:
			compareValueTCC1CH1 = compareValue;
			timerCallbackTCC1CH1 = timerCallback;
			break;
		#endif
		#ifdef USING_TIMER_SAMD51_TCC1_CH2
		case 2:
			compareValueTCC1CH2 = compareValue;;
			timerCallbackTCC1CH2 = timerCallback;
		break;
		#endif
		#ifdef USING_TIMER_SAMD51_TCC1_CH3
		case 3:
			compareValueTCC1CH3 = compareValue;;
			timerCallbackTCC1CH3 = timerCallback;
			break;
		#endif
		default:
			LOGERROR("missing #define USING_TIMER_SAMD51_TCC1_CHx");
			return false;
		}
	}
	#endif
	#ifdef USING_TIMER_SAMD51_TCC2
	else if (tcc == TCC2) {
		switch (channel) {
		#ifdef USING_TIMER_SAMD51_TCC2_CH1
		case 1:
			compareValueTCC2CH1 = compareValue;;
			timerCallbackTCC2CH1 = timerCallback;
			break;
		#endif
		#ifdef USING_TIMER_SAMD51_TCC2_CH2
		case 2:
			compareValueTCC2CH2 = compareValue;;
			timerCallbackTCC2CH2 = timerCallback;
			break;
		#endif
		default:
			LOGERROR("missing #define USING_TIMER_SAMD51_TCC2_CHx");
			return false;
		}
	}
	#endif
	#if defined (USING_TIMER_SAMD51_TCC3) && defined (TCC3)
	else if (tcc == TCC3) {
		switch (channel) {
		#ifdef USING_TIMER_SAMD51_TCC3_CH1
		case 1:
			compareValueTCC3CH1 = compareValue;;
			timerCallbackTCC3CH1 = timerCallback;
			break;
		#endif
		default:
			LOGERROR("missing #define USING_TIMER_SAMD51_TCC3_CH1");
			return false;
		}
	}
	#endif
	#if defined (USING_TIMER_SAMD51_TCC4) && defined (TCC4)
	else if (tcc == TCC4) {
		switch (channel) {
		#ifdef USING_TIMER_SAMD51_TCC4_CH1
		case 1:
			compareValueTCC4CH1 = compareValue;
			timerCallbackTCC4CH1 = timerCallback;
			break;
		#endif
		default:
			LOGERROR("missing #define USING_TIMER_SAMD51_TCC4_CH1");
			return false;
		}
	}
	#endif
	else {
		LOGERROR("missing #define USING_TIMER_SAMD51_TCCx ?");
		return false;
	}

	// set initial channel compare value
	tcc->CC[channel].reg = compareValue;
	while (tcc->SYNCBUSY.reg);

	// enable the channel's NVIC interrupt
	// (irq numbers are consecutive, TCCx_1_IRQn = channel 0)
	IRQn_Type irqnch = (IRQn_Type)(irqnCh0 + channel);
	NVIC_ClearPendingIRQ(irqnch);
	//NVIC_SetPriority(irqnch, 0); // advanced: set the interrupt priority (default = 0)
	NVIC_EnableIRQ(irqnch);

	// enable the channel's compare interrupt
	tcc->INTENSET.reg = TCC_INTENSET_MC0 << channel;

	return true;
}


// Calculate the prescaler and compare values
// returns false if timer cannot be configured for this frequency
// (assumes GCLK0, the MCU clock frequency F_CPU)
bool MultiTimerSAMD51TCC::calculatePrescalerAndCompareValue(float frequency,
	uint* prescaler, uint* compareValue)
{
	*prescaler = 0;
	*compareValue = 0;

	float t = (float)F_CPU / frequency;		// F_CPU == SystemCoreClock
	for (uint i = 0; i < 8; ++i) {
		uint count = (uint)(t / prescalers[i]);
		if (count <= 65536) {
			*prescaler = i;
			*compareValue = count;
			return true;
		}
	}
	return false;
}
const ushort MultiTimerSAMD51TCC::prescalers[8] = { 1, 2, 4, 8, 16, 64, 256, 1024 };


// The interrupt handler macro
// Different code is generated for each channel, because each channel has 
// its own interrupt handler, unlike the SAMD21
// The handler clears the interrupt flag, updates the compare value, and 
// call the user's timer callback function
#define TCC_HANDLER(tcc, channel, intflag, compareValue, timerCallback) \
	if (tcc->INTFLAG.reg & intflag) { \
		tcc->INTFLAG.reg = intflag; \
		tcc->CC[channel].bit.CC += compareValue; \
		if (timerCallback) \
			timerCallback(); \
	}

// Interrupt handlers for each channel, TCCx_x_Handler()
// these have fixed names according to the timer, which override the
// "weak" definitions that call Dummy_Handler() by default
// TCCx_1_Handler() is the interrupt for TCCx channel 0
// TCCx_2_Handler() is the interrupt for TCCx channel 1
// etc

// Only declare the data and interrupt handler if it is used
#ifdef USING_TIMER_SAMD51_TCC0
	uint MultiTimerSAMD51TCC::compareValueTCC0CH0 = 0;
	void (*MultiTimerSAMD51TCC::timerCallbackTCC0CH0)() = NULL;
	void TCC0_1_Handler() { 
		TCC_HANDLER(TCC0, 0, TCC_INTFLAG_MC0, MultiTimerSAMD51TCC::compareValueTCC0CH0, 
			MultiTimerSAMD51TCC::timerCallbackTCC0CH0)
	}
	#ifdef USING_TIMER_SAMD51_TCC0_CH1
		uint MultiTimerSAMD51TCC::compareValueTCC0CH1 = 0;
		void (*MultiTimerSAMD51TCC::timerCallbackTCC0CH1)() = NULL;
		void TCC0_2_Handler() {
			TCC_HANDLER(TCC0, 1, TCC_INTFLAG_MC1, MultiTimerSAMD51TCC::compareValueTCC0CH1, 
				MultiTimerSAMD51TCC::timerCallbackTCC0CH1)
		}
	#endif
	#ifdef USING_TIMER_SAMD51_TCC0_CH2
		uint MultiTimerSAMD51TCC::compareValueTCC0CH2 = 0;
		void (*MultiTimerSAMD51TCC::timerCallbackTCC0CH2)() = NULL;
		void TCC0_3_Handler() {
			TCC_HANDLER(TCC0, 2, TCC_INTFLAG_MC2, MultiTimerSAMD51TCC::compareValueTCC0CH2, 
				MultiTimerSAMD51TCC::timerCallbackTCC0CH2)
		}
	#endif
	#ifdef USING_TIMER_SAMD51_TCC0_CH3
		uint MultiTimerSAMD51TCC::compareValueTCC0CH3 = 0;
		void (*MultiTimerSAMD51TCC::timerCallbackTCC0CH3)() = NULL;
		void TCC0_4_Handler() {
			TCC_HANDLER(TCC0, 3, TCC_INTFLAG_MC3, MultiTimerSAMD51TCC::compareValueTCC0CH3, 
				MultiTimerSAMD51TCC::timerCallbackTCC0CH3)
		}
	#endif
	#ifdef USING_TIMER_SAMD51_TCC0_CH4
		uint MultiTimerSAMD51TCC::compareValueTCC0CH4 = 0;
		void (*MultiTimerSAMD51TCC::timerCallbackTCC0CH4)() = NULL;
		void TCC0_5_Handler() {
			TCC_HANDLER(TCC0, 4, TCC_INTFLAG_MC4, MultiTimerSAMD51TCC::compareValueTCC0CH4,
				MultiTimerSAMD51TCC::timerCallbackTCC0CH4)
		}
	#endif
	#ifdef USING_TIMER_SAMD51_TCC0_CH5
		uint MultiTimerSAMD51TCC::compareValueTCC0CH5 = 0;
		void (*MultiTimerSAMD51TCC::timerCallbackTCC0CH5)() = NULL;
		void TCC0_6_Handler() {
			TCC_HANDLER(TCC0, 5, TCC_INTFLAG_MC5, MultiTimerSAMD51TCC::compareValueTCC0CH5,
				MultiTimerSAMD51TCC::timerCallbackTCC0CH5)
		}
	#endif
#endif

#ifdef USING_TIMER_SAMD51_TCC1
	uint MultiTimerSAMD51TCC::compareValueTCC1CH0 = 0;
	void (*MultiTimerSAMD51TCC::timerCallbackTCC1CH0)() = NULL;
	void TCC1_1_Handler() {
		TCC_HANDLER(TCC1, 0, TCC_INTFLAG_MC0, MultiTimerSAMD51TCC::compareValueTCC1CH0, 
			MultiTimerSAMD51TCC::timerCallbackTCC1CH0)
	}
	#ifdef USING_TIMER_SAMD51_TCC1_CH1
		uint MultiTimerSAMD51TCC::compareValueTCC1CH1 = 0;
		void (*MultiTimerSAMD51TCC::timerCallbackTCC1CH1)() = NULL;
		void TCC1_2_Handler() {
			TCC_HANDLER(TCC1, 1, TCC_INTFLAG_MC1, MultiTimerSAMD51TCC::compareValueTCC1CH1, 
				MultiTimerSAMD51TCC::timerCallbackTCC1CH1)
		}
	#endif
	#ifdef USING_TIMER_SAMD51_TCC1_CH2
		uint MultiTimerSAMD51TCC::compareValueTCC1CH2 = 0;
		void (*MultiTimerSAMD51TCC::timerCallbackTCC1CH2)() = NULL;
		void TCC1_3_Handler() {
			TCC_HANDLER(TCC1, 2, TCC_INTFLAG_MC2, MultiTimerSAMD51TCC::compareValueTCC1CH2, 
				MultiTimerSAMD51TCC::timerCallbackTCC1CH2)
		}
	#endif
	#ifdef USING_TIMER_SAMD51_TCC1_CH3
		uint MultiTimerSAMD51TCC::compareValueTCC1CH3 = 0;
		void (*MultiTimerSAMD51TCC::timerCallbackTCC1CH3)() = NULL;
		void TCC1_4_Handler() {
			TCC_HANDLER(TCC1, 3, TCC_INTFLAG_MC3, MultiTimerSAMD51TCC::compareValueTCC1CH3, 
				MultiTimerSAMD51TCC::timerCallbackTCC1CH3)
		}
	#endif
#endif

#ifdef USING_TIMER_SAMD51_TCC2
	uint MultiTimerSAMD51TCC::compareValueTCC2CH0 = 0;
	void (*MultiTimerSAMD51TCC::timerCallbackTCC2CH0)() = NULL;
	void TCC2_1_Handler() {
		TCC_HANDLER(TCC2, 0, TCC_INTFLAG_MC0, MultiTimerSAMD51TCC::compareValueTCC2CH0, 
			MultiTimerSAMD51TCC::timerCallbackTCC2CH0)
	}
	#ifdef USING_TIMER_SAMD51_TCC2_CH1
		uint MultiTimerSAMD51TCC::compareValueTCC2CH1 = 0;
		void (*MultiTimerSAMD51TCC::timerCallbackTCC2CH1)() = NULL;
		void TCC2_2_Handler() {
			TCC_HANDLER(TCC2, 1, TCC_INTFLAG_MC1, MultiTimerSAMD51TCC::compareValueTCC2CH1, 
				MultiTimerSAMD51TCC::timerCallbackTCC2CH1)
		}
	#endif
	#ifdef USING_TIMER_SAMD51_TCC2_CH2
		uint MultiTimerSAMD51TCC::compareValueTCC2CH2 = 0;
		void MultiTimerSAMD51TCC::(*timerCallbackTCC2CH2)() = NULL;
		void TCC2_3_Handler() {
			TCC_HANDLER(TCC2, 2, TCC_INTFLAG_MC2, MultiTimerSAMD51TCC::compareValueTCC2CH2, 
				MultiTimerSAMD51TCC::timerCallbackTCC2CH2)
		}
	#endif
	#endif

#ifdef USING_TIMER_SAMD51_TCC3
	#ifndef TCC3
		#error TCC3 NOT PRESENT
	#else
		uint MultiTimerSAMD51TCC::compareValueTCC3CH0 = 0;
		void (*MultiTimerSAMD51TCC::timerCallbackTCC3CH0)() = NULL;
		void TCC3_1_Handler() {
			TCC_HANDLER(TCC3, 0, TCC_INTFLAG_MC0, MultiTimerSAMD51TCC::compareValueTCC3CH0, 
				MultiTimerSAMD51TCC::timerCallbackTCC3CH0)
		}
	#endif
	#ifdef USING_TIMER_SAMD51_TCC3_CH1
		uint MultiTimerSAMD51TCC::compareValueTCC3CH1 = 0;
		void (*MultiTimerSAMD51TCC::timerCallbackTCC3CH1)() = NULL;
		void TCC3_2_Handler() {
			TCC_HANDLER(TCC3, 1, TCC_INTFLAG_MC1, MultiTimerSAMD51TCC::compareValueTCC2CH1, 
				MultiTimerSAMD51TCC::timerCallbackTCC2CH1)
		}
	#endif
#endif

#ifdef USING_TIMER_SAMD51_TCC4
	#ifndef TCC4
	#error TCC4 NOT PRESENT
	#else
		uint MultiTimerSAMD51TCC::compareValueTCC4CH0 = 0;
		void (*MultiTimerSAMD51TCC::timerCallbackTCC4CH0)() = NULL;
		void TCC4_1_Handler() {
			TCC_HANDLER(TCC4, 0, TCC_INTFLAG_MC0, MultiTimerSAMD51TCC::compareValueTCC4CH0, 
				MultiTimerSAMD51TCC::timerCallbackTCC4CH0)
		}
	#endif
	#ifdef USING_TIMER_SAMD51_TCC4_CH1
		uint MultiTimerSAMD51TCC::compareValueTCC4CH1 = 0;
		void (*MultiTimerSAMD51TCC::timerCallbackTCC4CH1)() = NULL;
		void TCC4_2_Handler() {
			TCC_HANDLER(TCC4, 1, TCC_INTFLAG_MC1, MultiTimerSAMD51TCC::compareValueTCC4CH1, 
				MultiTimerSAMD51TCC::timerCallbackTCC4CH1)
		}
	#endif
#endif

#else
	#error THIS LIBRARY IS ONLY FOR SAMD51 DEVICES

#endif // #ifdef _SAMD51_

