#***********************************************************************
#
#  Copyright (c) 2006-2010  Broadcom Corporation
#  All Rights Reserved
#
#  <:license-gpl
#
#***********************************************************************/

#
# You probably don't need to modify anything in this Makefile.
#



#
# This is the list of directories to traverse.  
#
GPL_DIRS = apps libs


#
# We need to declare these directory names as phony targets,
# otherwise make will think the directories themselves are
# the targets and will never execute the cmds.
# See GNU Make, 4.6 
#
.PHONY : $(GPL_DIRS)
all    : $(GPL_DIRS)


#
# Generic rule for making the dirs
#
$(GPL_DIRS):
	$(MAKE) -C $@


#
# Cleaning rules
# See example in GNU Make, 4.6
#
clean: 
	for dir in $(GPL_DIRS); do \
		$(MAKE) -C $$dir clean; \
	done



