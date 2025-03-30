# Clockclock

This personal project first intention is to build a clock with a design inspired
by [Human since 1982](https://www.humanssince1982.com/).

1. [The idea](#the-idea)
2. [The solution](#the-solution)
    1. [Motors](#motors)
    2. [Real Time Clock (RTC)](#real-time-clock-rtc)
    3. [Hands](#hands)
3. [Hardware](#hardware)

## The idea

There are many open source models and all these projects use several microcontrollers, often one for two motors, which
makes the project expensive and adds a lot of complexity.
This makes the project expensive and adds a lot of complexity. My aim is to achieve a much simpler and cheaper project
with a single MCU controlling all 48 motors.
My aim is to achieve a much simpler and cheaper project with a single MCU controlling all 48 motors.

I also wanted to create a clock that was always on time but not connected to the internet.

The final constraint was to make the clock as small as possible, with a target length of 50cm.

## The solution

### Motors

I chose stepper motors [BKA30D-R5](datasheet/BKA30D-R5.webp) as they are relatively cheap (<4€/piece), then can turn 360°, and they are 6 cm long
which is small enough for the project. Also, they consume only 20mA per motor so a regular USB-C phone charger can
handle it (20mA x 48 motors = 960mA).

All motors are controlled by an ESP32-C3 (RISC-V architecture). This MCU is connected to a series of 12 bit-shift
registers. Bit-shift registers are connected to steppers driver: VID6606 (or equivalent like AX1201728SG/STI6606z). Each
driver can drive 4 steppers, 2 pins per stepper: direction and step.

![bit-shift and stepper driver](img/elec0.png)

One bit shift register has 8 output so it controls 2 motors. At any point of time the MCU knows the motor sequence step.
Periodically, the MCU sends the step sequence through bit shift registers and once all of them have been updated with
the correct sequence, the MCU latch the output. So the MCU is connected to the first bit shift register only and this
requires 3 GPIO: shift_clock, latch_clock and data_out. As all bitshift registers are connected in series, each of them
send the config to the next one.

![data flow](img/high-level-schematic.png)

### Real Time Clock (RTC)

For the clock to be always on time without internet, I integrated a DCF77 module that picks up the time sent by radio
from Germany. Then this time is used to configure the RTC in the DS3231 module.

### Hands

Hands are still under development right now, some mechanical issues must be solved.

## Hardware

The hardware consists of 2 parts:

- a main PCB containing:
    - 1 x [ESP32-C3 devkit-m1](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32c3/esp32-c3-devkitm-1/index.html)
    - 12 x [Power Logic 8-Bit Shift Register](https://www.ti.com/lit/ds/symlink/tpic6c595.pdf)
    - 12 x VID6606 (or equivalent like AX1201728SG/STI6606z)
    - 1 x DS3231 RTC & EEPROM
    - 1
      x [CANADUINO DCF77 Radio Clock Receiver Module](https://universal-solder.ca/docs/CANADUINO_Atomic_Clock_Receiver_Kit_V2.pdf)
- 12 PCB connected to steppers:
    - stepper motor BKA30D-R5

Power supply is provided by a USB-C dongle that provides 5V.

Here are the schematic of [the main board](elec/clockclock_main/output/clockclock_main.pdf)
and [the stepper board](elec/clokclock_stepper/output/clokclock_stepper.pdf).
