#ifndef __GLOBAL_HPP
#define __GLOBAL_HPP

#include "pico/stdlib.h"
#include <stdio.h>

const uint8_t gccDataPins[4] = {13, 12, 11, 10};

const uint8_t oscilloPins[3] = {2, 3, 4};

const int us = 125;

#define LED_PIN 25
#define USB_POWER_PIN 24

#endif