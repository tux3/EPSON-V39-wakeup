#include "scanner_io.h"
#include <iostream>

void print_usb_err(const char* msg, int err)
{
    auto libusb_err = (libusb_error)err;
    std::cerr << msg << ": " << libusb_strerror(libusb_err) << std::endl;
}

int write(libusb_device_handle* dev, const uint8_t* data, size_t size)
{
    int transferred = 0;
    int err = libusb_bulk_transfer(dev, ENDPOINT_OUT, (uint8_t*)data, size, &transferred, 0);
    if (err)
        print_usb_err("Failed to send message to scanner", err);
    if (transferred != size) {
        std::cerr << "Failed to send message to scanner, output truncated" << std::endl;
        return LIBUSB_ERROR_OTHER;
    }
    return err;
}

int write(libusb_device_handle* dev, std::initializer_list<uint8_t> data)
{
    return write(dev, data.begin(), data.size());
}

/// Returns an empty vector on error
std::vector<uint8_t> read(libusb_device_handle* dev, int expected_size)
{
    std::vector<uint8_t> data;
    data.resize(MAX_READ_SIZE);
    int transferred = 0;
    int err = libusb_bulk_transfer(dev, ENDPOINT_IN, data.data(), MAX_READ_SIZE, &transferred, 0);

    if (err) {
        print_usb_err("Failed to receive message from scanner", err);
        data.clear();
    } else if (expected_size >=0 && transferred != expected_size) {
        std::cerr << "Received truncated message from scanner, expected " << expected_size << " bytes but got " << transferred << std::endl;
        data.clear();
    } else {
        data.resize(transferred);
    }
    return data;
}

int expect_reply(libusb_device_handle* dev, std::initializer_list<uint8_t> expected_reply)
{
    auto reply = read(dev, expected_reply.size());
    if (reply.empty())
        return LIBUSB_ERROR_OTHER;
    if (!equal(reply.begin(), reply.end(), expected_reply.begin(), expected_reply.end())) {
        std::cerr << "Unexpected reply from scanner, aborting wake-up sequence!" << std::endl;
        return LIBUSB_ERROR_OTHER;
    }
    return 0;
}

int exchange_message(libusb_device_handle* dev, std::initializer_list<uint8_t> message, std::initializer_list<uint8_t> expected_reply)
{
    if (int err = write(dev, message))
        return err;
    return expect_reply(dev, expected_reply);
}
