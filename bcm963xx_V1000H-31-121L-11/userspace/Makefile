#***********************************************************************
#
#  Copyright (c) 2007  Broadcom Corporation
#  All Rights Reserved
#  No portions of this material may be reproduced in any form without the
#  written permission of:
#          Broadcom Corporation
#          5300 California Avenue
#          Irvine, California 92617
#  All information contained in this document is Broadcom Corporation
#  company private, proprietary, and trade secret.
#
#***********************************************************************/

#
# You probably don't need to modify anything in this Makefile.
#



#
# This is the list of directories.  But we cannot do a simple
# traversal into each directory because we need to build all libraries
# before building any of the apps.  See the "all" target.
#
USERSPACE_DIRS = public private gpl



#
# We need to declare these directory names as phony targets,
# otherwise make will think the directories themselves are
# the targets and will never execute the cmds.
# See GNU Make, 4.6 
#
.PHONY : $(USERSPACE_DIRS)

ifneq ($(strip $(AEI_CONTROL_LAYER)),)

all: ctl_layer private-apps

ctl_layer: private-libs public-libs
ifneq ($(strip $(AEI_VDSL_CUSTOMER_BELLALIANT)),)
else
endif

else

all: private-apps

endif

public-include:
	$(MAKE) -C public/include

private-libs: public-include

public-libs: public-include
	$(MAKE) -j $(ACTUAL_MAX_JOBS) -C public/libs

gpl-libs: public-include
	$(MAKE) -j $(ACTUAL_MAX_JOBS) -C gpl/libs

gpl-apps: private-libs public-libs gpl-libs
	$(MAKE) -j $(ACTUAL_MAX_JOBS) -C gpl/apps

public-apps: private-libs public-libs gpl-libs
	$(MAKE) -j $(ACTUAL_MAX_JOBS) -C public/apps

private-apps: gpl-apps public-apps


#
# Generic rule for making the dirs
# You can still make an entire directory at once, but be careful
# of the dependencies, you need to make all the libs before you
# can make any of the apps.
#
$(USERSPACE_DIRS):
	$(MAKE) -C $@



#
# Cleaning rules
# See example in GNU Make, 4.6
#
clean: 
	for dir in $(USERSPACE_DIRS); do \
		$(MAKE) -C $$dir clean; \
	done
