# fuses for 16mhz, serial write
/home/mattvenn/arduino-1.6.9/hardware/tools/avr/bin/avrdude -C/home/mattvenn/arduino-1.6.9/hardware/tools/avr/etc/avrdude.conf -v -patmega328p -cstk500v2 -P /dev/ttyACM0  -e -Ulock:w:0x3F:m -Uefuse:w:0x05:m -Uhfuse:w:0xDE:m -Ulfuse:w:0xFF:m

# bootloader & lock has to happen at same time
/home/mattvenn/arduino-1.6.9/hardware/tools/avr/bin/avrdude -C/home/mattvenn/arduino-1.6.9/hardware/tools/avr/etc/avrdude.conf -v -patmega328p -cstk500v2 -P/dev/ttyACM0 -Uflash:w:/home/mattvenn/arduino-1.6.9/hardware/arduino/avr/bootloaders/optiboot/optiboot_atmega328.hex:i -Ulock:w:0x0F:m 

