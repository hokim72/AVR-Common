# lufa_build.mk
if (NOT DEFINED BOARD)
	set(BOARD NONE)
endif()
if (NOT DEFINED OPTIMIZATION)
	set(OPTIMIZATION s)
endif()
if (NOT DEFINED C_STANDARD)
	set(C_STANDARD gnu99)
endif()
if (NOT DEFINED CPP_STANDARD)
	set(CPP_STANDARD gnu++98)
endif()
if (NOT DEFINED LINKER_RELAXATIONS)
	set(LINKER_RELAXATIONS Y)
endif()
if (NOT DEFINED DEBUG_FORMAT)
	set(DEBUG_FORMAT dwarf-2)
endif()
if (NOT DEFINED DEBUG_LEVEL)
	set(DEBUG_LEVEL 2)
endif()

set(BASE_CC_FLAGS "-pipe -g${DEBUG_FORMAT} -g${DEBUG_LEVEL}")
if (ARCH STREQUAL AVR8)
    set(BASE_CC_FLAGS "${BASE_CC_FLAGS} -mmcu=${MCU} -fshort-enums -fno-inline-small-functions -fpack-struct")
endif()
set(BASE_CC_FLAGS "${BASE_CC_FLAGS} -Wall -fno-strict-aliasing -funsigned-char -funsigned-bitfields -ffunction-sections")
set(BASE_CC_FLAGS "${BASE_CC_FLAGS} -I${CMAKE_SOURCE_DIR} -I${LUFA_PATH}/..")
set(BASE_CC_FLAGS "${BASE_CC_FLAGS} -DARCH=ARCH_${ARCH} -DBOARD=BOARD_${BOARD} -DF_USB=${F_USB}UL")
if (F_CPU)
	set(BASE_CC_FLAGS "${BASE_CC_FLAGS} -DF_CPU=${F_CPU}UL")
endif()
if (LINKER_RELAXATIONS STREQUAL Y)
	set(BASE_CC_FLAGS "${BASE_CC_FLAGS} -mrelax")
endif()
set(BASE_CC_FLAGS "${BASE_CC_FLAGS} -fno-jump-tables")

set(BASE_C_FLAGS "-x c -O${OPTIMIZATION} -std=${C_STANDARD} -Wstrict-prototypes")
set(BASE_CPP_FLAGS "-x c++ -O${OPTIMIZATION} -std=${CPP_STANDARD} -Wstrict-prototypes")
set(BASE_ASM_FLAGS "-x assembler-with-cpp")

set(BASE_LD_FLAGS "-lm -Wl,-Map=${TARGET}.map,--cref -Wl,--gc-sections")
if (LINKER_RELAXATIONS STREQUAL Y)
	set(BASE_LD_FLAGS "${BASE_LD_FLAGS} -Wl,--relax")
endif()
if (ARCH STREQUAL AVR8)
	set(BASE_LD_FLAGS "${BASE_LD_FLAGS} -mmcu=${MCU}")
endif()

execute_process(
	COMMAND ${SIZE} --help 
	COMMAND grep -- --mcu
	OUTPUT_VARIABLE SIZE_MCU_FLAG
)
if (SIZE_MCU_FLAG)
set(SIZE_MCU_FLAG "--mcu=${MCU}")
endif()

execute_process(
	COMMAND ${SIZE} --help 
	COMMAND grep -- --format=.*avr
	OUTPUT_VARIABLE SIZE_FORMAT_FLAG
)
if (SIZE_FORMAT_FLAG)
set(SIZE_FORMAT_FLAG "--format=avr")
endif()

# lufa_sources.mk
set(LUFA_ROOT_PATH ${LUFA_PATH})
set(LUFA_SRC_USB_COMMON 
${LUFA_ROOT_PATH}/Drivers/USB/Core/${ARCH}/USBController_${ARCH}.c
${LUFA_ROOT_PATH}/Drivers/USB/Core/${ARCH}/USBInterrupt_${ARCH}.c
${LUFA_ROOT_PATH}/Drivers/USB/Core/ConfigDescriptors.c
${LUFA_ROOT_PATH}/Drivers/USB/Core/Events.c
${LUFA_ROOT_PATH}/Drivers/USB/Core/USBTask.c
${LUFA_ROOT_PATH}/Drivers/USB/Class/Common/HIDParser.c
)
set(LUFA_SRC_USB_HOST
${LUFA_ROOT_PATH}/Drivers/USB/Core/${ARCH}/Host_${ARCH}.c
${LUFA_ROOT_PATH}/Drivers/USB/Core/${ARCH}/Pipe_${ARCH}.c
${LUFA_ROOT_PATH}/Drivers/USB/Core/${ARCH}/PipeStream_${ARCH}.c
${LUFA_ROOT_PATH}/Drivers/USB/Core/HostStandardReq.c
${LUFA_SRC_USB_COMMON}
)
set(LUFA_SRC_USB_DEVICE
${LUFA_ROOT_PATH}/Drivers/USB/Core/${ARCH}/Device_${ARCH}.c
${LUFA_ROOT_PATH}/Drivers/USB/Core/${ARCH}/Endpoint_${ARCH}.c
${LUFA_ROOT_PATH}/Drivers/USB/Core/${ARCH}/EndpointStream_${ARCH}.c
${LUFA_ROOT_PATH}/Drivers/USB/Core/DeviceStandardReq.c
${LUFA_SRC_USB_COMMON}
)
set(LUFA_SRC_USBCLASS_DEVICE
${LUFA_ROOT_PATH}/Drivers/USB/Class/Device/AudioClassDevice.c
${LUFA_ROOT_PATH}/Drivers/USB/Class/Device/CDCClassDevice.c
${LUFA_ROOT_PATH}/Drivers/USB/Class/Device/HIDClassDevice.c
${LUFA_ROOT_PATH}/Drivers/USB/Class/Device/MassStorageClassDevice.c
${LUFA_ROOT_PATH}/Drivers/USB/Class/Device/MIDIClassDevice.c
${LUFA_ROOT_PATH}/Drivers/USB/Class/Device/PrinterClassDevice.c
${LUFA_ROOT_PATH}/Drivers/USB/Class/Device/RNDISClassDevice.c
)
set(LUFA_SRC_USBCLASS_HOST
${LUFA_ROOT_PATH}/Drivers/USB/Class/Host/AndroidAccessoryClassHost.c
${LUFA_ROOT_PATH}/Drivers/USB/Class/Host/AudioClassHost.c
${LUFA_ROOT_PATH}/Drivers/USB/Class/Host/CDCClassHost.c
${LUFA_ROOT_PATH}/Drivers/USB/Class/Host/HIDClassHost.c
${LUFA_ROOT_PATH}/Drivers/USB/Class/Host/MassStorageClassHost.c
${LUFA_ROOT_PATH}/Drivers/USB/Class/Host/MIDIClassHost.c
${LUFA_ROOT_PATH}/Drivers/USB/Class/Host/PrinterClassHost.c
${LUFA_ROOT_PATH}/Drivers/USB/Class/Host/RNDISClassHost.c
${LUFA_ROOT_PATH}/Drivers/USB/Class/Host/StillImageClassHost.c
)
set(LUFA_SRC_USB
${LUFA_SRC_USB_COMMON} ${LUFA_SRC_USB_HOST} ${LUFA_SRC_USB_DEVICE}
)
list(SORT LUFA_SRC_USB)
list(REMOVE_DUPLICATES LUFA_SRC_USB)
set(LUFA_SRC_USBCLASS
${LUFA_SRC_USBCLASS_DEVICE} ${LUFA_SRC_USBCLASS_HOST}
)
set(LUFA_SRC_TEMPERATURE ${LUFA_ROOT_PATH}/Drivers/Board/Temperature.c)
