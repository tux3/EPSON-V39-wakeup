cmake_minimum_required(VERSION 3.0)
project(epson_v39_wakeup)

set(CMAKE_CXX_STANDARD 17)

add_executable(epson_v39_wakeup main.cpp device_list.cpp device_list.h wake_device.cpp wake_device.h blob.h scanner_io.cpp scanner_io.h)
target_link_libraries(epson_v39_wakeup usb-1.0)
