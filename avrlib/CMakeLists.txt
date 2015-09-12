cmake_minimum_required(VERSION 3.1)

set(MCU atmega328p)
#set(F_CPU 1000000UL)
#set(BAUD 9600UL)
set(OPTIMIZATION s)
set(C_STANDARD gnu99)

set(TARGET avrlib-${MCU})

#set(CPP_FLAGS "-DF_CPU=${F_CPU} -DBAUD=${BAUD} -mmcu=${MCU}")
set(CPP_FLAGS "-mmcu=${MCU}")
set(C_FLAGS "-O${OPTIMIZATION} -g -std=${C_STANDARD} -Wall -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -ffunction-sections -fdata-sections")

set(SRCS buffer.c uart.c rprintf.c timer.c uartsw.c uartsw2.c pulse.c a2d.c extint.c servo.c)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} conf)
add_library(${TARGET} STATIC ${SRCS})
set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS "${CPP_FLAGS} ${C_FLAGS}")
