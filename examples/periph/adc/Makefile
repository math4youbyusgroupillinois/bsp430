PLATFORM ?= exp430fr5739
# Not yet ported to ADC12_B.
TEST_PLATFORMS_EXCLUDE=wolverine
MODULES=$(MODULES_PLATFORM)
MODULES += $(MODULES_UPTIME)
MODULES += $(MODULES_CONSOLE)
MODULES += utility/tlv
SRC=main.c
include $(BSP430_ROOT)/make/Makefile.common
