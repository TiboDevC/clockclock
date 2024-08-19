#include <stdint.h>

#include "Arduino.h"

#define PIN_SHIFT_REGISTER_CLOCK   9
#define PIN_STORAGE_REGISTER_CLOCK 10
#define PIN_SERIAL_DATA_OUTPUT     11

#define NUM_PIN_PER_MOTOR 4

void shift_reg_init(void)
{
	pinMode(PIN_SHIFT_REGISTER_CLOCK, OUTPUT);
	pinMode(PIN_STORAGE_REGISTER_CLOCK, OUTPUT);
	pinMode(PIN_SERIAL_DATA_OUTPUT, OUTPUT);

	digitalWrite(PIN_SHIFT_REGISTER_CLOCK, LOW);
	digitalWrite(PIN_STORAGE_REGISTER_CLOCK, LOW);

	delay(10);
	delay(10);
}

void ctrl_test(void)
{
	int bit = 0;
	digitalWrite(PIN_SHIFT_REGISTER_CLOCK, LOW);
	digitalWrite(PIN_STORAGE_REGISTER_CLOCK, LOW);
	delay(10);

	for (;;) {
		if (bit % 4 == 0) {
			digitalWrite(PIN_SERIAL_DATA_OUTPUT, HIGH);
		} else {
			digitalWrite(PIN_SERIAL_DATA_OUTPUT, LOW);
		}

		digitalWrite(PIN_SHIFT_REGISTER_CLOCK, HIGH);
		digitalWrite(PIN_STORAGE_REGISTER_CLOCK, HIGH);

		digitalWrite(PIN_SHIFT_REGISTER_CLOCK, LOW);
		digitalWrite(PIN_STORAGE_REGISTER_CLOCK, LOW);

		delay(100);

		bit++;
	}
}

static void _digitalWrite_data(uint8_t val)
{
	const uint8_t bit = digitalPinToBitMask(PIN_SERIAL_DATA_OUTPUT);
	const uint8_t port = digitalPinToPort(PIN_SERIAL_DATA_OUTPUT);
	volatile uint8_t *out;

	out = portOutputRegister(port);

	uint8_t oldSREG = SREG;
	cli();

	if (val == LOW) {
		*out &= ~bit;
	} else {
		*out |= bit;
	}

	SREG = oldSREG;
}

static void _shift_register_low()
{
	const uint8_t bit = digitalPinToBitMask(PIN_SHIFT_REGISTER_CLOCK);
	const uint8_t port = digitalPinToPort(PIN_SHIFT_REGISTER_CLOCK);
	volatile uint8_t *out = portOutputRegister(port);
	const uint8_t oldSREG = SREG;

	cli();
	*out &= ~bit;

	SREG = oldSREG;
}

static void _shift_register_high()
{
	const uint8_t bit = digitalPinToBitMask(PIN_SHIFT_REGISTER_CLOCK);
	const uint8_t port = digitalPinToPort(PIN_SHIFT_REGISTER_CLOCK);
	volatile uint8_t *out = portOutputRegister(port);
	const uint8_t oldSREG = SREG;

	cli();
	*out |= bit;

	SREG = oldSREG;
}

void ctrl_motors(const uint8_t *byte_array, int num_motors)
{
	/* TODO replace this function with SPI transfer to make it more consistent and faster */
	uint8_t last_bit_value = 0;
	for (int motor_idx = 0; motor_idx < num_motors; motor_idx++) {
		for (int bit_id = 0; bit_id < NUM_PIN_PER_MOTOR; bit_id++) {
			const uint8_t bit_value = byte_array[motor_idx] >> bit_id & 0x01;
			if (last_bit_value != bit_value) {
				_digitalWrite_data(bit_value);
				last_bit_value = bit_value;
			}
		}

		_shift_register_high();
		_shift_register_low();
	}

	digitalWrite(PIN_STORAGE_REGISTER_CLOCK, HIGH);
	digitalWrite(PIN_STORAGE_REGISTER_CLOCK, LOW);
}
