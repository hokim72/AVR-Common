## Prerequisite
---
#### [CMake](http://www.cmake.org)
#### [Atmel AVR8 Toolchain](http://www.atmel.com/forms/software-download.aspx?target=tcm:26-64140)
## Setup
---
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
```
```
$ source ~/.bashrc
```
