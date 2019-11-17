#include <iostream>
#include <libusb-1.0/libusb.h>
#include "device_list.h"
#include "wake_device.h"

constexpr uint16_t TARGET_VID = 0x04B8; // Seiko Epson Corp.
constexpr uint16_t TARGET_PID = 0x013D; // V39

libusb_device_handle* open_target_device(libusb_context* ctx, uint16_t vid, uint16_t pid) {
    libusb_device_handle* handle;
    device_list devs(ctx);
    for (size_t i=0; i<devs.len(); ++i) {
        auto dev = devs[i];
        libusb_device_descriptor desc = {0};
        libusb_get_device_descriptor(dev, &desc);
        if (desc.idVendor == TARGET_VID && desc.idProduct == TARGET_PID) {
            libusb_open(dev, &handle); // Zeroes handle on error
            return handle;
        }
    }
    return nullptr;
}

int main() {
    libusb_context* ctx;
    if (libusb_init(&ctx)) {
        std::cerr << "Failed to init libusb" << std::endl;
        return 1;
    }

    libusb_device_handle* dev = open_target_device(ctx, TARGET_VID, TARGET_PID);
    if (!dev) {
        std::cerr << "Failed to open the scanner USB device" << std::endl;
        return 1;
    }

    int result = wake_device(dev);
    if (result != 0) {
        std::cerr << "Failed to initialize the scanner. You should probably plug it off and on again before trying anything else." << std::endl;
    }

    libusb_close(dev);
    return result;
}
