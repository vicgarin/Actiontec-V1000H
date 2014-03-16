#***********************************************************************
#
#  Copyright (c) 2009  Broadcom Corporation
#  All Rights Reserved
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
#***********************************************************************/

#
# You probably don't need to modify anything in this Makefile.
#



#
# This is the list of directories to traverse.  
# Note that when you type make from the top, it does not actually
# go through this Makefile.  See the Makefile one level above here.
# But when you type make clean from the top, it does go through this Makefile.
#
PUBLIC_DIRS = include libs apps


#
# We need to declare these directory names as phony targets,
# otherwise make will think the directories themselves are
# the targets and will never execute the cmds.
# See GNU Make, 4.6 
#
.PHONY : $(PUBLIC_DIRS)
all    : $(PUBLIC_DIRS)


#
# Generic rule for making the dirs
#
$(PUBLIC_DIRS):
	$(MAKE) -C $@


#
# Cleaning rules
# See example in GNU Make, 4.6
#
clean: 
	for dir in $(PUBLIC_DIRS); do \
		$(MAKE) -C $$dir clean; \
	done



