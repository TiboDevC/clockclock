#include <Arduino.h>

#include "animation.h"
#include "shift_register.h"

const struct anim_set_t anim_set = {
    .num_sequences = 2,
    .sequence      = {
        { // Séquence 1
         .num_motors = 2,
         .position = {
             {0, ANIM_SYNC, ANGLE_RELATIVE, 360, 1000},
             {1, ANIM_SYNC, ANGLE_RELATIVE, -360, 1500},
         }
        },
        { // Séquence 2
         .num_motors = 2,
         .position   = {
             {0, ANIM_ASYNC, ANGLE_RELATIVE, 360, 1200},
             {1, ANIM_SYNC, ANGLE_RELATIVE, -360, 1800},
         }
        }
    }
};

void setup() {


    Serial.begin(9600);
    Serial.println("start");
    shift_reg_init();

    process_anim_set(anim_set);
    ctrl_test();
    uint8_t shift_bit_reg[] = {0b11001100, 0b10001010};
    ctrl_motors(shift_bit_reg, 16);
}

void loop() {
    process_anim();
}