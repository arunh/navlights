//INIT

void init();

//DIGITAL

#define PIN_HIGH 0x01
#define PIN_LOW 0x00

void digital_write_port_d(uint8_t pin, uint8_t val);
uint8_t digital_read_port_d(uint8_t pin);
void set_pin_mode_output_port_d(uint8_t pin);


//TIME

#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )

#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))
// the whole number of milliseconds per timer0 overflow
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)

// the fractional number of milliseconds per timer0 overflow. we shift right
// by three to fit these numbers into a byte. (for the clock speeds we care
// about - 8 and 16 MHz - this doesn't lose precision.)
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)

uint32_t micros();
