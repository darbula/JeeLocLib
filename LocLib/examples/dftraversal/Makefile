# Arduino Makefile for Linux
# Please refer to https://github.com/sudar/Arduino-Makefile

# Hide the configuration output
ARDUINO_QUIET = 1

# Paths and directories
AVR_TOOLS_DIR = /usr
ARDMK_DIR = /opt/Arduino-Makefile

BOARD_TAG = atmega328

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
		avrdude -q -V -D -p $(BOARD_TAG)p -c arduino -b $(AVRDUDE_ARD_BAUDRATE) -P /dev/ttyUSB$$port -U flash:w:build-$(BOARD_TAG)/traversal.hex:i ; \
	done
else
include $(ARDMK_DIR)/Arduino.mk
endif
