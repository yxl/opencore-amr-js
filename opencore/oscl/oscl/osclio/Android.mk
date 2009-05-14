LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/oscl_dns.cpp \
 	src/oscl_dns_gethostbyname.cpp \
 	src/oscl_dns_imp_base.cpp \
 	src/oscl_dns_imp_pv.cpp \
 	src/oscl_dns_method.cpp \
 	src/oscl_dns_param.cpp \
 	src/oscl_dns_request.cpp \
 	src/oscl_file_async_read.cpp \
 	src/oscl_file_cache.cpp \
 	src/oscl_file_dir_utils.cpp \
 	src/oscl_file_find.cpp \
 	src/oscl_file_io.cpp \
 	src/oscl_file_native.cpp \
 	src/oscl_file_server.cpp \
 	src/oscl_file_stats.cpp \
 	src/oscl_ip_socket.cpp \
 	src/oscl_socket.cpp \
 	src/oscl_socket_accept.cpp \
 	src/oscl_socket_bind.cpp \
 	src/oscl_socket_connect.cpp \
 	src/oscl_socket_imp_base.cpp \
 	src/oscl_socket_imp_pv.cpp \
 	src/oscl_socket_imp_serv.cpp \
 	src/oscl_socket_listen.cpp \
 	src/oscl_socket_method.cpp \
 	src/oscl_socket_recv.cpp \
 	src/oscl_socket_recv_from.cpp \
 	src/oscl_socket_request.cpp \
 	src/oscl_socket_send.cpp \
 	src/oscl_socket_send_to.cpp \
 	src/oscl_socket_serv_imp_pv.cpp \
 	src/oscl_socket_serv_imp_reqlist.cpp \
 	src/oscl_socket_shutdown.cpp \
 	src/oscl_socket_stats.cpp \
 	src/oscl_tcp_socket.cpp \
 	src/oscl_udp_socket.cpp


LOCAL_MODULE := libosclio

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/oscl/oscl/osclio/src \
 	$(PV_TOP)/oscl/oscl/osclio/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/oscl_file_dir_utils.h \
 	src/oscl_file_find.h \
 	src/oscl_file_handle.h \
 	src/oscl_file_io.h \
 	src/oscl_file_server.h \
 	src/oscl_socket.h \
 	src/oscl_socket_types.h \
 	src/oscl_dns.h \
 	src/oscl_file_types.h

include $(BUILD_STATIC_LIBRARY)
