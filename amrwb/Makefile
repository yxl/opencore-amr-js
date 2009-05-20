# Just set OC_BASE to the opencore root, or set AMR_BASE directly to
# a detached gsm_amr directory
OC_BASE = ../opencore
AMR_BASE = $(OC_BASE)/codecs_v2/audio/gsm_amr

# To compile as C instead of C++, define BUILD_AS_C
ifneq (, $(BUILD_AS_C))
    CXX = $(CC)
    CXXFLAGS += -x c -std=c99
endif

ifeq (, $(PREFIX))
    PREFIX = /usr/local
endif

DEC_DIR = $(AMR_BASE)/amr_wb/dec
DEC_SRC_DIR = $(DEC_DIR)/src
OSCL = ../oscl

CPPFLAGS = -I$(OSCL) -I$(DEC_SRC_DIR) -I$(DEC_DIR)/include -I$(AMR_BASE)/common/dec/include

# Find all the source files
# Exclude only decoder_amr_wb.cpp, not dtx_decoder_amr_wb.cpp
DEC_SRC := $(shell cd $(DEC_SRC_DIR) && echo *.cpp | sed 's/ decoder_amr_wb.cpp//')

DEC_OBJS := $(DEC_SRC:.cpp=.o)
DEC_OBJS := $(patsubst %,$(DEC_SRC_DIR)/%, $(DEC_OBJS))

OBJS = wrapper.o $(DEC_OBJS)

#Versioning
MAJOR = 0
MINOR = 1
REVISION = 1

ifeq ($(shell uname), Darwin)
    SHLIB_EXT = dylib
    SHLIB_FLAGS = -dynamiclib
else
    SHLIB_EXT = so.$(MAJOR).$(MINOR).$(REVISION)
    SHLIB_FLAGS = -shared
    SONAME = libopencore-amrwb.so.$(MAJOR)
endif
SHLIB = libopencore-amrwb.$(SHLIB_EXT)

all: libopencore-amrwb.a $(SHLIB)

$(SHLIB): $(OBJS)
ifeq ($(shell uname), Darwin)
	$(CXX) $(SHLIB_FLAGS) -o $@ $+ $(LDFLAGS)
else
	$(CXX) $(SHLIB_FLAGS) -o $@ $+ -Wl,-soname,$(SONAME) $(LDFLAGS)
endif

libopencore-amrwb.a: $(OBJS)
	ar rcs $@ $+

install: libopencore-amrwb.a $(SHLIB)
	install -d $(DESTDIR)$(PREFIX)/lib
	install -m 644 libopencore-amrwb.a $(DESTDIR)$(PREFIX)/lib
	install $(SHLIB) $(DESTDIR)$(PREFIX)/lib
ifneq ($(shell uname), Darwin)
	ln -sf $(SHLIB) $(DESTDIR)$(PREFIX)/lib/$(SONAME)
	ln -sf $(SONAME) $(DESTDIR)$(PREFIX)/lib/libopencore-amrwb.so
endif
	install -d $(DESTDIR)$(PREFIX)/include/opencore-amrwb
	install -m 644 dec_if.h $(DESTDIR)$(PREFIX)/include/opencore-amrwb
	install -m 644 if_rom.h $(DESTDIR)$(PREFIX)/include/opencore-amrwb

clean:
	rm -f $(SHLIB) libopencore-amrwb.a *.o $(OBJS)

