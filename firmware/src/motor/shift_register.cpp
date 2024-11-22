#include <cstdint>

#include <Arduino.h>
#include <SPI.h>

#include "cfg.hpp"

/* https://docs.espressif.com/projects/esp-idf/en/v5.3.1/esp32c3/api-reference/peripherals/spi_master.html */
#define PIN_CLOCK_SHIFT_REGISTER   SCK  /* SHCP / SPI clock pin */
#define PIN_CLOCK_STORAGE_REGISTER SS   /* STCP / SPI SS pin */
#define PIN_SERIAL_DATA_OUTPUT     MOSI /* SPI MOSI pin */

void shift_reg_init()
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

void ctrl_test()
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

void ctrl_motors(const std::array<uint8_t, SHIFT_REG_SIZE> &byte_array)
{
	/* The Arduino library read SPI and writes it into the input buffer.
	 * So we need another buffer to not corrupt the original one. */
	uint8_t buf[SHIFT_REG_SIZE];
	memcpy(buf, byte_array.data(), byte_array.size());
	digitalWrite(PIN_CLOCK_STORAGE_REGISTER, LOW);
	SPI.transfer((void *) buf, byte_array.size());
	digitalWrite(PIN_CLOCK_STORAGE_REGISTER, HIGH);
}
