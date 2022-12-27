# libftdi for Windows

This library is a rewrite of [libftdi](https://www.intra2net.com/en/developer/libftdi/).
The design objective of **libftdi for Windows** is to provide a better
out-of-the-box experience for Windows users by supporting the default
Windows plug-and-play drivers provided by FTDI, while maintaining a
libftdi-compatibale programming API.

The original **libftdi** library only supports libusb-compatible drivers,
which requires you to swap the official drivers for a libusb-compatible driver
using tools such as Zadig.
As Windows serial ports and many other tools expect the default FTDI D2XX
drivers, you will find yourself swapping drivers back-and-forth when
using **libftdi** with your FTDI devices.

The latest version of libftdi for Windows can be found here:\
<https://github.com/avrdudes/libftdi>

The original README of libftdi can be found here: [README](./README).
