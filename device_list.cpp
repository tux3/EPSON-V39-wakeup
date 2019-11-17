#include "device_list.h"

device_list::device_list(libusb_context* ctx)
{
    list_len = libusb_get_device_list(ctx, &list_devs);
}

device_list::~device_list()
{
    libusb_free_device_list(list_devs, 1);
}

size_t device_list::len()
{
    return list_len;
}

libusb_device *device_list::operator[](size_t idx)
{
    return list_devs[idx];
}
