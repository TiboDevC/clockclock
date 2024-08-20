#include <Arduino.h>

#include "button.hpp"

#define PIN_BUTTON_MODE     6
#define PIN_BUTTON_ENCODER  4
#define PIN_BUTTON_SHUTDOWN 5
#define PIN_ENCODER_0       2 /* Interrupt pin 0 */
#define PIN_ENCODER_1       3 /* Interrupt pin 1 */

#define BT_PRESSED   HIGH
#define BT_UNPRESSED LOW

#define BT_SHORT_PRESS_MS 100
#define BT_LONG_PRESS_MS  2000

struct button_cfg_t {
	const uint8_t pin;
	uint8_t last_state;
	enum button_press_t press;
	unsigned long last_press_ms;
};

static struct button_cfg_t _buttons[BUTTON_MAX] = {
    {.pin = PIN_BUTTON_MODE}, {.pin = PIN_BUTTON_ENCODER}, {.pin = PIN_BUTTON_SHUTDOWN}};
static unsigned long _last_press_ms = 0;
static int8_t _encoder_count = 0;
static volatile uint8_t _last_encoder_0;

static void _encoder_int()
{
	const uint8_t encoder_0 = digitalRead(PIN_ENCODER_0);

	if (encoder_0 != _last_encoder_0) {
		const int encoder_1 = digitalRead(PIN_ENCODER_1);

		if (encoder_1 != encoder_0) {
			_encoder_count++;
		} else {
			_encoder_count--;
		}
	}
	_last_encoder_0 = encoder_0;
}

void button_init()
{
	pinMode(PIN_BUTTON_MODE, INPUT);
	pinMode(PIN_BUTTON_ENCODER, INPUT);
	pinMode(PIN_BUTTON_SHUTDOWN, INPUT);
	pinMode(PIN_ENCODER_0, INPUT);
	pinMode(PIN_ENCODER_1, INPUT);

	attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_0), _encoder_int, CHANGE);
	attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_1), _encoder_int, CHANGE);

	_last_encoder_0 = digitalRead(PIN_ENCODER_0);
}

void button_check()
{
	const unsigned long time_ms = millis();

	for (int button_idx = 0; button_idx < BUTTON_MAX; button_idx++) {
		struct button_cfg_t *bt = &_buttons[button_idx];
		const uint8_t input = digitalRead(bt->pin);

		if (input != bt->last_state) {
			if (BT_UNPRESSED == input) {
				if (time_ms - bt->last_press_ms > BT_LONG_PRESS_MS) {
					bt->press = LONG_PRESS;
#if 0
					Serial.print(button_idx);
					Serial.println(" long press");
#endif
				} else if (time_ms - bt->last_press_ms > BT_SHORT_PRESS_MS) {
					bt->press = SHORT_PRESS;
#if 0
					Serial.print(button_idx);
					Serial.println(" short press");
#endif
				} else {
					bt->press = NO_PRESS;
				}
			}
			bt->last_state = input;
			_buttons->last_press_ms = time_ms;
			_last_press_ms = time_ms;
		}
	}
}

void button_get_state(struct button_t *button, enum button_type_t button_type)
{
	if (NULL == button || BUTTON_MAX >= button_type) {
		return;
	}
	button->press = _buttons[button_type].press;
	_buttons[button_type].press = NO_PRESS;
}

void button_reset()
{
	for (int button_idx = 0; button_idx < BUTTON_MAX; button_idx++) {
		_buttons[button_idx].press = NO_PRESS;
	}
}

unsigned long button_last_press()
{
	return _last_press_ms;
}

int8_t button_get_encoder_count()
{
	return _encoder_count;
}
