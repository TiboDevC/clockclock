#include <Arduino.h>

#include "button.hpp"

#define PIN_BUTTON_MODE     6
#define PIN_BUTTON_ENCODER  4
#define PIN_BUTTON_SHUTDOWN 5
#define PIN_ENCODER_0       2 /* Interrupt pin 0 */
#define PIN_ENCODER_1       3 /* Interrupt pin 1 */

#define BT_PRESSED   LOW
#define BT_UNPRESSED HIGH

#define BT_SHORT_PRESS_MS 100
#define BT_LONG_PRESS_MS  2000

struct button_cfg_t {
	const uint8_t pin;
	uint8_t last_state;
	enum button_press_t press;
	unsigned long press_start_ms;
	uint8_t long_press_reported;
};

static struct button_cfg_t _buttons[BUTTON_MAX] = {{.pin = PIN_BUTTON_MODE,
                                                    .last_state = BT_UNPRESSED,
                                                    .press = NO_PRESS,
                                                    .press_start_ms = 0,
                                                    .long_press_reported = 0},
                                                   {.pin = PIN_BUTTON_ENCODER,
                                                    .last_state = BT_UNPRESSED,
                                                    .press = NO_PRESS,
                                                    .press_start_ms = 0,
                                                    .long_press_reported = 0},
                                                   {.pin = PIN_BUTTON_SHUTDOWN,
                                                    .last_state = BT_UNPRESSED,
                                                    .press = NO_PRESS,
                                                    .press_start_ms = 0,
                                                    .long_press_reported = 0}};
static unsigned long _last_action_ms = 0;
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
	pinMode(PIN_BUTTON_MODE, INPUT_PULLUP);
	pinMode(PIN_BUTTON_ENCODER, INPUT_PULLUP);
	pinMode(PIN_BUTTON_SHUTDOWN, INPUT_PULLUP);
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
			if (BT_PRESSED == input) {
				bt->press_start_ms = time_ms;
				bt->long_press_reported = false;
				bt->last_state = input;
			} else if (BT_UNPRESSED == input) {
				unsigned long press_duration = time_ms - bt->press_start_ms;

				if (press_duration >= BT_SHORT_PRESS_MS && !bt->long_press_reported) {
					bt->press = SHORT_PRESS;
					Serial.print(button_idx);
					Serial.println(": Short press");
				} else {
					bt->press = NO_PRESS;
				}

				bt->last_state = input;
			}
			_last_action_ms = time_ms;
		} else if (BT_PRESSED == input && !bt->long_press_reported) {
			unsigned long press_duration = time_ms - bt->press_start_ms;

			if (press_duration >= BT_LONG_PRESS_MS) {
				bt->press = LONG_PRESS;
				bt->long_press_reported = true;
				Serial.print(button_idx);
				Serial.println(": Long press");
			}
		}
	}
}

void button_get_state(struct button_t *button, enum button_type_t button_type)
{
	if (NULL == button || button_type >= BUTTON_MAX) {
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
	return _last_action_ms;
}

int8_t button_get_encoder_count()
{
	if (_encoder_count != 0) {
		_last_action_ms = millis();
	}
	const int8_t encoder_count = _encoder_count;
	_encoder_count = 0;
	return encoder_count;
}
