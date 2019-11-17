#ifndef SCANNER_IO_H
#define SCANNER_IO_H

#include <libusb-1.0/libusb.h>
#include <initializer_list>
#include <vector>
#include <cstdint>
#include <cstddef>

constexpr int DEV_CONFIGURATION = 1;
constexpr int DEV_INTERFACE = 0;
constexpr int ENDPOINT_OUT = 0x02;
constexpr int ENDPOINT_IN = 0x81;
constexpr int MAX_READ_SIZE = 256*1024; // In reality, we never expect to read more than a few hundred bytes

static constexpr std::initializer_list<uint8_t> MSG_GET_STATUS = {0x1b, 0x03};
static constexpr std::initializer_list<uint8_t> MSG_GET_SCANNER_NAME = {0x1b, 0x13};
static constexpr std::initializer_list<uint8_t> REPLY_OK = {0x06};

void print_usb_err(const char* msg, int err);
int write(libusb_device_handle* dev, const uint8_t* data, size_t size);
int write(libusb_device_handle* dev, std::initializer_list<uint8_t> data);
std::vector<uint8_t> read(libusb_device_handle* dev, int expected_size = -1);
int expect_reply(libusb_device_handle* dev, std::initializer_list<uint8_t> expected_reply);
int exchange_message(libusb_device_handle* dev, std::initializer_list<uint8_t> message, std::initializer_list<uint8_t> expected_reply);

#endif // SCANNER_IO_H
