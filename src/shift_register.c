#include <stdint.h>

#include "Arduino.h"

#define PIN_MASTER_RESET 2
#define PIN_SHIFT_REGISTER_CLOCK 3
#define PIN_STORAGE_REGISTER_CLOCK 4
#define PIN_OUTPUT_ENABLE 5
#define PIN_SERIAL_DATA_OUTPUT 6

void shift_reg_init(void) {
  pinMode(PIN_MASTER_RESET, OUTPUT);
  pinMode(PIN_SHIFT_REGISTER_CLOCK, OUTPUT);
  pinMode(PIN_STORAGE_REGISTER_CLOCK, OUTPUT);
  pinMode(PIN_OUTPUT_ENABLE, OUTPUT);
  pinMode(PIN_SERIAL_DATA_OUTPUT, OUTPUT);

  digitalWrite(PIN_SHIFT_REGISTER_CLOCK, LOW);
  digitalWrite(PIN_STORAGE_REGISTER_CLOCK, LOW);

  digitalWrite(PIN_MASTER_RESET, LOW);
  digitalWrite(PIN_OUTPUT_ENABLE, LOW);
  delay(10);
  digitalWrite(PIN_MASTER_RESET, HIGH);
  delay(10);
}

void ctrl_test(void) {
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

void ctrl_motors(const uint8_t* byte_array, int num_bits) {
  for (int bit_pos = 0; bit_pos < num_bits; bit_pos++) {
    const int bit_value = byte_array[bit_pos / 8] >> (bit_pos % 8) & 0x01;
    digitalWrite(PIN_SERIAL_DATA_OUTPUT, bit_value);

    digitalWrite(PIN_SHIFT_REGISTER_CLOCK, HIGH);
    digitalWrite(PIN_SHIFT_REGISTER_CLOCK, LOW);
  }

  digitalWrite(PIN_STORAGE_REGISTER_CLOCK, HIGH);
  digitalWrite(PIN_STORAGE_REGISTER_CLOCK, LOW);
  delay(2);
}
