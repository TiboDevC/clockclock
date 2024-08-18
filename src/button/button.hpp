#ifndef SOFT_BUTTON_HPP
#define SOFT_BUTTON_HPP

#include <stdint.h>

enum button_type_t : uint8_t {
	BUTTON_MODE,
	BUTTON_ENCODER,
	BUTTON_SHUTDOWN,
	BUTTON_MAX,
};

enum button_press_t : uint8_t {
	NO_PRESS = 0, /* Must be 0 */
	SHORT_PRESS,
	LONG_PRESS,
};

struct button_t {
	enum button_press_t press;
};

void button_init();
void button_check();
void button_get_state(struct button_t *button, enum button_type_t button_type);
void button_reset();
unsigned long button_last_press();

#endif /* SOFT_BUTTON_HPP */
