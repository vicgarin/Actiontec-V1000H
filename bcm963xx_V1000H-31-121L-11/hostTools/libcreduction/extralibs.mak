#
# Extra run-time libraries
#
# Note: this file is only used if LIBOPT=n
# 
# Copyright (C) 2004 Broadcom Corporation
#


TARGETS += $(LIBDIR)/libcrypt.so.0
TARGETS += $(LIBDIR)/libutil.so.0


# Any application which links with pthreads needs to be added here.
# Note: setting INCLUDE_PTHREAD is only important if libcreduction is not run.
# If libcreduction is run, libcreduction automatically detects whether 
# libpthread is used by any app and includes it in the build as needed.

INCLUDE_PTHREAD := n
INCLUDE_LIBM := n
INCLUDE_STDCPP := n

ifneq ($(strip $(BRCM_APP_PHONE)),)
  INCLUDE_PTHREAD := y
endif

ifneq ($(strip $(BUILD_SWMDK)),)
  INCLUDE_PTHREAD := y
endif

ifneq ($(strip $(BUILD_DLNA)),)
  INCLUDE_PTHREAD := y
  INCLUDE_LIBM := y
endif

ifneq ($(strip $(BUILD_MoCACTL2)),)
  INCLUDE_PTHREAD := y
  INCLUDE_LIBM := y
endif

ifneq ($(strip $(BUILD_VODSL)),)
  INCLUDE_LIBM := y
#  TARGETS += $(EXTRALIBDIR)/libstdc++.so.6
endif

ifneq ($(strip $(BUILD_OPROFILE)),)
  INCLUDE_STDCPP := y
  INCLUDE_LIBM := y
endif

ifneq ($(strip $(BUILD_OSGI_JVM)),)
  INCLUDE_PTHREAD := y
  INCLUDE_LIBM := y
endif

ifneq ($(strip $(BUILD_BMU)),)
  INCLUDE_STDCPP := y
  INCLUDE_LIBM := y
endif

ifeq ($(strip $(INCLUDE_PTHREAD)),y)
  TARGETS += $(LIBDIR)/libpthread.so.0
endif

ifeq ($(strip $(INCLUDE_LIBM)),y)
  TARGETS += $(LIBDIR)/libm.so.0
endif

ifeq ($(strip $(INCLUDE_STDCPP)),y)
  TARGETS += $(EXTRALIBDIR)/libstdc++.so.6
endif
