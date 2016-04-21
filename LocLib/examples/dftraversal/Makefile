# clone https://github.com/sudar/Arduino-Makefile@e30bb5 into ARDMK_DIR
# Notes for windows:
# make.exe v3.81 must be from avr, not from cygwin
# egrep.exe should be from cygwin

# Hide the configuration output
ARDUINO_QUIET = 1
PROJECT = dftraversal

ifeq ($(OS),Windows_NT)
	C = ../../../../../../../../

	ARDMK_DIR		= C:/Progra~2/Arduino-Makefile
	ARDUINO_DIR		= $CProgra~2/Arduino
	#AVR_TOOLS_DIR	= C:/Progra~2/Arduino/hardware/tools/avr
	AVR_TOOLS_DIR	= C:/WinAVR-20100110

	ARCHITECTURE	= avr
	BOARD_TAG		= uno
	SERIAL			= COM
else
	AVR_TOOLS_DIR	= /usr
	ARDMK_DIR		= /opt/Arduino-Makefile
	BOARD_TAG		= atmega328
	SERIAL			= /dev/ttyUSB
endif
PARTNO = atmega328p

MONITOR_PORT = 25

# This must be set otherwise stk500_getsync() error appears
AVRDUDE_ARD_BAUDRATE = 115200

# This part is used for quick upload to multiple targets on Linux like this:
# make upload_all 0 1 2
ifeq (upload_all, $(firstword $(MAKECMDGOALS)))
  MONITOR_PORTS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  # Turn monitor ports into do-nothing targets
  $(eval $(MONITOR_PORTS):;@:)

.PHONY: upload_all
upload_all:
	for port in $(MONITOR_PORTS) ; do \
		avrdude -q -V -D -p $(PARTNO) -c arduino -b $(AVRDUDE_ARD_BAUDRATE) -P $(SERIAL)$$port -U flash:w:build-$(BOARD_TAG)/$(PROJECT).hex:i ; \
	done
else
include $(ARDMK_DIR)/Arduino.mk
endif
