export MODULE:=$(shell basename $(CURDIR))
export OPENWSN_ROOT=$(CURDIR)

DIRS += $(OPENWSN_ROOT)/projects/common/01bsp_radiotimer \
		$(OPENWSN_ROOT)/projects/common/02drv_opentimers \
		$(OPENWSN_ROOT)/projects/common/03oos_openwsn \
		$(OPENWSN_ROOT)/openstack \
		$(OPENWSN_ROOT)/openapps \
		$(OPENWSN_ROOT)/kernel/openos \
		$(OPENWSN_ROOT)/drivers/common \
		$(OPENWSN_ROOT)/bsp/boards/dummyriot

INCLUDES += -I$(OPENWSN_ROOT)/kernel \
			-I$(OPENWSN_ROOT)/inc \
			-I$(OPENWSN_ROOT)/drivers/common \
			-I$(OPENWSN_ROOT)/bsp/boards/ \
			-I$(OPENWSN_ROOT)/bsp/boards/dummyriot \
			-I$(OPENWSN_ROOT)/openstack \
			-I$(OPENWSN_ROOT)/openapps

include $(RIOTBASE)/Makefile.base
