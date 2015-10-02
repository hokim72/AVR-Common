cmake_minimum_required(VERSION 3.1)

# MCU name
set(MCU atmega32u4)

# Target architecture (see library "Board Types" documentaion).
set(ARCH AVR8)

# Target board (see library "Board Types" documentation, NONE for projects not requiring
# LUFA board drivers). If USER is selected, put custom board drivers in a directory called
# "Board" inside the application directory.
set(BOARD USBKEY)

# Process frequency.
#	This will define a symbol, F_CPU, in all source code files equal to the
#	processor frequency in Hz. You can then use this symbol in your source code to
#	calculate timings. Do NOT tack on a 'UL' at the end, this will be done
#	automatically to create a 32-bit value in your source code.

#	This will be an integer division of F_USB below, as it is sourced by
#	F_USB after it has run through any CPU prescalers. Note that this value
#	does not *change* the processor frequency - it should merely be updated to
#	reflect the processor speed set externally so that the code can use accurate
#	software delays.
set(F_CPU 16000000)

# Input clock frequency.
#	This will define a symbol, F_USB, in all source code files equal to the
#	input clock frequency (before any prescaling is performed) in Hz. This value may
#	differ from F_CPU if prescaling is used on the latter, and is required as the
#	raw input clock is fed directly to the PLL sections of the AVR for high speed
#	clock genertion for the USB and other AVR subsections. Do NOT tack on a 'UL'
#	at the end, this will be done automatically to create a 32-bit value in your
#	source code.

#	If no clock division is performed on the input clock inside the AVR (via the
#	CPU clock adjust registers or the clock division fuses), this will be equal to F_CPU.
set(F_USB ${F_CPU})

# Starting byte address of the bootloader, as a byte address - computed via the formula
#	BOOT_START = ((FLASH_SIZE_KB - BOOT_SECTION_SIZE_KB) * 1024)

# Note that the bootloader size and start address given in AVRStudio is in words and not
# bytes, and so will need to be doubled to obtain the byte address needed by AVR-GCC.
set(FLASH_SIZE_KB 32)
set(BOOT_SECTION_SIZE_KB 4)
exec_program(
    "echo -n \"0x\"; echo \"obase=16; (${FLASH_SIZE_KB} - ${BOOT_SECTION_SIZE_KB})*1024\" | bc"
    OUTPUT_VARIABLE  BOOT_START
)
exec_program(
    "echo -n \"0x\"; echo \"obase=16; ${FLASH_SIZE_KB}*1024-96\" | bc"
    OUTPUT_VARIABLE  BOOT_START_TRAMPOLINES
)
exec_program(
    "echo -n \"0x\"; echo \"obase=16; ${FLASH_SIZE_KB}*1024-32\" | bc"
    OUTPUT_VARIABLE  BOOT_START_JUMPTABLE
)
exec_program(
    "echo -n \"0x\"; echo \"obase=16; ${FLASH_SIZE_KB}*1024-8\" | bc"
    OUTPUT_VARIABLE  BOOT_START_SIGNATURES
)

# Target file name (without extension).
set(TARGET BootloaderCDC)

# Path to the LUFA library
set(LUFA_PATH $ENV{AVR_COMMON}/lufa-LUFA-140928)

# LUFA library compile-time options and predefined tokens
set(LUFA_OPTS
	-D ORDERED_EP_CONFIG
	-D USE_STATIC_OPTIONS="\(USB_DEVICE_OPT_FULLSPEED | USB_OPT_REG_ENABLED | USB_OPT_AUTO_PLL\)"
	-D USB_DEVICE_ONLY
	-D NO_SOF_EVENTS
	-D USE_RAM_DESCRIPTORS
	-D NO_INTERNAL_SERIAL
	-D FIXED_CONTROL_ENDPOINT_SIZE=8
	-D DEVICE_STATE_AS_GPIOR=0
	-D FIXED_NUM_CONFIGURATIONS=1
	-D NO_DEVICE_REMOTE_WAKEUP
	-D NO_DEVICE_SELF_POWER
)	
string(REPLACE ";" " " LUFA_OPTS "${LUFA_OPTS}")

# Create the LUFA source path varaibles by including the LUFA root cmake file
include($ENV{AVR_COMMON}/lufa140928.cmake)

# List C source files here. (C dependencies are automatically generated.)
#set(SRCS ${TARGET}.c Descriptors.c BootloaderAPI.c ${LUFA_SRC_USB})
set(SRCS ${TARGET}.c Descriptors.c ${LUFA_SRC_USB})

# Place -D or -U options here for C sources
set(CPP_FLAGS 
	-DF_CPU=${F_CPU}UL
	-DF_USB=${F_USB}UL
	-DBOARD=BOARD_${BOARD} -DARCH=ARCH_${ARCH}
	-DBOOT_START_ADDR=${BOOT_START}UL
	${LUFA_OPTS}
)
string(REPLACE ";" " " CPP_FLAGS "${CPP_FLAGS}")

#---------------- Compiler Options C ----------------
#	-g*: generate debugging information
#	-O*: optimization level
#	-f...: tuning, see GCC manual and avr-libc documentation
#	-Wall..: warning level
#	-Wa,...: tell GCC to pass this to assembler.
#	-adhlns...: create assembler listing
set(C_FLAGS 
	-pipe
	-gdwarf-2
	-g2
	-mmcu=${MCU}
	-fshort-enums
	-fno-inline-small-functions 
	-fpack-struct 
	-Wall
	-fno-strict-aliasing
	-funsigned-char 
	-funsigned-bitfields 
	-ffunction-sections 
	-mrelax
	-fno-jump-tables
	-MMD -MP
	-x c -Os -std=gnu99 -Wstrict-prototypes
)
string(REPLACE ";" " " C_FLAGS "${C_FLAGS}")
set_source_files_properties(${SRCS} PROPERTIES COMPILE_FLAGS "${CPP_FLAGS} ${C_FLAGS}")

include_directories(${LUFA_PATH})

set(LD_FLAGS 
	-lm
	"-Wl,-Map=${TARGET}.map,--cref"
	"-Wl,--gc-sections"
	"-Wl,--relax"
	-mmcu=${MCU}
	"-Wl,--section-start=.text=${BOOT_START}"
	"-Wl,--section-start=.apitable_trampolines=${BOOT_START_TRAMPOLINES}"
	"-Wl,--undefined=BootloaderAPI_Trampolines"
	"-Wl,--section-start=.apitable_jumptable=${BOOT_START_JUMPTABLE}"
	"-Wl,--undefined=BootloaderAPI_JumpTable"
	"-Wl,--section-start=.aptitable_signatures=${BOOT_START_SIGNATURES}"
	"-Wl,--undefined=BootloaderAPI_Signatures"
)
string(REPLACE ";" " " LD_FLAGS "${LD_FLAGS}")

add_executable(${TARGET}.elf ${SRCS})
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

#set(PROG_TYPE avr109)
#set(PROG_ARGS -P /dev/ttyACM0 -b 57600)
set(PROG_TYPE usbasp)
set(PROG_ARGS)

add_custom_target(flash
    COMMAND ${AVRDUDE} -F -p ${MCU} -c ${PROG_TYPE} ${PROG_ARGS} -e -U flash:w:${TARGET}.hex
    DEPENDS ${TARGET}
)

