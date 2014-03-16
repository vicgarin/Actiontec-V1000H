
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
EXE=sendpackets

include $(BUILD_DIR)/make.common

sendpackets: sendpackets.c
	$(CC) -L ../../libs/libpcap -I../../libs/libpcap  sendpackets.c -lpcap  -o sendpackets



clean:
	rm -f sendpackets *.o

install: sendpackets generic_exe_install


