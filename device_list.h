#ifndef DEVICE_LIST_H
#define DEVICE_LIST_H

#include <libusb-1.0/libusb.h>

class device_list {
public:
    device_list(libusb_context* ctx);
    ~device_list();

    size_t len();
    libusb_device* operator[](size_t idx);

private:
    libusb_device** list_devs;
    ssize_t list_len;
};

#endif //DEVICE_LIST_H
