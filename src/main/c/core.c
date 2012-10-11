#include <util/atomic.h>
#include "core.h"

//INIT

void init()
{
	//Enable interrupts
	sei();

	// on the ATmega168, timer 0 is also used for fast hardware pwm
	// (using phase-correct PWM would mean that timer 0 overflowed half as often
	// resulting in different millis() behavior on the ATmega8 and ATmega168)
	sbi(TCCR0A, WGM01);
	sbi(TCCR0A, WGM00);

	// set timer 0 prescale factor to 64
	sbi(TCCR0B, CS01);
	sbi(TCCR0B, CS00);

	// enable timer 0 overflow interrupt
	sbi(TIMSK0, TOIE0);

	// timers 1 and 2 are used for phase-correct hardware pwm
	// this is better for motors as it ensures an even waveform
	// note, however, that fast pwm mode can achieve a frequency of up
	// 8 MHz (with a 16 MHz clock) at 50% duty cycle

	TCCR1B = 0;

	// set timer 1 prescale factor to 64
	sbi(TCCR1B, CS11);
	sbi(TCCR1B, CS10);


	// put timer 1 in 8-bit phase correct pwm mode
	sbi(TCCR1A, WGM10);

	// set timer 2 prescale factor to 64
	sbi(TCCR2B, CS22);

	// configure timer 2 for phase correct pwm (8-bit)
	sbi(TCCR2A, WGM20);


	// set a2d prescale factor to 128
	// 16 MHz / 128 = 125 KHz, inside the desired 50-200 KHz range.
	// XXX: this will not work properly for other clock speeds, and
	// this code should use F_CPU to determine the prescale factor.
	sbi(ADCSRA, ADPS2);
	sbi(ADCSRA, ADPS1);
	sbi(ADCSRA, ADPS0);

	// enable a2d conversions
	sbi(ADCSRA, ADEN);

	// the bootloader connects pins 0 and 1 to the USART; disconnect them
	// here so they can be used as normal digital i/o; they will be
	// reconnected in Serial.begin()
	UCSR0B = 0;
}

//TIME

volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;

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


// DIGITAL

void digital_write_port_d(uint8_t pin, uint8_t val) {
	ATOMIC_BLOCK ( ATOMIC_RESTORESTATE ) {
		if (val == PIN_LOW) {
			PORTD &= ~_BV(pin);
		} else {
			PORTD |= _BV(pin);
		}
	}
}

uint8_t digital_read_port_d(uint8_t pin) {
	if (PIND & _BV(pin)) {
		return PIN_HIGH;
	}
	return PIN_LOW;
}

void set_pin_mode_output_port_d(uint8_t pin) {
	ATOMIC_BLOCK ( ATOMIC_RESTORESTATE ) {
		DDRD |= _BV(pin);
	}
}
