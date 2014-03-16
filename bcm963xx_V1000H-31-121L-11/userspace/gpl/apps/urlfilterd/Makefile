#
# build file 
#
#
CFLAGS = -s -Os -fomit-frame-pointer
CFLAGS += $(BRCM_WERROR_CFLAGS)
CFLAGS += $(ACTION_TEC_DFLAGS)
LDFLAGS = -Wl,-allow-shlib-undefined -L$(INSTALL_DIR)/lib/public -lcms_util -lcms_msg -L./libnfnetlink -L./libnetfilter_queue 
ifeq ($(strip $(BUILD_URLFILTER)), static)
CFLAGS += -DBUILD_STATIC 
endif

LIBS = -lnetfilter_queue -lnfnetlink
CFLAGS+=-Wall -I./libnetfilter_queue/include -I./libnfnetlink/include -I$(BUILD_DIR)/userspace/public/include -I$(BUILD_DIR)/userspace/public/include/linux

OBJS = filter.o 

all: gen-libs urlfilterd

install: urlfilterd
	install -m 755 urlfilterd $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/urlfilterd

dynamic: all install

static: urlfilterd.a

.c.o:
	$(CC) $(CFLAGS) -c $<

gen-libs: 
	$(MAKE) -C libnfnetlink
	$(MAKE) -C libnetfilter_queue

urlfilterd: filter.o aei_url_util.o gen-libs aei_wlan_redirect.o
	$(CC) -o urlfilterd filter.o aei_url_util.o aei_wlan_redirect.o $(LDFLAGS) $(LIBS)

urlfilterd.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

clean:
	$(MAKE) -C libnfnetlink clean
	$(MAKE) -C libnetfilter_queue clean
	-rm -f urlfilterd *.o  urlfilterd.a
