## Prerequisite

#### [CMake](http://www.cmake.org)
#### [Atmel AVR8 Toolchain](http://www.atmel.com/forms/software-download.aspx?target=tcm:26-64140)
#### AVRDUDE
```
$ wget https://copr-be.cloud.fedoraproject.org/results/jlindeberg/arduino-epel7/epel-7-x86_64/avrdude-6.1-2.fc22/avrdude-6.1-2.el7.centos.x86_64.rpm
$ sudo yum install libftdi
$ sudo rpm -ivh avrdude-6.1-2.el7.centos.x86_64.rpm
$ sudo usermod -a -G dialout hokim
```

[Avrdude Test](http://techareg.net/AvrProgrammer.html)

## Setup

```
$ sudo tar xvzf avr8-gnu-toolchain-3.4.5.1522-linux.any.x86_64.tar.gz -C /opt
$ mkdir -p ~/work/avr; cd ~/work/avr
$ git clone https://github.com/hokim72/AVR-Common.git
```
~/.bashrc file:
```
export PATH=$PATH:/opt/avr8-gnu-toolchain-linux_x86_64/bin
export AVR_ROOT=/opt/avr8-gnu-toolchain-linux_x86_64/avr
export AVR_COMMON=$HOME/work/AVR-Common
alias cmake-avr='cmake -DCMAKE_TOOLCHAIN_FILE=$AVR_COMMON/gcc-avr.cmake'
```
```
$ source ~/.bashrc
```

## Arduino as ISP programmer
#### [Hex file for Arduino ](ArduinoISP.hex)
#### Arduino to ICSP Connection

1. Pin12 --- MISO (yellow)

2. VCC --- VCC (red)

3. Pin13 --- SCK (green)

4. Pin11 --- MOSI (blue)

5. Pin10 --- RESET (magenta)

6. GND --- GND (black)

#### Avrdude Usage

ArduinoISP programmer
```
$ avrdude -p atmega328p -c avrisp -P /dev/ttyUSB0 -b 19200 -e -U flash:w:firmware.hex
```

Usbasp programmer
```
$ avrdude -p atmega328p -c usbasp -e -U flash:w:firmware.hex
```

## AVR Software

[AVRlib](http://www.procyonengineering.com/embedded/avr/avrlib/)

[FatFs](http://elm-chan.org/fsw/ff/00index_e.html)

[avr-uip](https://code.google.com/p/avr-uip/)

[LUFA](http://www.fourwalledcubicle.com/LUFA.php)

[How To Write Simple Bootloader For AVR In C language](http://www.engineersgarage.com/embedded/avr-microcontroller-projects/How-To-Write-a-Simple-Bootloader-For-AVR-In-C-language)

[USnoobie Kit](http://www.seeedstudio.com/wiki/Usnoobie_Kit)

[HID-class USB Serial Communication for AVRs using V-USB](http://rayshobby.net/hid-class-usb-serial-communication-for-avrs-using-v-usb/)

[LUFA bootloader](http://fourwalledcubicle.com/blog/2013/03/the-new-lufa-bootloader/)

[DAN64, An AVR based 8-bit microcomputer](http://www.usebox.net/jjm/dan64/)

[Analyzing Compiler Generated Machine Code(.lss)](http://msoe.us/taylor/tutorial/ce2810/lssfiles)

[Using watchdog timer in your projects](http://www.embedds.com/using-watchdog-timer-in-your-projects/)

[Watchdog Reset](http://www.avrfreaks.net/forum/watchdog-reset-code-question)

[Three ways to reset](http://www.xappsoftware.com/wordpress/2013/06/24/three-ways-to-reset-an-arduino-board-by-code/)

[Using Standard IO Streams in AVR GCC](http://www.embedds.com/using-standard-io-streams-in-avr-gcc/)

[Merging Bootloader HEX-File and Application HEX-File to one](http://www.avrfreaks.net/forum/merging-bootloader-hex-file-and-application-hex-file-one)

#### How to convert .c to .S

```
$ avr-gcc -Os -S -DF_CPU=8000000 -mmcu=atmega328p somecode.c
```

#### .data section

> The .data section contains data definitions of initialized data items. Initialized data is data that has a value before the program begins running. These values are part of the executable file. They are loaded into memory when the executable file is loaded into memory for execution.

> The important thing to remember about the .data section is that the more initialized data items you define, the larger the executable file will be, and the longer it will take to load it from disk into memory when you run it.

#### .bss section

> Not all data items need to have values before the program begins running. When you’re reading data from a disk file, for example, you need to have a place for the data to go after it comes in from disk. Data buffers like that are defined in the .bss section of your program. You set aside some number of bytes for a buffer and give the buffer a name, but you don’t say what values are to be present in the buffer.

> There’s a crucial difference between data items defined in the .data section and data items defined in the .bss section: data items in the .data section add to the size of your executable file. Data items in the .bss section do not. A buffer that takes up 16,000 bytes (or more, sometimes much more) can be defined in .bss and add almost nothing (about 50 bytes for the description) to the executable file size.

#### Anatomy of AVR hex file

https://rajeshkovvuri.wordpress.com/2011/10/28/anatomy-of-avr-hex-file

https://en.wikipedia.org/wiki/Intel_HEX

#### Dump and Disassemble AVR uC Flash Memory

https://ucexperiment.wordpress.com/2015/01/04/dump-and-disassemble-avr-%C2%B5c-flash-memory/

#### [AT90USB82 - HWB connection options](http://www.avrfreaks.net/forum/at90usb82-hwb-connection-options)
