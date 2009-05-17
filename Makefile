all:
	$(MAKE) -C amrnb BUILD_AS_C=1
	$(MAKE) -C amrwb BUILD_AS_C=1

distclean: clean
clean:
	$(MAKE) -C amrnb clean
	$(MAKE) -C amrwb clean

install:
	$(MAKE) -C amrnb install
	$(MAKE) -C amrwb install
