# Just set OC_BASE to the opencore root, or set AMR_BASE directly to
# a detached gsm_amr directory
OC_BASE = ../opencore
AMR_BASE = $(OC_BASE)/codecs_v2/audio/gsm_amr

# To compile as C instead of C++, define BUILD_AS_C
ifneq (, $(BUILD_AS_C))
    CXX = $(CC)
    CXXFLAGS += -x c -std=c99 -D__inline="static inline"
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

ifeq ($(shell uname), Darwin)
    SHLIB_EXT = dylib
    SHLIB_FLAGS = -dynamiclib
else
    SHLIB_EXT = so
    SHLIB_FLAGS = -shared
endif
SHLIB = libamrnb.$(SHLIB_EXT)

all: libamrnb.a $(SHLIB)

$(SHLIB): $(OBJS)
	$(CXX) $(SHLIB_FLAGS) -o $@ $+ $(LDFLAGS)

libamrnb.a: $(OBJS)
	ar rcs $@ $+

install: libamrnb.a $(SHLIB)
	install -d $(DESTDIR)$(PREFIX)/lib
	install libamrnb.a $(DESTDIR)$(PREFIX)/lib
	install $(SHLIB) $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include/amrnb
	install interf_dec.h $(DESTDIR)$(PREFIX)/include/amrnb
	install interf_enc.h $(DESTDIR)$(PREFIX)/include/amrnb

clean:
	rm -f $(SHLIB) libamrnb.a *.o $(OBJS)

