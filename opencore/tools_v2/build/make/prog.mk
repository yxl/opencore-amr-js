# -*- makefile -*-
#
# This makefile template should be included by makefiles in
# program directories.
#

# Set the directory for the local sources
LOCAL_SRCDIR :=  $(abspath $(LOCAL_PATH)/$(SRCDIR))
LOCAL_INCSRCDIR :=  $(abspath $(LOCAL_PATH)/$(INCSRCDIR))

OBJDIR := $(patsubst $(SRC_ROOT)/%,$(BUILD_ROOT)/%,$(abspath $(LOCAL_PATH)/$(OUTPUT_DIR_COMPONENT)))

$(eval $(call set-src-and-obj-names,$(SRCS),$(LOCAL_SRCDIR)))

ifneq ($(strip $(FORCED_OBJS)),)
 # The point of this dependency is to force object rebuilds when the 
 # corresponding dependency files are missing (even if the object file exists).
 $(FORCED_OBJS): FORCE
endif


ifneq "$(MAKECMDGOALS)" "clean"
  ifneq ($(strip $(FOUND_DEPS)),)
# $(warning Including $(FOUND_DEPS))
  -include $(FOUND_DEPS)
  endif
endif

ifeq ($(strip $(DEFAULT_LIBMODE)),release)
  XCXXFLAGS+=$(OPT_CXXFLAG)
  XCXXFLAGS+=$(RELEASE_CXXFLAGS)
  XCPPFLAGS+=$(RELEASE_CPPFLAGS)
else
  XCPPFLAGS+=$(DEBUG_CPPFLAGS)
  XCXXFLAGS+=$(DEBUG_CXXFLAGS)
endif

LOCAL_XINCDIRS := $(abspath $(patsubst ../%,$(LOCAL_PATH)/../%,$(patsubst -I%,%,$(XINCDIRS))))

LOCAL_TOTAL_INCDIRS := $(LOCAL_SRCDIR) $(LOCAL_INCSRCDIR) $(LOCAL_XINCDIRS)

$(COMPILED_OBJS): XFLAGS := $(XCPPFLAGS) $(patsubst %,-I%,$(LOCAL_TOTAL_INCDIRS)) $(XCXXFLAGS)

# remove any leading / trailing whitespace
TARGET := $(strip $(TARGET))

# save compiled objects in a macro
$(TARGET)_compiled_objs := $(COMPILED_OBJS)

ifneq ($(strip $(REMOTE_DIRS)),)
# $(info remote dirs = $(REMOTE_DIRS))
$(foreach srcdir, $(strip $(REMOTE_DIRS)), $(eval $(call OBJ_TEMPLATE,$(srcdir),$(OBJDIR))))
endif


$(OBJDIR)/%.$(OBJ_EXT): $(LOCAL_SRCDIR)/%.cpp 
	$(call make-cpp-obj-and-depend,$<,$@,$(subst .$(OBJ_EXT),.d,$@),$(XFLAGS))

$(OBJDIR)/%.$(OBJ_EXT): $(LOCAL_SRCDIR)/%.c
	$(call make-c-obj-and-depend,$<,$@,$(subst .$(OBJ_EXT),.d,$@),$(XFLAGS))


#ifeq ($(HOST_ARCH), win32)
#  vpath %.so $(LIB_DIRS:$(LIBCOMPFLAG)%=%)
#  vpath %.$(STAT_LIB_EXT) $(LIB_DIRS:$(LIBCOMPFLAG)%=%)
#else
#  vpath lib%.so $(LIB_DIRS:$(LIBCOMPFLAG)%=%)
#  vpath lib%.$(STAT_LIB_EXT) $(LIB_DIRS:$(LIBCOMPFLAG)%=%)
#endif

LOCAL_LIBDIRS := $(abspath $(patsubst ../%,$(LOCAL_PATH)/../%,$(patsubst $(LIBCOMPFLAG)%,%,$(XLIBDIRS))))

LOCAL_LIBDIRS := $(patsubst %,$(LIBCOMPFLAG)%,$(LOCAL_LIBDIRS)) $(LIB_DIRS)


REALTARGET := $(TARGET:%=$(BUILD_ROOT)/bin/$(OUTPUT_DIR_COMPONENT)/%$(TARGET_EXT))

TMPDEPS := $(patsubst %,$$(%_fullname),$(LIBS))

$(eval $(TARGET)_LIBDEPS := $(TMPDEPS))

$(TARGET)_LDFLAGS := $(LOCAL_LIBDIRS) $($(TARGET)_LIBDEPS) $(LDFLAGS) $(XLDFLAGS)

$(REALTARGET): $(COMPILED_OBJS) $($(TARGET)_LIBDEPS)
	@echo Building $@
	$(call create_objdir,$(@D))
	$(call generate_prog,$@,$(notdir $@))
	@echo DONE building $@.

ALL_BIN_INSTALLED: $(REALTARGET)

# Pseudo-targets for executables. With this, we can use "make $(TARGET)" instead of "make $(BUILD_ROOT)/bin/$(OUTPUT_DIR_COMPONENT)/%$(TARGET_EXT)"
# # E.g., make pvplayer_engine_test
$(TARGET): $(REALTARGET)

.PRECIOUS:: $(DEPS) $(COMPILED_OBJS)

TARGET_TYPE := prog

-include $(PLATFORM_EXTRAS)

TARGET_LIST := $(TARGET_LIST) $(TARGET)

run_$(TARGET)_TEST_ARGS := $(TEST_ARGS)
run_$(TARGET)_SOURCE_ARGS := $(SOURCE_ARGS)
run_$(TARGET)_SOURCE_DIR := $(LOCAL_PATH)

###incluede targest for test apps###########
run_$(TARGET): $(REALTARGET)
		$(call cd_and_run_test,$($@_SOURCE_DIR),$<,$($@_TEST_ARGS),$($@_SOURCE_ARGS))
	
run_test: run_$(TARGET)
build_$(TARGET): $(REALTARGET)
build_test: build_$(TARGET)
