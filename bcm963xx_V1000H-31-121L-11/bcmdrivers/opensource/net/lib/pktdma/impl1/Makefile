# File: bcmdrivers/opensource/char/pktdma
#
# Makefile for the BCM63xx pktdma driver

ifneq ($(strip $(CONFIG_BCM_FAP)),)
obj-$(CONFIG_BCM_PKTDMA) += bcmPktDmaEthDqm.o
obj-$(CONFIG_BCM_PKTDMA) += bcmPktDmaXtmDqm.o
obj-$(CONFIG_BCM_PKTDMA) += bcmPktDmaEthIudma.o
obj-$(CONFIG_BCM_PKTDMA) += bcmPktDmaXtmIudma.o
obj-$(CONFIG_BCM_PKTDMA) += bcmPktDma.o
obj-$(CONFIG_BCM_PKTDMA) += bcmPktDmaBds.o
else
obj-$(CONFIG_BCM_PKTDMA) += bcmPktDmaEthIudma.o
obj-$(CONFIG_BCM_PKTDMA) += bcmPktDmaXtmIudma.o
obj-$(CONFIG_BCM_PKTDMA) += bcmPktDmaBds.o
endif

EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) 
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/fap/impl$(CONFIG_BCM_FAP_IMPL)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/fap/impl$(CONFIG_BCM_FAP_IMPL)/4ke
ifeq ($(CONFIG_BCM_XTMRT_IMPL),1)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/xtmrt/impl3
else
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/xtmrt/impl$(CONFIG_BCM_XTMRT_IMPL)
endif
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

EXTRA_CFLAGS += -Werror -Wall -Wfatal-errors

ifneq ($(strip $(BRCM_DRIVER_GPON)),)
EXTRA_CFLAGS += -DDBL_DESC
endif

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s

