# $(info within android.mk)

############################################################################
#
#  General rules to make and clean
#



############################################################################
############################################################################

# macro to define printf.  This macro needs to be able to handle
# backslash-escaped characters such as \t(tab), \n(newline),etc.
# "echo -e" handles this but certain shells such as dash (on ubuntu)
# don't handle the -e option in their builtin echo implementation.
# However, the printf builtin of these sh shells should work.
PRINTF:=printf

############################################################################
esc_dollar := \$$


############################################################################
# Original method using multiple echo statements separated by ';', but this 
# seemed much less efficient then putting it into one statement.  
# See second version below.
#define output_list
#$(foreach elem,$(call truncate,$1),$(PRINTF) "\t$(elem) \\" >> $2;) $(PRINTF) "\t$(lastword $1)\\n" >> $2
#endef



define output_list
  $(PRINTF) "$(foreach elem,$(call truncate,$1),\\t$(elem) \\\\\n) \\t$(lastword $1)\\n" >> $2
endef

define output_lib_list
  $(PRINTF) "$(foreach elem,$(call truncate,$1),\\tlib$(elem) \\\\\n) \\tlib$(lastword $1)\\n" >> $2
endef

define format_shared_lib_names
  $(subst -l,lib,$1)
endef

define convert_component_lib_makefile_name
  $(patsubst %,%/Android.mk,$(patsubst $(SRC_ROOT)/%,\$$(PV_TOP)/%,$1))
endef

define output_include_list
  $(PRINTF) "$(subst $(SPACE)include,include,$(foreach elem,$(call truncate,$(strip $1)),include $(call convert_component_lib_makefile_name,$(elem))\\n))include $(call convert_component_lib_makefile_name,$(lastword $(strip $1)))\\n" >> $2  
endef

define include_staticlibs_list
  $(if $(strip $(call remove_quotes,$1)),$(PRINTF) "$(foreach elem,$(strip $(call remove_quotes,$1)),include $(patsubst %,%/Android.mk,$(patsubst %,\$$(PV_TOP)%,$(call strip_two_levels_up,$(elem)/local.mk)))\n)" >> $2,)
endef

define output_assembly_srcs
  $(if $(strip $1),$(PRINTF) "ifeq (\$$(TARGET_ARCH),arm)\\nLOCAL_SRC_FILES += \\\\\n$(foreach elem,$(call truncate,$1),\\t$(elem) \\\\\n)\\t$(lastword $1)\\nendif\\n\\n" >> $2,)
endef

define extra_lib_list
  $(if $(strip $1),$(PRINTF) "\nLOCAL_WHOLE_STATIC_LIBRARIES += $1\n" >> $2,)
endef

define extra_include_list
  $(if $(strip $1),$(PRINTF) "$(foreach elem, $1,include $(patsubst %,%/Android.mk,$(patsubst %,\$$(PV_TOP)%,$(strip $(elem))))\n)" >> $2,)
endef

define is_prelinking_allowed
  $(if $(strip $1),,$(PRINTF) "\nLOCAL_PRELINK_MODULE := false\n" >> $2)
endef

############################################################################


include $(MK)/android_segments.mk

#############################################
#    Rules for aggregate makefiles
#
ifneq ($(AGGREGATE_LIBS_MAKEFILE),)

#### Start generation of aggregate makefiles #######
define create_aggregate_lib_android_mk
Android_$1.mk: FORCE
	$$(quiet) echo "LOCAL_PATH := $$(esc_dollar)(call my-dir)" > $$@
	$$(quiet) echo "include $$(esc_dollar)(CLEAR_VARS)" >> $$@
	$$(quiet) echo "" >> $$@
	$$(quiet) echo "LOCAL_WHOLE_STATIC_LIBRARIES := \\" >> $$@
	$$(quiet) $$(call output_lib_list,$$($1_CUMULATIVE_TARGET_LIST),$$@)
	$$(quiet) $$(call extra_lib_list, $$(EXTRA_LIBS_$1),$$@)
	$$(quiet) echo "" >> $$@
	$$(quiet) echo "LOCAL_MODULE := lib$1" >> $$@
	$$(quiet) $$(call is_prelinking_allowed,$$($1_PRELINK),$$@)
	$$(quiet) echo "" >> $$@
	$$(quiet) echo "-include $$(esc_dollar)(PV_TOP)/Android_platform_extras.mk" >> $$@
	$$(quiet) echo "" >> $$@
	$$(quiet) echo "-include $$(esc_dollar)(PV_TOP)/Android_system_extras.mk" >> $$@
	$$(quiet) echo "" >> $$@
	$$(quiet) echo "LOCAL_SHARED_LIBRARIES += $$(call format_shared_lib_names,$$(MODS_$1))" >> $$@
	$$(quiet) echo "" >> $$@
	$$(quiet) echo "include $$(esc_dollar)(BUILD_SHARED_LIBRARY)" >> $$@
	$$(quiet) $$(call output_include_list,$$($1_CUMULATIVE_MAKEFILES),$$@)
	$$(quiet) $$(call extra_include_list, $$(EXTRA_MAKEFILES_PATHS_$1),$$@)
	$$(quiet) echo "" >> $$@
endef

#### End generation of aggregate makefiles #######



#### Start generation of top level makefile #######

define include_module_mk_list
 $(PRINTF) "$(subst $(SPACE)include,include,$(foreach elem,$1,include \$$(PV_TOP)/build_config/opencore_dynamic/Android_$(elem).mk\n))" >> $2
endef

define include_test_mk_list
 $(PRINTF) "$(subst $(SPACE)include,include,$(foreach app,$(strip $(call remove_quotes,$(TESTAPPS))),include \$$(PV_TOP)$(strip $(call strip_two_levels_up,$(call remove_quotes,$(TESTAPP_DIR_$(app))/local.mk)))/Android.mk\n))" >> $1
endef

define create_toplevel_android_mk
$1: FORCE
	$$(quiet) echo "ifneq ($$(esc_dollar)(BUILD_WITHOUT_PV),true)" > $$@
	$$(quiet) echo "LOCAL_PATH := $$(esc_dollar)(call my-dir)" >> $$@
	$$(quiet) echo "PV_TOP := $$(esc_dollar)(LOCAL_PATH)" >> $$@
	$$(quiet) echo "include $$(esc_dollar)(CLEAR_VARS)" >> $$@
	$$(quiet) echo "" >> $$@
	$$(quiet) echo "PV_CFLAGS := -Wno-non-virtual-dtor -DENABLE_MEMORY_PLAYBACK -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DUSE_CML2_CONFIG" >> $$@
	$$(quiet) echo "" >> $$@
	$$(quiet) echo "ifeq ($$(esc_dollar)(ENABLE_PV_LOGGING),1)" >> $$@
	$$(quiet) echo " PV_CFLAGS += -DPVLOGGER_INST_LEVEL=5" >> $$@
	$$(quiet) echo "endif"  >> $$@
	$$(quiet) echo "" >> $$@
	$$(quiet) echo "ifeq ($$(esc_dollar)(TARGET_ARCH),arm)" >> $$@
	$$(quiet) echo "  PV_CFLAGS += -DPV_ARM_GCC_V5" >> $$@
	$$(quiet) echo "endif"  >> $$@
	$$(quiet) echo "" >> $$@
	$$(quiet) echo "include $$(esc_dollar)(CLEAR_VARS)" >> $$@
	$$(quiet) echo "" >> $$@
	$$(quiet) echo "FORMAT := android" >> $$@
	$$(quiet) echo "" >> $$@
	$$(quiet) echo "PV_COPY_HEADERS_TO := libpv" >> $$@
	$$(quiet) echo "" >> $$@
	$$(quiet) echo "PV_INCLUDES := \\" >> $$@
	$$(quiet) $(PRINTF) "\t$$(esc_dollar)(PV_TOP)/android \\\\\n" >> $$@
	$$(quiet) $(PRINTF) "\t$$(esc_dollar)(PV_TOP)/extern_libs_v2/khronos/openmax/include \\\\\n" >> $$@
	$$(quiet) $(PRINTF) "\t$$(esc_dollar)(PV_TOP)/engines/common/include \\\\\n" >> $$@
	$$(quiet) $(PRINTF) "\t$$(esc_dollar)(PV_TOP)/engines/player/config/android \\\\\n" >> $$@
	$$(quiet) $(PRINTF) "\t$$(esc_dollar)(PV_TOP)/engines/player/include \\\\\n" >> $$@
	$$(quiet) $(PRINTF) "\t$$(esc_dollar)(PV_TOP)/nodes/pvmediaoutputnode/include \\\\\n" >> $$@
	$$(quiet) $(PRINTF) "\t$$(esc_dollar)(PV_TOP)/nodes/pvdownloadmanagernode/config/opencore \\\\\n" >> $$@
	$$(quiet) $(PRINTF) "\t$$(esc_dollar)(PV_TOP)/pvmi/pvmf/include \\\\\n" >> $$@
	$$(quiet) $(PRINTF) "\t$$(esc_dollar)(PV_TOP)/fileformats/mp4/parser/config/opencore \\\\\n" >> $$@
	$$(quiet) $(PRINTF) "\t$$(esc_dollar)(PV_TOP)/oscl/oscl/config/android \\\\\n" >> $$@
	$$(quiet) $(PRINTF) "\t$$(esc_dollar)(PV_TOP)/oscl/oscl/config/shared \\\\\n" >> $$@
	$$(quiet) $(PRINTF) "\t$$(esc_dollar)(PV_TOP)/engines/author/include \\\\\n" >> $$@
	$$(quiet) $(PRINTF) "\t$$(esc_dollar)(PV_TOP)/android/drm/oma1/src \\\\\n" >> $$@
	$$(quiet) $(PRINTF) "\t$$(esc_dollar)(PV_TOP)/build_config/opencore_dynamic \\\\\n" >> $$@
	$$(quiet) $(PRINTF) "\t$$(esc_dollar)(TARGET_OUT_HEADERS)/$$(esc_dollar)(PV_COPY_HEADERS_TO)" >> $$@
	$$(quiet) echo "" >> $$@
	$$(quiet) echo "" >> $$@
	$$(quiet) echo "$$(esc_dollar)(call add-prebuilt-files, ETC, pvplayer.cfg)" >> $$@
	$$(quiet) echo "" >> $$@
	$$(quiet) $$(call include_module_mk_list,$2,$$@)
	$$(quiet) $$(call include_staticlibs_list,$$(LIBDIR_static),$$@)
	$$(quiet) $$(call include_test_mk_list,$$@)
	$$(quiet) echo "endif" >> $$@
endef
#### End generation of top level makefile #######


# loop over all the names in SHARED_LIB_TARGET_LIST and 
#  create the Android makefile name list.
#  Append top-level Android.mk
ANDROID_TOPLEVEL_MAKE_NAME := Android.mk
ANDROID_MAKE_NAMES := $(patsubst %,Android_%.mk,$(SHARED_LIB_TARGET_LIST)) $(ANDROID_TOPLEVEL_MAKE_NAME)

$(strip $(foreach lib,$(SHARED_LIB_TARGET_LIST),$(eval $(call create_aggregate_lib_android_mk,$(lib)))))

$(eval $(call create_toplevel_android_mk,$(ANDROID_TOPLEVEL_MAKE_NAME),$(SHARED_LIB_TARGET_LIST)))

android_clean: modulelevel_android_mk_clean toplevel_android_mk_clean

modulelevel_android_mk_clean: ANDROID_MAKE_FILES_TO_CLEAN := $(ANDROID_MAKE_NAMES) 
toplevel_android_mk_clean: ANDROID_MAKE_FILES_TO_CLEAN += $(ANDROID_TOPLEVEL_MAKE_NAME)

modulelevel_android_mk_clean: FORCE
	$(quiet) $(RM) $(ANDROID_MAKE_FILES_TO_CLEAN)

toplevel_android_mk_clean: FORCE
	$(quiet) $(RM) $(ANDROID_TOPLEVEL_MAKE_NAME)

#############################################
#    Rules for a single library makefile
#
else

ifeq ($(LOCAL_ANDROID_MK_PATH),)
  LOCAL_ANDROID_MK_PATH := $(patsubst /%,%,$(call strip_two_levels_up,$(subst $(SRC_ROOT),$$(SRC_ROOT),$(strip $(LOCAL_PATH)))/local.mk))
endif

CUMULATIVE_ANDROID_MK_PATH := $(CUMULATIVE_ANDROID_MK_PATH) $(LOCAL_ANDROID_MK_PATH)

define include_system_extras
  $(if $(strip $(filter $1,BUILD_EXECUTABLE)),$(PRINTF) "\n-include \$$(PV_TOP)/Android_system_extras.mk\n" >> $2,)
endef

#$(eval ANDROID_MAKE_NAMES := $(LOCAL_ANDROID_MK_PATH)/Android.mk)
ANDROID_MAKE_NAMES := $(LOCAL_PATH)/../../Android.mk

ANDROID_TMP_LOCAL_INC := $(subst $(SRC_ROOT),\$$(PV_TOP),$(LOCAL_TOTAL_INCDIRS)) \$$(PV_INCLUDES)

ANDROID_PATH_COMPONENTS := $(subst /, ,$(LOCAL_PATH))

AND_LOCAL_ARM_MODE := $(if $(strip $(filter codecs_v2,$(ANDROID_PATH_COMPONENTS))),LOCAL_ARM_MODE := arm,)

$(ANDROID_MAKE_NAMES): ANDROID_CPP_SRCS := $(if $(strip $(SRCS)),$(patsubst %,$(call go_up_two_levels,$(SRCDIR))/%,$(filter %.cpp,$(SRCS))),)
$(ANDROID_MAKE_NAMES): ANDROID_ASM_SRCS := $(if $(strip $(SRCS)),$(patsubst %,$(call go_up_two_levels,$(SRCDIR))/%,$(filter-out %.cpp,$(SRCS))),)
$(ANDROID_MAKE_NAMES): ANDROID_TARGET := $(if $(strip $(filter prog,$(TARGET_TYPE))),"LOCAL_MODULE :=" $(TARGET),$(if $(strip $(TARGET)),"LOCAL_MODULE :=" lib$(TARGET),))
$(ANDROID_MAKE_NAMES): ANDROID_HDRS := $(patsubst %,$(call go_up_two_levels,$(INCSRCDIR))/%,$(HDRS))
$(ANDROID_MAKE_NAMES): ANDROID_C_FLAGS := $(filter-out %PV_ARM_GCC_V5,$(XCPPFLAGS))
$(ANDROID_MAKE_NAMES): ANDROID_C_INC := $(ANDROID_TMP_LOCAL_INC)
$(ANDROID_MAKE_NAMES): ANDROID_ARM_MODE := $(AND_LOCAL_ARM_MODE)
$(ANDROID_MAKE_NAMES): ANDROID_MAKE_TYPE := $(if $(strip $(filter prog,$(TARGET_TYPE))),BUILD_EXECUTABLE,$(if $(strip $(SRCS)),BUILD_STATIC_LIBRARY,BUILD_COPY_HEADERS))
$(ANDROID_MAKE_NAMES): ANDROID_STATIC_LIBS := $(foreach library,$(LIBS),$(if $(findstring $(strip $(BUILD_ROOT)/installed_lib/$(BUILD_ARCH)/lib$(library)$(TARGET_NAME_SUFFIX).a), $(ALL_LIBS)),lib$(library),))
$(ANDROID_MAKE_NAMES): ANDROID_SHARED_LIBS := $(foreach library,$(LIBS),$(if $(findstring $(strip $(BUILD_ROOT)/installed_lib/$(BUILD_ARCH)/lib$(library)$(TARGET_NAME_SUFFIX).so), $(SHARED_LIB_FULLNAMES)),lib$(library),))

$(ANDROID_MAKE_NAMES): FORCE
	$(quiet) echo "LOCAL_PATH := \$$(call my-dir)" > $@
	$(quiet) echo "include \$$(CLEAR_VARS)" >> $@
	$(quiet) echo "" >> $@
	$(quiet) echo "LOCAL_SRC_FILES := \\" >> $@
	$(quiet) $(call output_list,$(ANDROID_CPP_SRCS),$@)
	$(quiet) echo "" >> $@
	$(quiet) $(call output_assembly_srcs,$(ANDROID_ASM_SRCS),$@)
	$(quiet) echo "" >> $@
	$(quiet) echo "$(ANDROID_TARGET)" >> $@
	$(quiet) echo "" >> $@
	$(quiet) echo "LOCAL_CFLAGS := $(ANDROID_C_FLAGS) \$$(PV_CFLAGS)" >> $@
	$(quiet) echo "" >> $@
	$(quiet) echo "$(ANDROID_ARM_MODE)" >> $@
	$(quiet) echo "" >> $@
	$(quiet) echo "LOCAL_STATIC_LIBRARIES := $(ANDROID_STATIC_LIBS)" >> $@
	$(quiet) echo "" >> $@
	$(quiet) echo "LOCAL_SHARED_LIBRARIES := $(ANDROID_SHARED_LIBS)" >> $@
	$(quiet) echo "" >> $@
	$(quiet) echo "LOCAL_C_INCLUDES := \\" >> $@
	$(quiet) $(call output_list,$(ANDROID_C_INC),$@)
	$(quiet) echo "" >> $@
	$(quiet) echo "LOCAL_COPY_HEADERS_TO := \$$(PV_COPY_HEADERS_TO)" >> $@
	$(quiet) echo "" >> $@
	$(quiet) echo "LOCAL_COPY_HEADERS := \\" >> $@
	$(quiet) $(call output_list,$(ANDROID_HDRS),$@)
	$(quiet) $(call include_system_extras,$(ANDROID_MAKE_TYPE),$@)
	$(quiet) echo "" >> $@
	$(quiet) echo "include \$$($(ANDROID_MAKE_TYPE))" >> $@

android_clean: $(LOCAL_PATH)_android_mk_clean

$(LOCAL_PATH)_android_mk_clean: ANDROID_MAKE_FILES_TO_CLEAN := $(ANDROID_MAKE_NAMES)

$(LOCAL_PATH)_android_mk_clean: FORCE
	$(quiet) $(RM) $(ANDROID_MAKE_FILES_TO_CLEAN)


endif


android_make: $(ANDROID_MAKE_NAMES)
CMD_COMPLETION_TARGETS += android_make android_clean

