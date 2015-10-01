set(LUFA_ROOT_PATH ${LUFA_PATH}/LUFA)
if (NOT DEFINED ARCH)
	set(ARCH AVR8)
endif()
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
set(LUFA_SRC_SERIAL ${LUFA_ROOT_PATH}/Drivers/Peripheral/${ARCH}/Serial_${ARCH}.c)
set(LUFA_SRC_TWI ${LUFA_ROOT_PATH}/Drivers/Peripheral/${ARCH}/TWI_${ARCH}.c)
set(LUFA_SRC_SCHEDULER ${LUFA_ROOT_PATH}/Scheduler/Scheduler.c)
set(LUFA_SRC_PLATFORM)
set(LUFA_SRC_ALL_FILES
	${LUFA_SRC_USB}
	${LUFA_SRC_USBCLASS}
	${LUFA_SRC_TEMPERATURE}
	${LUFA_SRC_SERIAL}
	${LUFA_SRC_TWI}
	${LUFA_SRC_PLATFORM}
)
