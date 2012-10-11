#include <util/atomic.h>
#include "time.h"

volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;

void init_timer0()
{

	// set timer 0 prescale factor to 64
	sbi(TCCR0B, CS01);
	sbi(TCCR0B, CS00);

	// enable timer 0 overflow interrupt
	sbi(TIMSK0, TOIE0);
}

ISR( TIMER0_OVF_vect) {
	// copy these to local variables so they can be stored in registers
	// (volatile variables must be read from memory on every access)
	unsigned long m = timer0_millis;
	unsigned char f = timer0_fract;

	m += MILLIS_INC;
	f += FRACT_INC;
	if (f >= FRACT_MAX) {
		f -= FRACT_MAX;
		m += 1;
	}

	timer0_fract = f;
	timer0_millis = m;
	timer0_overflow_count++;
}

uint32_t micros() {

	uint32_t m;
	uint8_t t;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		m = timer0_overflow_count;
		t = TCNT0;

		if ((TIFR0 & _BV(TOV0)) && (t < 255))
			m++;
	}

	return ((m << 8) + t) * (64 / clockCyclesPerMicrosecond());
}
