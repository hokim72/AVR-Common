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

1. Pin12 to ICSP MISO

2. Arduino VCC	to ICSP VCC

3. Arduino Pin13 to ICSP SCK

4. Arduino Pin11 to ICSP MOSI

5. Arduino Pin10 to ICSP RESET

6. Arduino GND to GND

#### Avrdude Usage

```
$ avrdude -p atmega328p -c avrisp -P /dev/ttyUSB0 -b 19200 -e -U flash:w:firmware.hex
```

## AVR Software

http://homepage.hispeed.ch/peterfleury/avr-software.html

[busy-wait delay loops](http://www.nongnu.org/avr-libc/user-manual/group__util__delay.html)

[LUFA](http://www.fourwalledcubicle.com/LUFA.php)

[How To Write Simple Bootloader For AVR In C language](http://www.engineersgarage.com/embedded/avr-microcontroller-projects/How-To-Write-a-Simple-Bootloader-For-AVR-In-C-language)

#### AVR USB Bootloader

[USnoobie Kit](http://www.seeedstudio.com/wiki/Usnoobie_Kit)

[HID-class USB Serial Communication for AVRs using V-USB](http://rayshobby.net/hid-class-usb-serial-communication-for-avrs-using-v-usb/)

[Installing Arduino Bootloader on an ATmega32u4](http://electronut.in/bootloader-atmega32u4/)

https://github.com/arduino/Arduino/tree/master/hardware/arduino/avr/bootloaders/caterina

[DAN64, An AVR based 8-bit microcomputer](http://www.usebox.net/jjm/dan64/)
