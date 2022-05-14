#ifndef USB__PROTOCOL_HPP
#define USB__PROTOCOL_HPP

#include "usb/common.hpp"

struct USBConfiguration {
    uint16_t inEpMaxPacketSize;
    uint16_t inEpActualPacketSize;
    uint16_t outEpMaxPacketSize;
    uint8_t epOutId; // 1 or 2
    const char **descriptorStrings;
    uint16_t descriptorStringsLen;
    bool hid;
    uint16_t bcdHID;
    uint8_t* hidReportDescriptor;
    uint16_t hidReportDescriptorLen;
    bool useWinUSB;
    uint16_t VID;
    uint16_t PID;
    uint16_t bcdDevice;

    void (*ep_in_handler)(uint8_t *buf, uint16_t len);
    void (*ep_out_handler)(uint8_t *buf, uint16_t len);
};

void initMode(USBConfiguration);

void usb_start_transfer_in_ep(uint8_t *buf, uint16_t len);

void usb_start_transfer_out_ep(uint8_t *buf, uint16_t len);

bool in_ep_available();

#endif