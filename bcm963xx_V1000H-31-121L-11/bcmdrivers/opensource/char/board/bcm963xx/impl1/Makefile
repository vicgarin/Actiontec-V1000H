# Makefile for the bcm963xx board drivers
#
ifneq ($(strip $(AEI_VDSL_CUSTOMER_NCS)),)
obj-y := board.o aei_bcm63xx_flash.o bcm63xx_flash.o bcm63xx_led.o bcm63xx_gpio.o
else
obj-y := board.o bcm63xx_flash.o bcm63xx_led.o bcm63xx_gpio.o
endif

ifeq ($(strip $(BRCM_CHIP)),6816)
obj-y += spidevices.o
endif

ifeq ($(strip $(BRCM_CHIP)),63268)
obj-y += spidevices.o
endif


ifneq ($(strip $(CONFIG_BCM_CPLD1)),)
    obj-y += bcmcpld1.o
	EXTRA_CFLAGS += -DCONFIG_BCM_CPLD1
endif

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

ifneq ($(strip $(AEI_VDSL_CUSTOMER_NCS)),)
EXTRA_CFLAGS += $(ACTION_TEC_DFLAGS)
else
ifneq ($(strip $(AEI_VDSL_BCMSDK)),)
EXTRA_CFLAGS += $(ACTION_TEC_DFLAGS)
else
EXTRA_CFLAGS += -Werror -Wfatal-errors
endif
endif

EXTRA_CFLAGS += -g

ifneq ($(strip $(CONFIG_BCM_WLAN)),)
	EXTRA_CFLAGS += -DWIRELESS
endif

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s

