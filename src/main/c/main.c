#include <util/atomic.h>
#include <avr/sleep.h>

#include "Arduino.h"
#include "uart.h"

#define VERSION	"0.1"

//Physical connectivity [ 8FG(SA) -> Rx(5) -> Arduino(2) ]
#define RX_SIGNAL_PIN	2
#define NAVIGATION_PIN	5
#define LANDING_PIN		4
#define STROBE_PIN		6

//Switch positions (1000=min, 2000=max)
//lo if (rx_signal > X)
#define SWITCH_OFF_THRESHOLD	1800
//hi if (rx_signal < X)
#define SWITCH_ON_THRESHOLD		1200

//Flags to indicate which lighting circuits are on
#define NAVIGATION_ON	1
#define STROBE_ON		2
#define LANDING_ON		4

//Precomputed circuit state flags
//000
#define CIRCUIT_ALL_OFF			0
//011
#define CIRCUIT_NAV_AND_STROBE	3
//111
#define CIRCUIT_ALL_ON			7

//Interrupts: int0 -> digital 2
#define RX_SIGNAL_INT			0

//Channel flags
//Volatile as written by isr routines, read in control loop
volatile uint8_t event_flags_shared;

#define RX_SIGNAL_EVT			1
#define TIMER_1_EVT				2

//Stores length of pulse
//Volatile as written by isr routine, read in control loop
volatile uint16_t rx_signal_shared;

//Stores rising edge
//Non-volatile as only used in isr (consider making static inside isr)
uint32_t rx_signal_start;

#define PIN_HIGH 0x01
#define PIN_LOW 0x00

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

uint16_t get_next_counter() {
	static uint8_t idx = 0;
	static const uint16_t delay[4] = {
			62490, //off: 1000ms
			6249,  //on:  100ms
			6249,  //off: 100ms
			6249   //on:  100ms
	};
	uint16_t next_delay = delay[idx++];
	if (idx >= 4) {
		idx = 0;
	}
	return next_delay;
}

uint8_t get_circuit_state_flags(uint16_t rx_signal) {

	//Everything off
	if (rx_signal < SWITCH_ON_THRESHOLD) {
		return CIRCUIT_ALL_ON;
	}
	if (rx_signal > SWITCH_OFF_THRESHOLD) {
		return CIRCUIT_ALL_OFF;
	}
	return CIRCUIT_NAV_AND_STROBE;

}

uint8_t circuit_state_changed(uint8_t old_flags, uint8_t new_flags, uint8_t flag) {
	return (old_flags & flag) != (new_flags & flag);
}

void core_loop() {

	// Keep local copies to keep critical section short
	// Persist values across loop through use of static
	static uint16_t rx_signal = 0;
	static uint8_t  event_flags = 0;
	static uint8_t	prev_circuit_state = 0;
	static uint8_t	current_circuit_state = 0;

	while( true ) {

		// CRITICAL SECTION
		ATOMIC_BLOCK ( ATOMIC_RESTORESTATE ) {

			if (event_flags_shared) {

				//Critical section while we copy volatiles to local
				event_flags = event_flags_shared;

				if (event_flags & RX_SIGNAL_EVT) {
					rx_signal = rx_signal_shared;
				}

				// Zero shared flags
				event_flags_shared = 0;
			}

		}
		// END CRITICAL SECTION

		if (event_flags & RX_SIGNAL_EVT) {

			current_circuit_state = get_circuit_state_flags(rx_signal);

			//Navigation lights
			if (circuit_state_changed(prev_circuit_state, current_circuit_state, NAVIGATION_ON)) {
				digital_write_port_d(NAVIGATION_PIN, current_circuit_state & NAVIGATION_ON);
			}

			//Landing lights
			if (circuit_state_changed(prev_circuit_state, current_circuit_state, LANDING_ON)) {
				digital_write_port_d(LANDING_PIN, current_circuit_state & LANDING_ON);
			}

		}

		if (event_flags & TIMER_1_EVT) {

			//Strobe lights
			if (current_circuit_state & STROBE_ON) {
				//Flash the circuit
				digital_write_port_d(STROBE_PIN, digital_read_port_d(STROBE_PIN) ^ 1);
			}
			else {
				//Switch off the circuit
				digital_write_port_d(STROBE_PIN, PIN_LOW);
			}

			// Set next counter delay
			cli();
			OCR1A = get_next_counter();
			sei();

		}

		//Update circuit state
		prev_circuit_state = current_circuit_state;

		// React to user input if there is any
		if (uart_available()) {
			char cmd = getchar();
			if (cmd == 'd') {
				printf("Rx signal: %d\n", rx_signal);
			}
		}

		// Zero local flags
		event_flags = 0;

		// Go to sleep until an interrupt occurs, only 'idle' allows awake on pin change
		// Make sure we don't sleep if there is already another event pending
		set_sleep_mode(SLEEP_MODE_IDLE);
		cli();
		if (event_flags_shared) {
			sei();
			sleep_mode();
		}
		else {
			sei();
		}

	} // End while

}

// ISR to handle interrupts on the Rx signal pin
void isr_rx_signal() {

}

void setup_timer() {

	// See ATmega*8P data sheet - section 15.11
	// Nano clock speed is 16Mhz, we want an interrupt at rate of 10Hz
	// CTC compare value = (16Mhz/256/10)-1 = 6249

	// Needs to be explicitly zeroed as Arduino software sets to non-default
	// NB: May affect operation of hardware PWM pins?
	TCCR1A = 0;

	// Configure timer 1 for CTC mode
	TCCR1B |= (1 << WGM12);

	// Enable CTC interrupt and global interrupts
	TIMSK1 |= (1 << OCIE1A);
	sei ();

	// Set CTC compare value
	OCR1A = get_next_counter();

	// Start timer at Fcpu /256 (see ATMega328P, 15.11.2, p136)
	TCCR1B |= (1 << CS12);
	TCCR1B &= ~(1 << CS11);
	TCCR1B &= ~(1 << CS10);

}

// ISR to handle timer interrupts (See ATmega*8P data sheet - section 11.1)
ISR( TIMER1_COMPA_vect) {
	event_flags_shared |= TIMER_1_EVT;
}


void setup_rx_interrupt() {
	//Configure external interrupt 0 to react on change
	#define CHANGE 1
	EICRA = (EICRA & ~((1 << ISC00) | (1 << ISC01))) | (CHANGE << ISC00);
    EIMSK |= (1 << INT0);
}

ISR( INT0_vect ) {
	//TODO: Remove Arduino - micros()
	if (digital_read_port_d(RX_SIGNAL_PIN) == PIN_HIGH) {
		rx_signal_start = micros();
	} else {
		rx_signal_shared = (uint16_t) (micros() - rx_signal_start);
		event_flags_shared |= RX_SIGNAL_EVT;
	}
}

// Main entry point
int main(void) {

	//TODO: Remove Wiring - init()
	init();

	//Initialise logger, redirect stdin/stdout to serial
	uart_init();
	stdout = stdin = &uart_str;

	printf("Navlights version: %s\n", VERSION);

	//Attach Rx interrupt
	setup_rx_interrupt();

	//Attach timer interrupt
	setup_timer();

	//Set pin states
	digital_write_port_d(NAVIGATION_PIN, PIN_LOW);
	digital_write_port_d(LANDING_PIN, PIN_LOW);
	digital_write_port_d(STROBE_PIN, PIN_LOW);

	//Needed to be able to do digital_read_port_d ^ 1
	set_pin_mode_output_port_d(STROBE_PIN);

	//Enter loop
	core_loop();

	return 0;

}
