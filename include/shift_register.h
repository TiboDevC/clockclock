#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void ctrl_test(void);
void shift_reg_init(void);
void ctrl_motors(const uint8_t *byte_array, int num_motors);

#ifdef __cplusplus
}
#endif

#endif /* SHIFT_REGISTER_H */
