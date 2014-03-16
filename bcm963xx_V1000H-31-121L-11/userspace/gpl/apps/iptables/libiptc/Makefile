#! /usr/bin/make

EXTRAS+=libiptc/libiptc.a


EXTRA_DEPENDS+=libiptc/libip4tc.d
LIBIPTC_OBJS := libiptc/libip4tc.o

# This original way of specifying the libiptc.a library elements sometimes
# fails during mulit-threaded makes.  One thread creates the lib for 
# libip4tc.o and another thread creates another copy of the lib for libip6tc.o
#libiptc/libiptc.a: libiptc/libiptc.a(libiptc/libip4tc.o)


ifeq ($(DO_IPV6), 1)
EXTRA_DEPENDS+= libiptc/libip6tc.d
LIBIPTC_OBJS += libiptc/libip6tc.o
#libiptc/libiptc.a: libiptc/libiptc.a(libiptc/libip6tc.o)
endif


# this is the multi-thread safe way of creating the archive
libiptc/libiptc.a: $(LIBIPTC_OBJS)
	rm -f $@; $(AR) crv $@ $(LIBIPTC_OBJS)


libiptc/libip4tc.d libiptc/libip6tc.d: %.d: %.c
	@-$(CC) -M -MG $(CFLAGS) $< | sed -e 's@^.*\.o:@$*.d libiptc/libiptc.a:@' > $@
