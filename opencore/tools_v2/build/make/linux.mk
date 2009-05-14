XCPPFLAGS += -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64

INSTALL := install
INSTALL_OPTS := -c -D -m 444

SED := sed
MV := mv
RM := rm -f
RMDIR := rm -rf
CP := cp
MKDIR := mkdir -p

export RANLIB = ranlib

TOOLSET ?= g++

LIB_DIRS += $(LIBCOMPFLAG)/usr/local

SYS_THREAD_LIB := -lpthread
SYS_DL_LIB := -ldl

#########################################################

define map_abspath
  $(abspath $1)
endef

#########################################################

define INST_TEMPLATE
$(3)/$(1): $(2)/$(1)
	$(quiet) $(INSTALL) $(INSTALL_OPTS) $(2)/$(1) $(3)/$(1)
endef

define clean-path
  $(patsubst %/,%,$1)
endef
#########################################################
# $(call cd_and_run_test,$@_SOURCE_DIR,$<,$@_TEST_ARGS,$@_SOURCE_ARGS)
define cd_and_run_test
	$(quiet) cd $1 ; $2 $3 $4
endef
