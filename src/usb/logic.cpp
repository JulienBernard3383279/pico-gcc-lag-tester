#include "usb/logic.hpp"
#include "usb/protocol.hpp"

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "pico/util/queue.h"

#include "global.hpp"

#include <array>
#include <functional>

#include "hardware/structs/systick.h"
#include "hardware/adc.h"
#include "hardware/regs/rosc.h"

#include "string.h"

#include "joybus.hpp"

void local_ep_in_handler(uint8_t *buf, uint16_t len);
void local_ep_out_handler(uint8_t *buf, uint16_t len);
void local_main();
void core1_entry();

const uint16_t descriptor_strings_len = 3;
const char *descriptor_strings[descriptor_strings_len] = {
        "Arte",
        "Lag test controller ult",
        "1" // The "release number"
};

const int inReportSize = 64;
const int outReportSize = 64;

//uint8_t hidReportDescriptor[1] = { 0 };

#define NUMBER_OF_READINGS_PER_TEST 512
//1024

// 2048 * 2 bytes = toute la stack (4KB par core)

void await_time32us(uint32_t target) {
    while ( (time_us_32() - target) & (1 << 31) );
}

void enterMode() {

    USBConfiguration usbConfiguration =
    {
        .inEpMaxPacketSize = inReportSize,
        .inEpActualPacketSize = inReportSize,
        .outEpMaxPacketSize = outReportSize,
        .epOutId = 2,
        .descriptorStrings = descriptor_strings,
        .descriptorStringsLen = descriptor_strings_len,
        .hid = false,
        .bcdHID = 0x0110,
        .hidReportDescriptor = nullptr,
        .hidReportDescriptorLen = 0,
        .useWinUSB = true,
        .VID = 0xA57E,
        .PID = 0x0002,
        .bcdDevice = 0x100,

        .ep_in_handler = local_ep_in_handler,
        .ep_out_handler = local_ep_out_handler
    };

    initMode(usbConfiguration);

    multicore_launch_core1(core1_entry);

    local_main();
}

volatile bool transferHappened = true;
void local_ep_in_handler(uint8_t *buf, uint16_t len) {
    transferHappened = true;
}

void local_ep_out_handler(uint8_t *buf, uint16_t len) {
}

/* Lag test controller ult mode
*
* Core 1: acts as a controller; initially select port 1 and accept, wait for a few secs then start the lag test (set a flag in the bridge)
* Lag test: joybus mode, with the function checking the bridge's l flag to know if it should set L and analog L
* 
* Core 0 : lag test: wait for X + random(0, 1frame) then set L and analog L in the bridge; start the phototransistor reading then ; then start sending over USB
*/

enum class State {
    NONE,
    XandDpadDown
};

struct Bridge {
    volatile State state = State::NONE;
}; 
Bridge bridge;

uint32_t rnd(void){
    int k, random=0;
    volatile uint32_t *rnd_reg=(uint32_t *)(ROSC_BASE + ROSC_RANDOMBIT_OFFSET);
    
    for(k=0;k<32;k++){
    
    random = random << 1;
    random=random + (0x00000001 & (*rnd_reg));

    }
    return random;
}

void local_main() {
    std::array<uint8_t, 64> full0 {};
    std::array<uint16_t, NUMBER_OF_READINGS_PER_TEST> array;

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    multicore_fifo_push_blocking((uintptr_t)&bridge);

    uint32_t timestamp = time_us_32();
    uint32_t awaitTransferTimestamp = 0;

    while (true) {

        // 3 iterations before the iteration that matters.
        // So, (like auto test) we need to be on white to test black>white

        // if (lagTestModeCounter < 33 || ( lagTestModeCounter >= 67 && lagTestModeCounter < 100 || lagTestModeCounter >= 133 && lagTestModeCounter < 167 ||
        // lagTestModeCounter >= 400 && lagTestModeCounter < 433))
        // Then 701
        // But here we can be off phase 1 because we're not bound by USB

        // We can tighten the wait times since we got an autocorrect now

        timestamp = time_us_32();
        bridge.state = State::XandDpadDown;

        while (time_us_32() - timestamp <= 35'000);
        bridge.state = State::NONE;

        while (time_us_32() - timestamp <= 70'000);
        bridge.state = State::XandDpadDown;

        while (time_us_32() - timestamp <= 105'000);
        bridge.state = State::NONE;

        while (time_us_32() - timestamp <= 140'000);
        bridge.state = State::XandDpadDown;

        while (time_us_32() - timestamp <= 185'000);
        bridge.state = State::NONE;

        while (time_us_32() - timestamp <= 210'000);

        bool success = false;

        while (!success) {
            // Read
            bridge.state = State::XandDpadDown;
            uint32_t timestamp = time_us_32();
            for (int i = 0; i<array.size(); i++) {
                if (i == 50*5) bridge.state = State::NONE; // Turn off the x+dpaddown press after 100ms
                while (time_us_32() - timestamp <= 200*i); // 200 <=> every 0.2ms
                array[i] = adc_read();
            }

            // Auto correct: if it doesn't look like the result of a lag test, we may have gotten out of sync, check again (we advanced by one)
            // "Doesn't look" meaning, the same check as in the python analyser
            int index;
            for (index = 10; index < NUMBER_OF_READINGS_PER_TEST - 5; index++) {
                if (array[index] < 3800 && array[index+5] < array[index]) {
                    success = true; // < 2ms => fail
                    break;
                }
            }
        }

        // Report
        while (!transferHappened);
        usb_start_transfer_in_ep((uint8_t*) &full0, 64);
        gpio_put(25, 1);
        transferHappened = false;
        for (int i = 0; i < NUMBER_OF_READINGS_PER_TEST*2/64; i++) {
            while (!transferHappened);
            transferHappened = false;
            usb_start_transfer_in_ep(((uint8_t*) &(array[0]) +i*64), 64);
        }

        sleep_us(rnd() % 16'667);
    }
    
}

void core1_entry() {
    set_sys_clock_khz(us*1000, true);

    Bridge* localBridge = (Bridge*) multicore_fifo_pop_blocking();
    std::function<GCReport()> callback = [localBridge](){ // Quand on passe des choses à un lambda on ne peut plus le convertir en pointeur vers fonction //TODO
        GCReport gcReport = defaultGcReport;
        State state = localBridge->state;
        switch (state) {
        case State::NONE:
            break;
        case State::XandDpadDown:
            gcReport.x = 1;
            gcReport.dDown = 1;
            break;
        }
        return gcReport;
    };

    CommunicationProtocols::Joybus::enterMode(22, callback);
}