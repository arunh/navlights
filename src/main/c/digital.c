#include <util/atomic.h>
#include "digital.h"

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
