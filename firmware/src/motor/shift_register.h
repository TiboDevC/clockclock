#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#include <array>
#include <stdint.h>

#include "cfg.hpp"

void ctrl_test();
void shift_reg_init();
void ctrl_motors(const std::array<uint8_t, SHIFT_REG_SIZE> &byte_array);

#endif /* SHIFT_REGISTER_H */
