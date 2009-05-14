
CXX := arm-linux-g++
CC := arm-linux-gcc
LINK := $(CXX)
SHARED_LINK := $(CXX)
AR := arm-linux-ar
STRIP := arm-linux-strip

# overwrite RANLIB set in linux.mk
export RANLIB = arm-linux-ranlib

# This toolchain supports linux-arm v5
DEFAULT_CPP_ASM_FLAGS := -DPV_ARM_GCC_V5
PROCESSOR := arm

# include the common engine header files in the search path
INCDIRS += \
    -I /opt/environments/linux_arm/data/omapts/linux/arm-tc/gcc-3.4.0-1/arm-linux \
    -I /opt/environments/linux_arm/target/include/dspbridge/ \
    -I /opt/environments/linux_arm/pvinclude

# Compiler specific libraries locations
LIB_DIRS += \
    -L/opt/environments/linux_arm/data/omapts/linux/arm-tc/gcc-3.4.0-1/arm-linux/lib \
    -L/opt/environments/linux_arm/pvlib \
    -L/opt/environments/linux_arm/target/lib \
    -L/opt/environments/linux_arm/rootfs/lib
