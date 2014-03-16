
all:
	(tar xkfz iperf-2.0.5.tar.gz 2> /dev/null || true)
	if [ ! -e ./iperf-2.0.5/Makefile ];then \
	cd ./iperf-2.0.5; ./configure --host=mips-linux-uclibc --target=mips-linux-uclibc ac_cv_func_malloc_0_nonnull=yes; \
	fi

	$(MAKE) -C ./iperf-2.0.5 srcdir=./ builddir=./
	install -m 755 ./iperf-2.0.5/src/iperf $(INSTALL_DIR)/bin/iperf

clean:
	if [ -f ./iperf-2.0.5/Makefile ];then $(MAKE) -C ./iperf-2.0.5 clean srcdir=./ builddir=./;fi;

