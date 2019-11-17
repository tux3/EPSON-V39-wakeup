#include <libusb-1.0/libusb.h>
#include <iostream>
#include <vector>
#include <thread>
#include <numeric>
#include "blob.h"
#include "scanner_io.h"

using namespace std::chrono_literals;

int wait_for_status_change(libusb_device_handle* dev, std::initializer_list<uint8_t> desired, size_t max_steps)
{
    for (auto i=1; i<=max_steps; ++i) {
        if (int err = write(dev, MSG_GET_STATUS))
            return err;
        auto reply = read(dev, desired.size());
        if (reply.empty())
            return LIBUSB_ERROR_OTHER;

        if (equal(reply.begin(), reply.end(), desired.begin(), desired.end()))
            return i;

        std::this_thread::sleep_for(50ms);
    }

    std::cerr << "Timed out waiting for the scanner's status to be ready" << std::endl;
    return LIBUSB_ERROR_OTHER;
}

int send_iota_buf(libusb_device_handle* dev, std::initializer_list<uint8_t> iota_target)
{
    std::array<uint8_t, 0x100> iota_buf = {0};
    std::iota(iota_buf.begin(), iota_buf.end(), 0);

    if (int err = exchange_message(dev, {0x1e, 0x84}, REPLY_OK))
        return err;
    if (int err = write(dev, iota_target.begin(), iota_target.size()))
        return err;
    if (int err = write(dev, iota_buf.begin(), iota_buf.size()))
        return err;
    return expect_reply(dev, REPLY_OK);
}

int wake_device(libusb_device_handle* dev)
{
    if (libusb_kernel_driver_active(dev, DEV_INTERFACE))
    {
        if (int err = libusb_detach_kernel_driver(dev, DEV_INTERFACE)) {
            print_usb_err("Failed to detach active kernel driver", err);
            return 1;
        }
    }

    std::cout << "Sending USB reset to the scanner" << std::endl;
    libusb_reset_device(dev);

    std::cout << "Activating device configuration " << DEV_CONFIGURATION << std::endl;
    if (int err = libusb_set_configuration(dev, DEV_CONFIGURATION)) {
        print_usb_err("Failed to set configuration", err);
        return 1;
    }

    std::cout << "Activating interface " << DEV_INTERFACE << std::endl;
    if (int err = libusb_claim_interface(dev, DEV_INTERFACE)) {
        print_usb_err("Failed to claim interface", err);
        return 1;
    }

    std::cout << "Checking the scanner is alive and ready for init" << std::endl;
    if (int err = exchange_message(dev, MSG_GET_STATUS, {0x08, 0x00}))
        return err;
    if (int err = exchange_message(dev, {0x1b, 0x06}, REPLY_OK))
        return err;

    std::cout << "Writing binary blob to the scanner (" << sizeof(EPSON_V39_BINARY_BLOB) << " bytes)" << std::endl;
    if (int err = write(dev, {0x01, 0x00, 0x01, 0x00}))
        return err;
    if (int err = write(dev, EPSON_V39_BINARY_BLOB, sizeof(EPSON_V39_BINARY_BLOB)))
        return err;
    if (int err = write(dev, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}))
        return err;
    if (int err = exchange_message(dev, {0x8b}, REPLY_OK))
        return err;


    std::cout << "Power on..." << std::flush;
    if (int err = exchange_message(dev, {0x1b, 0x16}, REPLY_OK))
        return err;
    if (int err = exchange_message(dev, {0x80}, REPLY_OK))
        return err;
    if (int err = exchange_message(dev, MSG_GET_STATUS, {0x1a, 0x00}))
        return err;
    std::cout << "Device is on!" << std::endl;

    std::cout << "Waiting for device self-init... " << std::flush;
    int success_iterations = wait_for_status_change(dev, {0x18, 0x00}, 200);
    if (success_iterations < 0)
        return success_iterations;
    std::cout << "Ready after "<<success_iterations<<" requests!" << std::endl;

    std::cout << "Sending iota buffers" << std::endl;
    if (int err = send_iota_buf(dev, {0x03, 0x00, 0x00, 0xfc, 0x1f, 0x02, 0x00, 0x01, 0x00, 0x00}))
        return err;
    if (int err = send_iota_buf(dev, {0x03, 0x00, 0x00, 0xfd, 0x1f, 0x02, 0x00, 0x01, 0x00, 0x00}))
        return err;
    if (int err = send_iota_buf(dev, {0x03, 0x00, 0x00, 0xfe, 0x1f, 0x02, 0x00, 0x01, 0x00, 0x00}))
        return err;
    if (int err = exchange_message(dev, MSG_GET_STATUS, {0x18, 0x00}))
        return err;

    std::cout << "Finishing init" << std::endl;
    if (int err = exchange_message(dev, {0x1e, 0xe4}, REPLY_OK))
        return err;
    if (int err = exchange_message(dev, {0x0b, 0x00, 0x04, 0x00}, REPLY_OK))
        return err;
    if (int err = expect_reply(dev, {0x14, 0x05, 0x19, 0x01}))
        return err;
    if (int err = exchange_message(dev, MSG_GET_STATUS, {0x18, 0x00}))
        return err;

    if (int err = exchange_message(dev, {0x1e, 0x85}, {0x00}))
        return err;
    if (int err = exchange_message(dev, MSG_GET_STATUS, {0x18, 0x00}))
        return err;
    if (int err = exchange_message(dev, {0x1b, 0x40}, REPLY_OK))
        return err;
    if (int err = exchange_message(dev, MSG_GET_STATUS, {0x18, 0x00}))
        return err;

    if (int err = exchange_message(dev, {0x1e, 0x9f}, {0x00}))
        return err;
    if (int err = exchange_message(dev, MSG_GET_STATUS, {0x18, 0x00}))
        return err;
    if (int err = exchange_message(dev, {0x1e, 0x65}, {0x00}))
        return err;
    if (int err = exchange_message(dev, MSG_GET_STATUS, {0x18, 0x00}))
        return err;

    if (int err = write(dev, MSG_GET_SCANNER_NAME))
        return err;
    auto scanner_name = read(dev);
    std::cout << "Scanner ready: " << scanner_name.data() << std::endl;

    return 0;
}
