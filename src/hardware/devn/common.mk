ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

USEFILE=$(PROJECT_ROOT)/$(SECTION)/devn-$(SECTION).use
EXTRA_SRCVPATH = $(EXTRA_SRCVPATH_$(SECTION))

EXTRA_INCVPATH = $(PROJECT_ROOT)/public $(PROJECT_ROOT)/public/drvr $(USE_ROOT_nto)/usr/include/drvr

EXTRA_INCVPATH+=$(INSTALL_ROOT_nto)/usr/include/xilinx
EXTRA_LIBVPATH+=$(INSTALL_ROOT_nto)/usr/lib/xilinx

PRE_SRCVPATH = $(foreach var,$(filter a, $(VARIANTS)),$(CPU_ROOT)/$(subst $(space),.,$(patsubst a,dll,$(filter-out g, $(VARIANTS)))))

EXTRA_SILENT_VARIANTS = $(subst -, ,$(SECTION))
NAME=$(PROJECT)-$(SECTION)

define PINFO
PINFO DESCRIPTION=
endef


include $(MKFILES_ROOT)/qmacros.mk

ifeq ($(filter dll, $(VARIANTS)),)
DEBUG = -g
CCFLAGS += -Dio_net_dll_entry=io_net_dll_entry_$(SECTION)
USEFILE=
else
LIBS = drvrS pmS cacheS
endif

include $(PROJECT_ROOT)/$(SECTION)/pinfo.mk
-include $(PROJECT_ROOT)/roots.mk
   INSTALL_ROOT_nto = $(PROJECT_ROOT)/../../../install
   USE_INSTALL_ROOT=1
include $(MKFILES_ROOT)/qtargets.mk
