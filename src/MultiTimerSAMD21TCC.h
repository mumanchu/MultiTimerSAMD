#pragma once

// SAMD21 Hardware Timer Interrupt Library for TCCx Timer Counter for Control
// muman.ch, 2026.03.15
// https://github.com/mumanchu/MultiTimerSAMD

#include <Arduino.h>
#include "MumanchuDebug.h"

#ifdef _SAMD21_

class MultiTimerSAMD21TCC
{
protected:
	Tcc* tcc = NULL;			// this timer's APB base address
	int numberOfChannels = -1;

public:
	MultiTimerSAMD21TCC(Tcc* tcc) : tcc(tcc) { }

	bool begin(float frequency, void (*timerCallback)());
	bool beginChannel(uint channel, float frequency, void (*timerCallback)());

	void enable()  { tcc->CTRLA.bit.ENABLE = 1; while (tcc->SYNCBUSY.bit.ENABLE); }
	void disable() { tcc->CTRLA.bit.ENABLE = 0; while (tcc->SYNCBUSY.bit.ENABLE); }

	bool interruptPending(uint channel = 0) {
		return (tcc->INTFLAG.reg & (TCC_INTFLAG_MC0 << channel)) != 0; }

	// Only declare the data if we need it
	#undef TIMER_SAMD21_TCC_DEFINED
	#ifdef USING_TIMER_SAMD21_TCC0
		#define TIMER_SAMD21_TCC_DEFINED
		static uint compareValueTCC0CH0;
		static void (*timerCallbackTCC0CH0)();
		#ifdef USING_TIMER_SAMD21_TCC0_CH1
			static uint compareValueTCC0CH1;
			static void (*timerCallbackTCC0CH1)();
		#endif
		#ifdef USING_TIMER_SAMD21_TCC0_CH2
			static uint compareValueTCC0CH2;
			static void (*timerCallbackTCC0CH2)();
		#endif
		#ifdef USING_TIMER_SAMD21_TCC0_CH3
			static uint compareValueTCC0CH3;
			static void (*timerCallbackTCC0CH3)();
		#endif
	#endif
	#ifdef USING_TIMER_SAMD21_TCC1
		#define TIMER_SAMD21_TCC_DEFINED
		static uint compareValueTCC1CH0;
		static void (*timerCallbackTCC1CH0)();
		#ifdef USING_TIMER_SAMD21_TCC1_CH1
			static uint compareValueTCC1CH1;
			static void (*timerCallbackTCC1CH1)();
		#endif
	#endif
	#ifdef USING_TIMER_SAMD21_TCC2
		#define TIMER_SAMD21_TCC_DEFINED
		static uint compareValueTCC2CH0;
		static void (*timerCallbackTCC2CH0)();
		#ifdef USING_TIMER_SAMD21_TCC2_CH1
			static uint compareValueTCC2CH1;
			static void (*timerCallbackTCC2CH1)();
		#endif
	#endif

protected:
	bool calculatePrescalerAndCompareValue(float frequency, uint counterWidth,
		uint* prescaler, uint* compareValue);
	static const ushort prescalers[8];
};


// Configure the timer, channel 0
// note: call enable() to start the timer after adding additional channels
bool MultiTimerSAMD21TCC::begin(float frequency, void (*timerCallback)())
{
	ASSERT(tcc != NULL && timerCallback != NULL);
	ASSERT(sizeof(uint) == 4);

	// calculate the prescaler and compare values for the desired frequency
	uint counterSize = tcc == TCC2 ? 16 : 24;	// 16 or 24 bit counter
	uint prescaler;
	uint compareValue;
	if (!calculatePrescalerAndCompareValue(frequency, counterSize, &prescaler, &compareValue)) {
		LOGERROR("unsupported frequency");
		return false;
	}

	IRQn_Type irqn;			// interrupt request number for timer
	uint gclkid;			// generic clock multiplexer id
	uint pmapbcmask;		// power manager APB clock enable mask

	// only generate the code if we need it
	if (0) { }
	#ifdef USING_TIMER_SAMD21_TCC0
	else if (tcc == TCC0) {
		numberOfChannels = TCC0_CC_NUM;
		compareValueTCC0CH0 = compareValue;
		timerCallbackTCC0CH0 = timerCallback;
		irqn = TCC0_IRQn;
		gclkid = GCM_TCC0_TCC1;
		pmapbcmask = PM_APBCMASK_TCC0;
	}
	#endif
	#ifdef USING_TIMER_SAMD21_TCC1
	else if (tcc == TCC1) {
		numberOfChannels = TCC1_CC_NUM;
		compareValueTCC1CH0 = compareValue;
		timerCallbackTCC1CH0 = timerCallback;
		irqn = TCC1_IRQn;
		gclkid = GCM_TCC0_TCC1;
		pmapbcmask = PM_APBCMASK_TCC1;
	}
	#endif
	#ifdef USING_TIMER_SAMD21_TCC2
	else if (tcc == TCC2) {
		numberOfChannels = TCC2_CC_NUM;
		compareValueTCC2CH0 = compareValue;
		timerCallbackTCC2CH0 = timerCallback;
		irqn = TCC2_IRQn;
		gclkid = GCM_TCC2_TC3;
		pmapbcmask = PM_APBCMASK_TCC2;
	}
	#endif
	else {
		LOGERROR("missing #define USING_TIMER_SAMD21_TCCx or TCCx not present");
		return false;
	}

	// enable (unmask) the peripheral interface clock using the power management peripheral
	// (there are 3 mask registers, A/B/C, the APB'C'MASK register has the timer mask bits)
	PM->APBCMASK.reg |= pmapbcmask;

	// connect GCLK0 to the timer's general clock multiplexer id (gcmid)
	// GCLK0 is the CPU main clock, SAMD21 = 48MHz = F_CPU
	REG_GCLK_CLKCTRL = (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(gclkid));
	while (GCLK->STATUS.bit.SYNCBUSY);

	// disable the timer
	// "The TCC should be disabled before the TCC is reset to avoid undefined behavior"
	disable();

	// reset timer to defaults
	tcc->CTRLA.bit.SWRST = 1;
	while (tcc->CTRLA.bit.SWRST);

	// normal freq mode (not synchronised)
	tcc->WAVE.bit.WAVEGEN = TCC_WAVE_WAVEGEN_NFRQ;

	// prescaler (not synchronised)
	tcc->CTRLA.bit.PRESCALER = prescaler;

	// set initial compare value, channel 0
	tcc->CC[0].bit.CC = compareValue;
	while (tcc->SYNCBUSY.bit.CC0);

	// enable compare interrupt, channel 0
	// calls the TCCx_Handler() 
	tcc->INTENSET.reg = TCC_INTENSET_MC0;

	// enable the NVIC interrupt
	NVIC_ClearPendingIRQ(irqn);
	//NVIC_SetPriority(irqn, 0); // advanced: set the interrupt priority (default = 0)
	NVIC_EnableIRQ(irqn);

	// note: timer is still disabled

	return true;
}

//#if defined(USING_TIMER_SAMD21_TCC0_CH1) || defined(USING_TIMER_SAMD21_TCC0_CH2) || \
//	defined(USING_TIMER_SAMD21_TCC0_CH3) || defined(USING_TIMER_SAMD21_TCC1_CH1) || \
//	defined(USING_TIMER_SAMD21_TCC2_CH1)

// Configure an additional compare channel (1..3) for a different frequency and interrupt
// 24-bit TCC0 has 4 channels, you can add channels 1..3
// 24-bit TCC1 has 2 channels, you can add channel 1
// 16-bit TCC2 has 2 channels, you can add channel 1
// the frequency must use the same prescaler as channel 0 
bool MultiTimerSAMD21TCC::beginChannel(uint channel, float frequency, void (*timerCallback)())
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
	uint compareValue = (uint)(t / MultiTimerSAMD21TCC::prescalers[currentPrescaler]);
	uint maxCount = (tcc == TCC2) ? 0x00010000 : 0x01000000;
	if (compareValue > maxCount) {
		LOGERROR("unsupported frequency");
		return false;
	}

	// initialise the channel's static compareValue and timerCallback values
	// these are used by the interrupt handler
	// as usual, only generate the code if we need it
	if (0) { }
	#ifdef USING_TIMER_SAMD21_TCC0
	else if (tcc == TCC0) {
		switch (channel) {
		#ifdef USING_TIMER_SAMD21_TCC0_CH1
		case 1:
			compareValueTCC0CH1 = compareValue;
			timerCallbackTCC0CH1 = timerCallback;
			break;
		#endif
		#ifdef USING_TIMER_SAMD21_TCC0_CH2
		case 2:
			compareValueTCC0CH2 = compareValue;
			timerCallbackTCC0CH2 = timerCallback;
			break;
		#endif
		#ifdef USING_TIMER_SAMD21_TCC0_CH3
		case 3:
			compareValueTCC0CH3 = compareValue;
			timerCallbackTCC0CH3 = timerCallback;
			break;
		#endif
		default:
			LOGERROR("missing #define USING_TIMER_SAMD21_TCC0_CHx");
			return false;
		}
	}
	#endif
	#ifdef USING_TIMER_SAMD21_TCC1
	else if (tcc == TCC1) {
		switch (channel) {
		#ifdef USING_TIMER_SAMD21_TCC1_CH1
		case 1:
			compareValueTCC1CH1 = compareValue;
			timerCallbackTCC1CH1 = timerCallback;
			break;
		#endif
		default:
			LOGERROR("missing #define USING_TIMER_SAMD21_TCC1_CH1");
			return false;
		}
	}
	#endif
	#ifdef USING_TIMER_SAMD21_TCC2
	else if (tcc == TCC2) {
		switch (channel) {
		#ifdef USING_TIMER_SAMD21_TCC2_CH1
		case 1:
			compareValueTCC2CH1 = compareValue;
			timerCallbackTCC2CH1 = timerCallback;
			break;
		#endif
		default:
			LOGERROR("missing #define USING_TIMER_SAMD21_TCC2_CH1");
			return false;
		}
	}
	#endif
	else {
		LOGERROR("missing #define USING_TIMER_SAMD21_TCCx");
		return false;
	}

	// set the channel's initial compare value
	tcc->CC[channel].reg = compareValue;
	while (tcc->SYNCBUSY.reg);

	// enable the channel's compare interrupt
	tcc->INTENSET.reg = TCC_INTENSET_MC0 << channel;

	return true;
}
//#endif


// Calculate the prescaler and compare values
// returns false if the timer cannot be configured for this frequency
// (assumes GCLK0, the MCU clock frequency F_CPU)
// counterWidth : 16 or 24 bits
bool MultiTimerSAMD21TCC::calculatePrescalerAndCompareValue(float frequency, 
	uint counterSize, uint* prescaler, uint* compareValue)
{
	*prescaler = 0;
	*compareValue = 0;
	ASSERT(counterSize == 16 || counterSize == 24);

	uint maxCount = 1 << counterSize;
	float t = (float)F_CPU / frequency;		// F_CPU == SystemCoreClock
	for (uint i = 0; i < 8; ++i) {
		uint count = (uint)(t / prescalers[i]);
		if (count <= maxCount) {
			*prescaler = i;
			*compareValue = count;
			return true;
		}
	}
	return false;
}
const ushort MultiTimerSAMD21TCC::prescalers[8] = { 1, 2, 4, 8, 16, 64, 256, 1024 };


// Only declare the data and interrupt handlers if we need them

#ifdef USING_TIMER_SAMD21_TCC0
	uint MultiTimerSAMD21TCC::compareValueTCC0CH0;
	void (*MultiTimerSAMD21TCC::timerCallbackTCC0CH0)();
	#ifdef USING_TIMER_SAMD21_TCC0_CH1
		uint MultiTimerSAMD21TCC::compareValueTCC0CH1;
		void (*MultiTimerSAMD21TCC::timerCallbackTCC0CH1)();
	#endif
	#ifdef USING_TIMER_SAMD21_TCC0_CH2
		uint MultiTimerSAMD21TCC::compareValueTCC0CH2;
		void (*MultiTimerSAMD21TCC::timerCallbackTCC0CH2)();
	#endif
	#ifdef USING_TIMER_SAMD21_TCC0_CH3
		uint MultiTimerSAMD21TCC::compareValueTCC0CH3;
		void (*MultiTimerSAMD21TCC::timerCallbackTCC0CH3)();
	#endif
#endif
#ifdef USING_TIMER_SAMD21_TCC1
	uint MultiTimerSAMD21TCC::compareValueTCC1CH0;
	void (*MultiTimerSAMD21TCC::timerCallbackTCC1CH0)();
	#ifdef USING_TIMER_SAMD21_TCC1_CH1
		uint MultiTimerSAMD21TCC::compareValueTCC1CH1;
		void (*MultiTimerSAMD21TCC::timerCallbackTCC1CH1)();
	#endif
#endif
#ifdef USING_TIMER_SAMD21_TCC2
	uint MultiTimerSAMD21TCC::compareValueTCC2CH0;
	void (*MultiTimerSAMD21TCC::timerCallbackTCC2CH0)();
	#ifdef USING_TIMER_SAMD21_TCC2_CH1
		uint MultiTimerSAMD21TCC::compareValueTCC2CH1;
		void (*MultiTimerSAMD21TCC::timerCallbackTCC2CH1)();
	#endif
#endif


// Timer interrupt handlers, TCx_Handler(), only if needed
// these have fixed names according to the timer
// they override the "weak" definitions which call the Dummy_Handler() alias
// the same handler is called for all compare channels on that timer

#ifdef USING_TIMER_SAMD21_TCC0
	void TCC0_Handler()
	{
		uint intflags = TCC0->INTFLAG.reg;

		// channel 0 is always used
		if (intflags & TCC_INTFLAG_MC0) {
			// clear the interrupt flag
			TCC0->INTFLAG.reg = TCC_INTFLAG_MC0;
			// update the counter with the next compare value, roll-over is ok
			TCC0->CC[0].bit.CC += MultiTimerSAMD21TCC::compareValueTCC0CH0;
			// call the user's timer interrupt handler
			if (MultiTimerSAMD21TCC::timerCallbackTCC0CH0)
				MultiTimerSAMD21TCC::timerCallbackTCC0CH0();
		}

		// channels 1..3 optional
		#ifdef USING_TIMER_SAMD21_TCC0_CH1
		if (intflags & TCC_INTFLAG_MC1) {
			TCC0->INTFLAG.reg = TCC_INTFLAG_MC1;
			TCC0->CC[1].bit.CC += MultiTimerSAMD21TCC::compareValueTCC0CH1;
			if (MultiTimerSAMD21TCC::timerCallbackTCC0CH1)
				MultiTimerSAMD21TCC::timerCallbackTCC0CH1();
		}
		#endif
		#ifdef USING_TIMER_SAMD21_TCC0_CH2
		if (intflags & TCC_INTFLAG_MC2) {
			TCC0->INTFLAG.reg = TCC_INTFLAG_MC2;
			TCC0->CC[2].bit.CC += MultiTimerSAMD21TCC::compareValueTCC0CH2;
			if (MultiTimerSAMD21TCC::timerCallbackTCC0CH2)
				MultiTimerSAMD21TCC::timerCallbackTCC0CH2();
		}
		#endif
		#ifdef USING_TIMER_SAMD21_TCC0_CH3
		if (intflags & TCC_INTFLAG_MC3) {
			TCC0->INTFLAG.reg = TCC_INTFLAG_MC3;
			TCC0->CC[3].bit.CC += MultiTimerSAMD21TCC::compareValueTCC0CH3;
			if (MultiTimerSAMD21TCC::timerCallbackTCC0CH3)
				MultiTimerSAMD21TCC::timerCallbackTCC0CH3();
		}
		#endif
	}
#endif

#ifdef USING_TIMER_SAMD21_TCC1
	void TCC1_Handler()
	{
		uint intflags = TCC1->INTFLAG.reg;

		if (intflags & TCC_INTFLAG_MC0) {
			TCC1->INTFLAG.reg = TCC_INTFLAG_MC0;
			TCC1->CC[0].bit.CC += MultiTimerSAMD21TCC::compareValueTCC1CH0;
			if (MultiTimerSAMD21TCC::timerCallbackTCC1CH0)
				MultiTimerSAMD21TCC::timerCallbackTCC1CH0();
		}
		#ifdef USING_TIMER_SAMD21_TCC1_CH1
		if (intflags & TCC_INTFLAG_MC1) {
			TCC1->INTFLAG.reg = TCC_INTFLAG_MC1;
			TCC1->CC[1].bit.CC += MultiTimerSAMD21TCC::compareValueTCC1CH1;
			if (MultiTimerSAMD21TCC::timerCallbackTCC1CH1)
				MultiTimerSAMD21TCC::timerCallbackTCC1CH1();
		}
		#endif
	}
#endif

#ifdef USING_TIMER_SAMD21_TCC2
	void TCC2_Handler()
	{
		uint intflags = TCC2->INTFLAG.reg;

		if (intflags & TCC_INTFLAG_MC0) {
			TCC2->INTFLAG.reg = TCC_INTFLAG_MC0;
			TCC2->CC[0].bit.CC += MultiTimerSAMD21TCC::compareValueTCC2CH0;
			if (MultiTimerSAMD21TCC::timerCallbackTCC2CH0)
				MultiTimerSAMD21TCC::timerCallbackTCC2CH0();
		}
		#ifdef USING_TIMER_SAMD21_TCC2_CH1
		if (intflags & TCC_INTFLAG_MC1) {
			TCC2->INTFLAG.reg = TCC_INTFLAG_MC1;
			TCC2->CC[1].bit.CC += MultiTimerSAMD21TCC::compareValueTCC2CH1;
			if (MultiTimerSAMD21TCC::timerCallbackTCC2CH1)
				MultiTimerSAMD21TCC::timerCallbackTCC2CH1();
		}
		#endif
	}
#endif

#else
	#error THIS LIBRARY IS ONLY FOR SAMD21 DEVICES

#endif // #ifdef _SAMD21_

