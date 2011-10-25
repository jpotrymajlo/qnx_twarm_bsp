ROOT_DIR := $(notdir $(CURDIR))
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

HOST_MKIFS := mkifs

SUFFIXES := .build .ifs .bin

.PHONY: all clean

all: ifs-at91rm9200twarm.bin

flash: ipl-ifs-at91rm9200.bin

binary: ifs-at91rm9200warm.bin

clean: 
	$(RM_HOST) ifs-at91rm9200twarm.bin ipl-ifs-at91rm9200.bin ipl-at91rm9200.bin

ifs-at91rm9200twarm.bin: at91rm9200twarm.build
	$(HOST_MKIFS) -r../install $(MKIFSFLAGS) $^ $@

ipl-ifs-at91rm9200.bin: ifs-at91rm9200twarm.bin ../install/armle/boot/sys/ipl-at91rm9200twarm
	$(SHELL) ./mkflashimage $^ $@
