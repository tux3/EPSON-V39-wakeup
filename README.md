## What is this, even?

I'm the unfortunate Linux-using owner of a cursed EPSON V39 scanner.  
When that scanner is used on a Windows machine it will boot up, and it can then be turned off and on again on a Linux, then everything works fine. For a while.  
Let it sit on a shelf for a few hours and it doesn't want anything to do with Linux anymore, showing roughly as many signs of life as a USB-powered brick, until plugged on a Windows again.

This program sends the right incantation that will wake up a V39 without having to reboot on Windows.

## How?

This uses libusb to send the boot commands it expects (that, and 64KB of binary blob).

