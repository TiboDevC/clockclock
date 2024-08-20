#include <stdint.h>

#include <Arduino.h>
#include <SPI.h>

#define PIN_CLOCK_SHIFT_REGISTER   13 /* SPI clock pin */
#define PIN_CLOCK_STORAGE_REGISTER 10 /* SPI SS pin */
#define PIN_SERIAL_DATA_OUTPUT     11 /* SPI MOSI pin */

void shift_reg_init(void)
{
	pinMode(PIN_CLOCK_SHIFT_REGISTER, OUTPUT);
	pinMode(PIN_CLOCK_STORAGE_REGISTER, OUTPUT);
	pinMode(PIN_SERIAL_DATA_OUTPUT, OUTPUT);

	digitalWrite(PIN_CLOCK_SHIFT_REGISTER, LOW);
	digitalWrite(PIN_CLOCK_STORAGE_REGISTER, LOW);

	delay(10);
	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV8);
	SPI.setDataMode(SPI_MODE0); /* Data changes on clock falling edge */
	SPI.setBitOrder(MSBFIRST);
}

void ctrl_test(void)
{
	int bit = 0;
	digitalWrite(PIN_CLOCK_SHIFT_REGISTER, LOW);
	digitalWrite(PIN_CLOCK_STORAGE_REGISTER, LOW);
	delay(10);

	for (;;) {
		if (bit % 4 == 0) {
			digitalWrite(PIN_SERIAL_DATA_OUTPUT, HIGH);
		} else {
			digitalWrite(PIN_SERIAL_DATA_OUTPUT, LOW);
		}

		digitalWrite(PIN_CLOCK_SHIFT_REGISTER, HIGH);
		digitalWrite(PIN_CLOCK_STORAGE_REGISTER, HIGH);

		digitalWrite(PIN_CLOCK_SHIFT_REGISTER, LOW);
		digitalWrite(PIN_CLOCK_STORAGE_REGISTER, LOW);

		delay(100);

		bit++;
	}
}

void ctrl_motors(const uint8_t *byte_array, int byte_array_size)
{
	digitalWrite(PIN_CLOCK_STORAGE_REGISTER, LOW);
	SPI.transfer((void *) byte_array, byte_array_size);
	digitalWrite(PIN_CLOCK_STORAGE_REGISTER, HIGH);
}
