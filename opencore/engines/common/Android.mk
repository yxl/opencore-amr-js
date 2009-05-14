LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
 	




LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/engines/common/src \
 	$(PV_TOP)/engines/common/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pv_common_types.h \
 	include/pv_config_interface.h \
 	include/pv_engine_observer.h \
 	include/pv_engine_observer_message.h \
 	include/pv_engine_types.h \
 	include/pv_interface_cmd_message.h

include $(BUILD_COPY_HEADERS)
