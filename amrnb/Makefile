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

DEC_DIR = $(AMR_BASE)/amr_nb/dec
ENC_DIR = $(AMR_BASE)/amr_nb/enc
COMMON_DIR = $(AMR_BASE)/amr_nb/common
DEC_SRC_DIR = $(DEC_DIR)/src
ENC_SRC_DIR = $(ENC_DIR)/src
COMMON_SRC_DIR = $(COMMON_DIR)/src
OSCL = ../oscl

CPPFLAGS = -I$(OSCL) -I$(DEC_SRC_DIR) -I$(COMMON_DIR)/include -I$(DEC_DIR)/include -I$(AMR_BASE)/common/dec/include -I$(ENC_SRC_DIR)

# Find all the source files
DEC_SRC := $(shell cd $(DEC_SRC_DIR) && echo *.cpp)
ENC_SRC := $(shell cd $(ENC_SRC_DIR) && echo *.cpp)
COMMON_SRC := $(shell cd $(COMMON_SRC_DIR) && echo *.cpp)

# Exclude these files
DEC_SRC := $(DEC_SRC:decoder_gsm_amr.cpp=)
DEC_SRC := $(DEC_SRC:pvgsmamrdecoder.cpp=)
ENC_SRC := $(ENC_SRC:gsmamr_encoder_wrapper.cpp=)

DEC_OBJS := $(DEC_SRC:.cpp=.o)
DEC_OBJS := $(patsubst %,$(DEC_SRC_DIR)/%, $(DEC_OBJS))
ENC_OBJS := $(ENC_SRC:.cpp=.o)
ENC_OBJS := $(patsubst %,$(ENC_SRC_DIR)/%, $(ENC_OBJS))
COMMON_OBJS := $(COMMON_SRC:.cpp=.o)
COMMON_OBJS := $(patsubst %,$(COMMON_SRC_DIR)/%, $(COMMON_OBJS))

OBJS = wrapper.o $(DEC_OBJS) $(ENC_OBJS) $(COMMON_OBJS)

#Versioning
MAJOR = 0
MINOR = 1
REVISION = 0

ifeq ($(shell uname), Darwin)
    SHLIB_EXT = dylib
    SHLIB_FLAGS = -dynamiclib
else
    SHLIB_EXT = so.$(MAJOR).$(MINOR).$(REVISION)
    SHLIB_FLAGS = -shared
    SONAME = libopencore-amrnb.so.$(MAJOR)
endif
SHLIB = libopencore-amrnb.$(SHLIB_EXT)

all: libopencore-amrnb.a $(SHLIB)

$(SHLIB): $(OBJS)
ifeq ($(shell uname), Darwin)
	$(CXX) $(SHLIB_FLAGS) -o $@ $+ $(LDFLAGS)
else
	$(CXX) $(SHLIB_FLAGS) -o $@ $+ -Wl,-soname,$(SONAME) $(LDFLAGS)
endif

libopencore-amrnb.a: $(OBJS)
	ar rcs $@ $+

install: libopencore-amrnb.a $(SHLIB)
	install -d $(DESTDIR)$(PREFIX)/lib
	install -m 644 libopencore-amrnb.a $(DESTDIR)$(PREFIX)/lib
	install $(SHLIB) $(DESTDIR)$(PREFIX)/lib
ifneq ($(shell uname), Darwin)
	ln -s $(SHLIB) $(DESTDIR)$(PREFIX)/lib/$(SONAME)
	ln -s $(SONAME) $(DESTDIR)$(PREFIX)/lib/libopencore-amrnb.so
endif
	install -d $(DESTDIR)$(PREFIX)/include/opencore-amrnb
	install -m 644 interf_dec.h $(DESTDIR)$(PREFIX)/include/opencore-amrnb
	install -m 644 interf_enc.h $(DESTDIR)$(PREFIX)/include/opencore-amrnb

clean:
	rm -f $(SHLIB) libopencore-amrnb.a *.o $(OBJS)

