#include "global.hpp"

#include "usb/logic.hpp"

#include "hardware/structs/systick.h"

#include "hardware/gpio.h"

#include "pico/time.h"

int main() {
    // Clock at 125MHz
    set_sys_clock_khz(us*1000, true);
    
    initialize_uart();

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    enterMode();
}