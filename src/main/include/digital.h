#define PIN_HIGH 0x01
#define PIN_LOW 0x00

void digital_write_port_d(uint8_t pin, uint8_t val);
uint8_t digital_read_port_d(uint8_t pin);
void set_pin_mode_output_port_d(uint8_t pin);

