# clone https://github.com/sudar/Arduino-Makefile@e30bb5 into ARDMK_DIR
# Notes for windows:
# make.exe v3.81 must be from avr, not from cygwin

# To hide the configuration output
ARDUINO_QUIET = 1

ARDMK_DIR         = C:/Progra~2/Arduino-Makefile
ARDUINO_DIR       = C:/Progra~2/Arduino
AVR_TOOLS_DIR     = C:/Progra~2/Arduino/hardware/tools/avr
#AVR_TOOLS_DIR     = C:/WinAVR-20100110
BOARD_TAG         = atmega328
MONITOR_PORT = 25
# This must be set otherwise stk500_getsync() error appears
AVRDUDE_ARD_BAUDRATE = 115200

### USER_LIB_PATH
### Path to where the your project's libraries are stored.
#USER_LIB_PATH    :=  $(PROJECT_DIR)/lib

### Set the C standard to be used during compilation. Documentation (https://github.com/WeAreLeka/Arduino-Makefile/blob/std-flags/arduino-mk-vars.md#cflags_std)
#CFLAGS_STD        = -std=gnu11

### Set the C++ standard to be used during compilation. Documentation (https://github.com/WeAreLeka/Arduino-Makefile/blob/std-flags/arduino-mk-vars.md#cxxflags_std)
#CXXFLAGS_STD      = -std=gnu++11

### Flags you might want to set for debugging purpose.
#CXXFLAGS         += -pedantic -Wall -Wextra

#CPPFLAGS = -DMAX_NEIGHBORS=7 -DMAX_AOA_MEASUREMENTS=112

# This part is used for quick upload to multiple targets on Windows like this:
# make upload_all 25 27 28
ifeq (upload_all, $(firstword $(MAKECMDGOALS)))
  MONITOR_PORTS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  # turn monitor ports into do-nothing targets
  $(eval $(MONITOR_PORTS):;@:)

.PHONY: upload_all
upload_all:
	for port in $(MONITOR_PORTS) ; do \
	    start avrdude -q -V -D -p $(BOARD_TAG)p -c arduino -b $(AVRDUDE_ARD_BAUDRATE) -P COM$$port -U flash:w:build-$(BOARD_TAG)/pingpong.hex:i ; \
	done
else
include $(ARDMK_DIR)/Arduino.mk
endif

