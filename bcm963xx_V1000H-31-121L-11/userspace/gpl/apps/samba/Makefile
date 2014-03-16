
all:
	if [ ! -e ./source/Makefile ];then \
	cd ./source; ./configure --host=mips-linux-uclibc --target=mips-linux-uclibc --disable-developer --enable-cups=no --enable-fam=no --enable-shared=yes --enable-iprint=no --enable-pie=no --without-ldap --without-cifsupcall --without-cifsmount --without-cluster-support --without-acl-support --with-privatedir=/var/samba --with-lockdir=/var/samba/locks --with-configdir=/var/samba --with-piddir=/var/samba --with-logfilebase=/var/samba SMB_BUILD_CC_NEGATIVE_ENUM_VALUES=no linux_getgrouplist_ok=no samba_cv_have_longlong=yes samba_cv_HAVE_IFACE_IFCONF=yes ; \
	fi

	$(MAKE) -C ./source srcdir=./ builddir=./
	install -m 755 ./source/bin/smbd $(INSTALL_DIR)/bin/smbd
	install -m 755 ./source/bin/smbpasswd $(INSTALL_DIR)/bin/smbpasswd
ifneq ($(strip $(AEI_VDSL_CUSTOMER_TELUS)),)
	install -m 444 ./source/conf/smb_telus.conf $(INSTALL_DIR)/etc/smb.conf
else
	install -m 444 ./source/conf/smb.conf $(INSTALL_DIR)/etc
endif

clean:
	if [ -f ./source/Makefile ];then $(MAKE) -C ./source clean srcdir=./ builddir=./;fi;

