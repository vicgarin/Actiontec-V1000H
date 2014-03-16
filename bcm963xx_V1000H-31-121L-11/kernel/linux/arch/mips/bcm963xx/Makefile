#
# Makefile for generic Broadcom MIPS boards
#
# Copyright (C) 2004 Broadcom Corporation
#

obj-y           := irq.o prom.o setup.o
obj-$(CONFIG_SMP)	+= smp-brcm.o

ifneq ($(strip $(CONFIG_BCM_HOSTMIPS_PWRSAVE)),)
obj-y += pwrmngtclk.o        
else
ifneq ($(strip $(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)),)
obj-y += pwrmngtclk.o        
endif
endif

SRCBASE         := $(TOPDIR)
EXTRA_CFLAGS    += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD) -I$(SRCBASE)/include -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
#EXTRA_CFLAGS    += -I$(INC_ADSLDRV_PATH) -DDBG
EXTRA_CFLAGS    += -I$(INC_ADSLDRV_PATH) 
cflags-$(CONFIG_MIPS_BRCM)      += -I$(srctree)/arch/mips/include/asm/bcm963xx
EXTRA_CFLAGS += -g
EXTRA_CFLAGS += $(BRCM_WERROR_CFLAGS)

ifneq ($(strip $(BUILD_SWMDK)),)
EXTRA_CFLAGS += -DSUPPORT_SWMDK
endif


ifeq "$(ADSL)" "ANNEX_B"
EXTRA_CFLAGS += -DADSL_ANNEXB
endif
ifeq "$(ADSL)" "SADSL"
EXTRA_CFLAGS += -DADSL_SADSL
endif
ifeq "$(ADSL)" "ANNEX_C"
EXTRA_CFLAGS += -DADSL_ANNEXC
endif
ifeq "$(BRCM_PHY_BONDING)" "y"
EXTRA_CFLAGS += -DSUPPORT_DSL_BONDING
endif

