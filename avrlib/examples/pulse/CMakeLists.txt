cmake_minimum_required(VERSION 3.1)

set(MCU atmega328p)
set(F_CPU 16000000UL)
set(OPTIMIZATION s)
set(C_STANDARD gnu99)

set(PROG_TYPE arduino)
set(PROG_ARGS -P /dev/ttyUSB0 -b 115200)

set(TARGET pulsetest)

set(CPP_FLAGS "-DF_CPU=${F_CPU} -mmcu=${MCU}")
set(C_FLAGS "-O${OPTIMIZATION} -g -std=${C_STANDARD} -Wall -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -ffunction-sections -fdata-sections")
set(LD_FLAGS "-mmcu=${MCU} -Wl,-Map,${TARGET}.map -Wl,--gc-sections")

set(AVRLIB $ENV{AVR_COMMON}/avrlib)
include_directories(${CMAKE_SOURCE_DIR} ${AVRLIB})
set(AVRLIB_SRCS 
	${AVRLIB}/buffer.c
	${AVRLIB}/uart.c
	${AVRLIB}/timer.c
	${AVRLIB}/rprintf.c
	${AVRLIB}/pulse.c
)

set(SRCS ${TARGET}.c ${AVRLIB_SRCS})
set_source_files_properties(${SRCS} PROPERTIES COMPILE_FLAGS "${CPP_FLAGS} ${C_FLAGS}")

add_executable(${TARGET}.elf ${SRCS})
#set_target_properties(${TARGET}.elf PROPERTIES COMPILE_FLAGS "${CPP_FLAGS} ${C_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${LD_FLAGS}")

add_custom_target(${TARGET} ALL
	COMMAND ${OBJCOPY} -O ihex -R .eeprom -R .fuse -R .lock -R .signature ${TARGET}.elf ${TARGET}.hex
	COMMAND ${OBJCOPY} -O ihex -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 --no-change-warnings ${TARGET}.elf ${TARGET}.eep
	COMMAND ${OBJCOPY} -O binary -R .eeprom -R .fuse -R .lock -R .signature ${TARGET}.elf ${TARGET}.bin
	COMMAND ${OBJDUMP} -h -d -S -z ${TARGET}.elf > ${TARGET}.lss
	COMMAND ${NM} -n ${TARGET}.elf > ${TARGET}.sym
	COMMAND ${SIZE} -C --mcu=${MCU} ${TARGET}.elf
	DEPENDS ${TARGET}.elf
)

add_custom_target(flash
	 COMMAND ${AVRDUDE} -p ${MCU} -c ${PROG_TYPE} ${PROG_ARGS} -e -U flash:w:${TARGET}.hex
	 DEPENDS ${TARGET}
)

add_custom_target(fflash
	 COMMAND ${AVRDUDE} -F -p ${MCU} -c ${PROG_TYPE} ${PROG_ARGS} -e -U flash:w:${TARGET}.hex
	 DEPENDS ${TARGET}
)
