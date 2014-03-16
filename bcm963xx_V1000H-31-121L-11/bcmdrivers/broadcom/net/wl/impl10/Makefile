obj-$(CONFIG_BCM_WLAN) += wl.o
wl-objs += wl_dep.o

$(obj)/wl_dep.o:
	cp $(obj)/wl$(PROFILE).o_save $(obj)/wl_dep.o
	if [ ! -d $(INSTALL_DIR)/etc/wlan ]; then \
		rm -f $(INSTALL_DIR)/etc/wlan; \
		install -d $(INSTALL_DIR)/etc/wlan; \
	fi;
	for x in $(obj)/*map.bin_save; do \
		cp $$x $${x%_save}; \
		install -m 755 $${x%_save} $(INSTALL_DIR)/etc/wlan; \
	done
