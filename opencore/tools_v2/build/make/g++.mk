
CXX ?= g++
CC ?= gcc
LINK ?= $(CXX)
SHARED_LINK ?= $(CXX)
AR ?= ar
STRIP ?= strip

# The following macro will turn on combined
# compilation and dependency generation
COMBINED_COMPILE_AND_DEPEND:=1

override SYSLIBS = -lc -lm -ldl -lstdc++ -lpthread

SHARED_CFLAGS ?= -fPIC
SHARED_CXXFLAGS ?= -fPIC
SHARED_PRE_LDFLAGS ?= -shared -Wl,-Bsymbolic -Wl,--allow-multiple-definition -Wl,--whole-archive
SHARED_POST_LDFLAGS ?= -Wl,-no-whole-archive -Wl,--no-undefined $(SYSLIBS)
SONAME_ARG := -Wl,-h,

STRIP_FLAGS := --strip-unneeded
AR_ARGS := rl # make sure to leave a space at the end


OUTLINKFLAG := -o 
CO := -c -o # make sure to leave a space at the end


#Make all warnings into errors.
FLAG_COMPILE_WARNINGS_AS_ERRORS := -Werror

STAT_LIB_EXT:=a
SHARED_LIB_EXT:=so
SHARED_ARCHIVE_LIB_EXT:=sa
OBJ_EXT := o
LIBCOMPFLAG:=-L
DEBUG_CXXFLAGS?=-g
RELEASE_CPPFLAGS?=-DNDEBUG
OPT_CXXFLAG?=-O3
INCDIRS += -I$(BUILD_ROOT)/installed_include
CXXFLAGS?=-Wall -Wno-non-virtual-dtor

#########################################################
# $(call make-depend,source-file,object-file,depend-file,xflags)
define make-depend
  $(quiet) $(CXX) $4 $(CPPFLAGS) $(INCDIRS) $(CXXFLAGS) -MM $1 |        \
   $(SED) -e 's,\($(notdir $2)\) *:,$2: ,' -e 's,$(BUILD_ROOT),$$(BUILD_ROOT),'  -e 's,$(SRC_ROOT),$$(SRC_ROOT),' > $3.tmp
  $(quiet) $(CP) $3.tmp $3
  $(quiet) $(SED) -e 's/#.*//'  \
	-e 's/^[^:]*: *//'      \
	-e 's/ *\\$$//'		\
	-e '/^$$/ d'		\
	-e 's/$$/ :/'  $3.tmp >> $3
  $(quiet) $(RM) $3.tmp
endef
#########################################################


ifneq ($(strip $(COMBINED_COMPILE_AND_DEPEND)),)
#########################################################
# $(call combined-cxx-compile-depend,source-file,object-file,depend-file,xflags)
define combined-cxx-compile-depend
  $(quiet) $(CXX) $4 $(CPPFLAGS) $(INCDIRS) $(CXXFLAGS) -MMD $(CO)$2 $1
  $(quiet) $(SED) -e '/^ *\\ *$$/ d' -e 's,$(BUILD_ROOT),$$(BUILD_ROOT),'  -e 's,$(SRC_ROOT),$$(SRC_ROOT),' $3 > $3.tmp
  $(quiet) $(CP) $3.tmp $3
  $(quiet) $(SED) -e 's/#.*//'  \
	-e 's/^[^:]*: *//'      \
	-e 's/ *\\$$//'		\
	-e '/^$$/ d'		\
	-e 's/$$/ :/'  $3.tmp >> $3
  $(quiet) $(RM) $3.tmp
endef
#########################################################

# $(call combined-cc-compile-depend,source-file,object-file,depend-file,xflags)
define combined-cc-compile-depend
  $(quiet) $(CC) $4 $(CPPFLAGS) $(INCDIRS) $(CXXFLAGS) -MMD $(CO)$2 $1
  $(quiet) $(SED) -e '/^ *\\ *$$/ d' -e 's,$(BUILD_ROOT),$$(BUILD_ROOT),'  -e 's,$(SRC_ROOT),$$(SRC_ROOT),' $3 > $3.tmp
  $(quiet) $(CP) $3.tmp $3
  $(quiet) $(SED) -e 's/#.*//'  \
	-e 's/^[^:]*: *//'      \
	-e 's/ *\\$$//'		\
	-e '/^$$/ d'		\
	-e 's/$$/ :/'  $3.tmp >> $3
  $(quiet) $(RM) $3.tmp
endef
#########################################################

define assembly-compile
  $(quiet) $(CXX) $4 $(CPPFLAGS) $(INCDIRS) $(CXXFLAGS) -MD $(CO)$2 $1
endef
#########################################################
endif


#########################################################

define generate_prog
  $(quiet) $(LINK) $(BINDING) $(OUTLINKFLAG)$1 $($2_compiled_objs) $(filter $(LIBCOMPFLAG)%,$($2_LDFLAGS)) \
  $(PRE_LDFLAGS) $(XOBJECTS)  $(filter-out $(LIBCOMPFLAG)%,$($2_LDFLAGS)) $(POST_LDFLAGS) $(SYSLIBS)
  $(if $(filter release,$(strip $(DEFAULT_LIBMODE))),$(call strip_binary,$1))
endef

#########################################################

define generate_static_lib
  $(quiet) $(AR) $(AR_ARGS)$1  $2
endef

#########################################################

define generate_shared-archive_lib
  $(quiet) $(AR) $(AR_ARGS)$1  $2
endef


#########################################################

define generate_shared_lib
  $(quiet) $(SHARED_LINK) $(SHARED_PRE_LDFLAGS) $(SONAME_ARG)$(notdir $1) -o $1 $2 $(SHARED_POST_LDFLAGS)
  $(if $(filter release,$(strip $(DEFAULT_LIBMODE))),$(call strip_binary,$1))
endef

define strip_binary
  $(quiet) $(STRIP) $(STRIP_FLAGS) $1
endef
