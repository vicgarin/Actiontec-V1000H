# File: bcmdrivers/Makefile
#
# Makefile for the Linux kernel modules.
#

LN_NAME=bcm9$(BRCM_CHIP)
LN_DRIVER_DIRS =
CLEAN_DRIVER_DIRS =

-include $(KERNEL_DIR)/.config


# All kernel drivers are instrumented, when BOUNCE is configured for datapath.
ifdef CONFIG_BRCM_BOUNCE_DATAPATH
ifeq ($(SUBLEVEL),30)
	EXTRA_CFLAGS  += -O0 -finstrument-functions
else
	CFLAGS  += -O0 -finstrument-functions
endif
endif

ifneq ($(CONFIG_BCM_BOARD),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_BOARD_IMPL) opensource/char/board/$(BRCM_BOARD)/$(LN_NAME);
obj-$(CONFIG_BCM_BOARD) += opensource/char/board/$(BRCM_BOARD)/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_CHIPINFO),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_CHIPINFO_IMPL) broadcom/char/chipinfo/$(BRCM_CHIPINFO)/$(LN_NAME);
obj-$(CONFIG_BCM_CHIPINFO) += broadcom/char/chipinfo/$(BRCM_CHIPINFO)/$(LN_NAME)/
endif


ifneq ($(CONFIG_BCM_SERIAL),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_SERIAL_IMPL) opensource/char/serial/$(LN_NAME);
obj-$(CONFIG_BCM_SERIAL) += opensource/char/serial/$(LN_NAME)/
endif

# Broadcom Packet Flow Forwarding Cache
ifneq ($(CONFIG_BCM_PKTFLOW),)
	LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_PKTFLOW_IMPL) broadcom/char/pktflow/$(LN_NAME);
	obj-$(CONFIG_BCM_PKTFLOW) += broadcom/char/pktflow/$(LN_NAME)/
endif

# Broadcom Packet CMF
ifneq ($(CONFIG_BCM_PKTCMF),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_PKTCMF_IMPL) broadcom/char/pktcmf/$(LN_NAME);
  obj-$(CONFIG_BCM_PKTCMF) += broadcom/char/pktcmf/$(LN_NAME)/
endif

ifeq ($(CONFIG_BCM_PKTCMF_IMPL),2)
  CLEAN_DRIVER_DIRS += broadcom/char/pktcmf/impl$(CONFIG_BCM_PKTCMF_IMPL)
endif

# Broadcom FAP
ifneq ($(CONFIG_BCM_FAP),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_FAP_IMPL) broadcom/char/fap/$(LN_NAME);
  obj-$(CONFIG_BCM_FAP) += broadcom/char/fap/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_PKTDMA),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_PKTDMA_IMPL) opensource/net/lib/pktdma/$(LN_NAME);
  obj-$(CONFIG_BCM_PKTDMA) += opensource/net/lib/pktdma/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_XTMCFG),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_XTMRT_IMPL) opensource/net/xtmrt/$(LN_NAME);
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_XTMCFG_IMPL) broadcom/char/xtmcfg/$(LN_NAME);
  obj-y += opensource/net/xtmrt/$(LN_NAME)/
  obj-$(CONFIG_BCM_XTMCFG) += broadcom/char/xtmcfg/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_EXT_BONDING),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_EXT_BONDING_IMPL) broadcom/char/ext_bonding/$(LN_NAME);
  obj-$(CONFIG_BCM_EXT_BONDING) += broadcom/char/ext_bonding/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_ADSL),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_ADSL_IMPL) broadcom/char/adsl/$(LN_NAME);
  obj-$(CONFIG_BCM_ADSL) += broadcom/char/adsl/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_PROCFS),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_PROCFS_IMPL) broadcom/char/bcmprocfs/$(LN_NAME);
  obj-$(CONFIG_BCM_PROCFS) += broadcom/char/bcmprocfs/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_ENDPOINT),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_ENDPOINT_IMPL) broadcom/char/endpoint/$(LN_NAME);
  obj-$(CONFIG_BCM_ENDPOINT) += broadcom/char/endpoint/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_BCMPROF),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_BCMPROF_IMPL) broadcom/char/profiler/$(LN_NAME);
  obj-$(CONFIG_BCM_BCMPROF) += broadcom/char/profiler/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_BCMDSP),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_BCMDSP_IMPL) broadcom/char/dspapp/$(LN_NAME);
  obj-$(CONFIG_BCM_BCMDSP) += broadcom/char/dspapp/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_DECT),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_DECT_IMPL) broadcom/char/dect/$(LN_NAME);
  CLEAN_DRIVER_DIRS += broadcom/char/dect/impl$(CONFIG_BCM_DECT_IMPL)
  obj-$(CONFIG_BCM_DECT) += broadcom/char/dect/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_DECTSHIM),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_DECTSHIM_IMPL) broadcom/char/dectshim/$(LN_NAME);
  obj-$(CONFIG_BCM_DECTSHIM) += broadcom/char/dectshim/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_P8021AG),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_P8021AG_IMPL) broadcom/char/p8021ag/$(LN_NAME);
  obj-$(CONFIG_BCM_P8021AG) += broadcom/char/p8021ag/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_ENET),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_ENET_IMPL) opensource/net/enet/$(LN_NAME);
  obj-$(CONFIG_BCM_ENET) += opensource/net/enet/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_GPON),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_GPON_IMPL) broadcom/char/gpon/$(LN_NAME);
  obj-$(CONFIG_BCM_GPON) += broadcom/char/gpon/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_LASER),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_LASER_IMPL) broadcom/char/laser/$(LN_NAME);
  obj-$(CONFIG_BCM_LASER) += broadcom/char/laser/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_MoCA),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_MoCA_IMPL) broadcom/char/moca/$(LN_NAME);
  obj-$(CONFIG_BCM_MoCA) += broadcom/char/moca/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_I2C_BUS),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_I2C_BUS_IMPL) broadcom/char/i2c/busses/$(LN_NAME);
  obj-$(CONFIG_BCM_I2C_BUS) += broadcom/char/i2c/busses/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_I2C_CHIP),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_I2C_CHIP_IMPL) broadcom/char/i2c/chips/$(LN_NAME);
  obj-$(CONFIG_BCM_I2C_CHIP) += broadcom/char/i2c/chips/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_VLAN),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_VLAN_IMPL) broadcom/char/vlan/$(LN_NAME);
  obj-$(CONFIG_BCM_VLAN) += broadcom/char/vlan/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_USB),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_USB_IMPL) broadcom/net/usb/$(LN_NAME);
  obj-$(CONFIG_BCM_USB) += broadcom/net/usb/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_ISDN),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_ISDN_IMPL) broadcom/char/isdn/$(LN_NAME);
  obj-$(CONFIG_BCM_ISDN) += broadcom/char/isdn/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_WLAN),)

ifeq ($(strip $(WL)),)
ifneq ($(CONFIG_BCM_WLAN_IMPL_SELECT),"")
   WL=IMPL$(subst ",,$(CONFIG_BCM_WLAN_IMPL_SELECT))
endif
endif

ifneq ($(WL),)
   CONFIG_BCM_WLAN_IMPL=$(patsubst IMPL%,%,$(WL))  
endif
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_WLAN_IMPL) broadcom/net/wl/$(LN_NAME);
  obj-$(CONFIG_BCM_WLAN) += broadcom/net/wl/$(LN_NAME)/
  
endif

ifneq ($(CONFIG_BCM_SPU),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_SPU_IMPL) broadcom/char/spudd/$(LN_NAME);
  obj-$(CONFIG_BCM_SPU) += broadcom/char/spudd/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_PWRMNGT),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_PWRMNGT_IMPL) broadcom/char/pwrmngt/$(LN_NAME);
  obj-$(CONFIG_BCM_PWRMNGT) += broadcom/char/pwrmngt/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_TRNG),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_TRNG_IMPL) opensource/char/trng/$(LN_NAME);
  obj-$(CONFIG_BCM_TRNG) += opensource/char/trng/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_WATCHDOG),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_WATCHDOG_IMPL) broadcom/char/watchdog/$(LN_NAME);
  obj-$(CONFIG_BCM_WATCHDOG) += broadcom/char/watchdog/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_INGQOS),)
	LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_INGQOS_IMPL) broadcom/char/ingqos/$(LN_NAME);
	obj-$(CONFIG_BCM_INGQOS) += broadcom/char/ingqos/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_BPM),)
	LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_BPM_IMPL) broadcom/char/bpm/$(LN_NAME);
	obj-$(CONFIG_BCM_BPM) += broadcom/char/bpm/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_ARL),)
	LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_ARL_IMPL) broadcom/char/arl/$(LN_NAME);
	obj-$(CONFIG_BCM_ARL) += broadcom/char/arl/$(LN_NAME)/
endif


.PHONY : applinks symlinks clean

applinks:
ifneq ($(CONFIG_BCM_PKTCMF),)
ifeq ($(CONFIG_BCM_PKTCMF_IMPL),2)
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/char/pktcmf/impl$(CONFIG_BCM_PKTCMF_IMPL) applinks
endif
endif


symlinks: applinks
	find . -lname "*" -name "$(LN_NAME)" -print -exec rm -f "{}" ";"
	$(CONFIG_SHELL) -c "$(LN_DRIVER_DIRS)"


clean:
	for dir in $(CLEAN_DRIVER_DIRS) ; do \
		$(MAKE) -C $$dir clean; \
	done
