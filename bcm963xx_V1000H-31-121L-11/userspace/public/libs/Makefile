#****************************************************************************
#
# Copyright (c) 2006-2008 Broadcom Corporation
#
# 
# 
# This program is free software; you can redistribute it and/or modify 
# it under the terms of the GNU General Public License, version 2, as published by  
# the Free Software Foundation (the "GPL"). 
# 
#
# 
# This program is distributed in the hope that it will be useful,  
# but WITHOUT ANY WARRANTY; without even the implied warranty of  
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
# GNU General Public License for more details. 
#  
# 
#  
#   
# 
# A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by 
# writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
# Boston, MA 02111-1307, USA. 
#
#
#****************************************************************************

#
# Add new libraries to BASE_SUBDIRS or OTHER_SUBDIRS.
# Do not set MODULAR_SUBDIRS here.  It is set dynamically based on
# "hint" files in the filesystem.
#
OTHER_SUBDIRS := openssl libpcap

SUBDIRS = $(BASE_SUBDIRS) $(OTHER_SUBDIRS) $(MODULAR_SUBDIRS)


# === this is the default target ===
# Need to add a level of indirection here so that MODULAR_SUBDIRS can be
# populated in make.common before we build.
#
all : make_all_subdirs


#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common

ifneq ($(strip $(BUILD_VODSL)),)
include $(BUILD_DIR)/make.voice
endif


#
# We need to declare these directory names as phony targets,
# otherwise make will think the directories themselves are
# the targets and will never execute the cmds.
# See GNU Make, 4.6 
#
.PHONY : $(SUBDIRS)
make_all_subdirs : $(SUBDIRS)


clean: generic_subdir_clean



##############################################################
# 
# Rules for inidividual libs
# libs can be compiled or not depending on the profile (which
# is modified via make menuconfig)
#
##############################################################


$(BASE_SUBDIRS) $(MODULAR_SUBDIRS): 
	$(MAKE) -C $@


# DO_BUILD_OPENSSL is set in make.common
ifneq ($(strip $(DO_BUILD_OPENSSL)),)
openssl: sanity_check
	(tar xkfj openssl.tar.bz2 2> /dev/null || true)
	$(MAKE) -j 1 -C openssl all
	$(MAKE) -j 1 -C openssl install
else
openssl: sanity_check
	@echo "skipping openssl (not configured)"
endif


ifneq ($(strip $(BUILD_LIBPCAP)),)
libpcap: sanity_check
	ln -sf libpcap libpcap-1.1.1 
	(tar xkz -f libpcap.tar.gz 2> /dev/null || true)
	rm -f libpcap-1.1.1
	$(MAKE) -C libpcap libpcap.a
else
libpcap: sanity_check
	@echo "skipping libpcap (not configured)"
endif


