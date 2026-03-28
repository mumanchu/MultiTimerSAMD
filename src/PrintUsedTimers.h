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


