cmake_minimum_required(VERSION 3.1)

set(MCU atmega328p)
set(F_CPU 16000000L)
set(BAUD 115200)
set(OPTIMIZATION s)

set(PROG_TYPE usbasp)
set(PROG_ARGS)
set(FUSE_FLAGS -U lfuse:w:0xff:m -U hfuse:w:0xd4:m -U efuse:w:0x05:m)

set(TARGET optiboot_atmega328)

set(CPP_FLAGS "-DF_CPU=${F_CPU} -DBAUD_RATE=${BAUD} -DLED_START_FLASHES=3 -DSUPPORT_EEPROM")
set(C_FLAGS "-g -Wall -O${OPTIMIZATION} -mmcu=${MCU} -fno-split-wide-types -mrelax")
set(LD_FLAGS "-mmcu=${MCU} -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe -Wl,--relax -nostartfiles -nostdlib")

set(SRCS optiboot.c)
set_source_files_properties(${SRCS} PROPERTIES COMPILE_FLAGS "${CPP_FLAGS} ${C_FLAGS}")

add_executable(${TARGET}.elf ${SRCS})
target_link_libraries(${TARGET}.elf -lc)
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
     COMMAND ${AVRDUDE} -p ${MCU} -c ${PROG_TYPE} ${PROG_ARGS} -e -U flash:w:${TARGET}.hex ${FUSE_FLAGS}
     DEPENDS ${TARGET}
)

add_custom_target(fflash
     COMMAND ${AVRDUDE} -F -p ${MCU} -c ${PROG_TYPE} ${PROG_ARGS} -e -U flash:w:${TARGET}.hex ${FUSE_FLAGS}
     DEPENDS ${TARGET}
)
