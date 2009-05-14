# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := osclio

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../src

# compose final src list for actual build
SRCS :=	oscl_dns.cpp \
	oscl_dns_gethostbyname.cpp \
	oscl_dns_imp_base.cpp \
	oscl_dns_imp_pv.cpp \
	oscl_dns_method.cpp \
	oscl_dns_param.cpp \
	oscl_dns_request.cpp \
	oscl_file_async_read.cpp \
	oscl_file_cache.cpp \
	oscl_file_dir_utils.cpp \
	oscl_file_find.cpp \
	oscl_file_io.cpp \
	oscl_file_native.cpp \
	oscl_file_server.cpp \
	oscl_file_stats.cpp \
	oscl_ip_socket.cpp \
	oscl_socket.cpp \
	oscl_socket_accept.cpp \
	oscl_socket_bind.cpp \
	oscl_socket_connect.cpp \
	oscl_socket_imp_base.cpp \
	oscl_socket_imp_pv.cpp \
	oscl_socket_imp_serv.cpp \
	oscl_socket_listen.cpp \
	oscl_socket_method.cpp \
	oscl_socket_recv.cpp \
	oscl_socket_recv_from.cpp \
	oscl_socket_request.cpp \
	oscl_socket_send.cpp \
	oscl_socket_send_to.cpp \
	oscl_socket_serv_imp_pv.cpp \
	oscl_socket_serv_imp_reqlist.cpp \
	oscl_socket_shutdown.cpp \
	oscl_socket_stats.cpp \
	oscl_tcp_socket.cpp \
	oscl_udp_socket.cpp


HDRS := oscl_file_dir_utils.h \
	oscl_file_find.h \
	oscl_file_handle.h \
	oscl_file_io.h \
	oscl_file_server.h \
	oscl_socket.h \
	oscl_socket_types.h \
	oscl_dns.h \
	oscl_file_types.h

include $(MK)/library.mk
