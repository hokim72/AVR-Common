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

## AVR Software

http://homepage.hispeed.ch/peterfleury/avr-software.html

[busy-wait delay loops](http://www.nongnu.org/avr-libc/user-manual/group__util__delay.html)

[LUFA](http://www.fourwalledcubicle.com/LUFA.php)

#### AVR USB Bootloader

[USnoobie Kit](http://www.seeedstudio.com/wiki/Usnoobie_Kit)

[HID-class USB Serial Communication for AVRs using V-USB](http://rayshobby.net/hid-class-usb-serial-communication-for-avrs-using-v-usb/)

