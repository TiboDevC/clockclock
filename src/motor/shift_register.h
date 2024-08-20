#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#include <stdint.h>

void ctrl_test(void);
void shift_reg_init(void);
void ctrl_motors(const uint8_t *byte_array, int byte_array_size);

#endif /* SHIFT_REGISTER_H */
