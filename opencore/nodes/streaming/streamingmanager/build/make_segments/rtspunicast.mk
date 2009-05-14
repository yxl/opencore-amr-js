#RTSP Unicast Plugin

XINCDIRS += -I../../../../../protocols/rtsp_client_engine/src
XINCDIRS += -I../../plugins/rtspunicast/include
XINCDIRS += -I../../../../streaming/jitterbuffernode/jitterbuffer/rtp/include

SRCS += ../plugins/rtspunicast/src/pvmf_sm_rtsp_unicast_node_factory.cpp
SRCS += ../plugins/rtspunicast/src/pvmf_sm_fsp_rtsp_unicast.cpp
