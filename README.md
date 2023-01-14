# libftdi for Windows

This library is a fork of [libftdi](https://www.intra2net.com/en/developer/libftdi/).

The design objective of this library is to provide a better
out-of-the-box experience for [AVRDUDE](https://github.com/avrdudes/avrdude) Windows users.

This library only supports a subset of the original implementation.
The intend is not to provide a general purpose implementation,
but to satisfy the usage scenarios of AVRDUDE.

This library supports only **Microsoft Visual C/C++**,
as used in the **msvc** job of the [AVRDUDE build action](https://github.com/avrdudes/avrdude/blob/main/.github/workflows/build.yml).

The latest version of libftdi for Windows can be found here:\
<https://github.com/avrdudes/libftdi>

The original README of libftdi can be found here: [README](./README).

## Notable Changes

This library was completely rewritten to support the default Windows
plug-and-play drivers provided by FTDI (aka D2XX driver),
while maintaining a libftdi compatible programming API.

The original **libftdi** library only supports libusb-compatible drivers,
which requires you to swap the official drivers for a libusb-compatible driver
using tools such as Zadig.
As Windows serial ports and many other tools expect the default FTDI D2XX
drivers, you will find yourself swapping drivers back-and-forth when
using **libftdi** with your FTDI devices.
