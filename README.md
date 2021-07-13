# proto-rak5010-arduino

Prototype of connected position switch with RAK5010 and arduino.

### Bootloader

In order to utilize Arduino with RAK5010, the first step is to flash the specific bootloader located here in the project folder: position-switch-connected\proto-rak5010-arduino\output\hex\feather_nrf52840_express_bootloader-0.2.13_s140_6.1.1.hex

To brun the bootloader, you can use a JLINK (see the official [doc](https://docs.rakwireless.com/Product-Categories/WisTrio/RAK5010/Quickstart/#prerequisites) at chapter *Product Configuration*) and the JFLASH tool with the configuration file located here: position-switch-connected\proto-rak5010-arduino\output\nrf52840.jflash

