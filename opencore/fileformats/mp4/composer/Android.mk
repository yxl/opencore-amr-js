LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/amrdecoderspecificinfo3gpp.cpp \
 	src/amrsampleentry.cpp \
 	src/assetinfoatoms.cpp \
 	src/atom.cpp \
 	src/atomutils.cpp \
 	src/audiosampleentry.cpp \
 	src/avcconfiguration.cpp \
 	src/avcsampleentry.cpp \
 	src/basedescriptor.cpp \
 	src/chunkoffsetatom.cpp \
 	src/dataentryatom.cpp \
 	src/dataentryurlatom.cpp \
 	src/datainformationatom.cpp \
 	src/datareferenceatom.cpp \
 	src/decoderconfigdescriptor.cpp \
 	src/decoderspecificinfo.cpp \
 	src/editatom.cpp \
 	src/editlistatom.cpp \
 	src/esdatom.cpp \
 	src/esdescriptor.cpp \
 	src/es_id_inc.cpp \
 	src/es_id_ref.cpp \
 	src/expandablebaseclass.cpp \
 	src/filetypeatom.cpp \
 	src/fullatom.cpp \
 	src/h263decoderspecificinfo3gpp.cpp \
 	src/h263sampleentry.cpp \
 	src/handleratom.cpp \
 	src/hintmediaheaderatom.cpp \
 	src/impeg4file.cpp \
 	src/mediaatom.cpp \
 	src/mediadataatom.cpp \
 	src/mediaheaderatom.cpp \
 	src/mediainformationatom.cpp \
 	src/mediainformationheaderatom.cpp \
 	src/movieatom.cpp \
 	src/movieheaderatom.cpp \
 	src/mpeg4file.cpp \
 	src/mpeg4mediaheaderatom.cpp \
 	src/mpegsampleentry.cpp \
 	src/sampledescriptionatom.cpp \
 	src/sampleentry.cpp \
 	src/samplesizeatom.cpp \
 	src/sampletableatom.cpp \
 	src/sampletochunkatom.cpp \
 	src/slconfigdescriptor.cpp \
 	src/soundmediaheaderatom.cpp \
 	src/syncsampleatom.cpp \
 	src/timetosampleatom.cpp \
 	src/trackatom.cpp \
 	src/trackheaderatom.cpp \
 	src/trackreferenceatom.cpp \
 	src/trackreferencetypeatom.cpp \
 	src/userdataatom.cpp \
 	src/videomediaheaderatom.cpp \
 	src/visualsampleentry.cpp \
 	src/textsampleentry.cpp \
 	src/boxrecord.cpp \
 	src/fontrecord.cpp \
 	src/fonttableatom.cpp \
 	src/stylerecord.cpp \
 	src/interleavebuffer.cpp \
 	src/movieextendsatom.cpp \
 	src/movieextendsheaderatom.cpp \
 	src/moviefragmentatom.cpp \
 	src/moviefragmentheaderatom.cpp \
 	src/moviefragmentrandomaccessatom.cpp \
 	src/moviefragmentrandomaccessoffsetatom.cpp \
 	src/trackextendsatom.cpp \
 	src/trackfragmentatom.cpp \
 	src/trackfragmentheaderatom.cpp \
 	src/trackfragmentrandomaccessatom.cpp \
 	src/trackfragmentrunatom.cpp


LOCAL_MODULE := libpvmp4ffcomposer

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/fileformats/mp4/composer/src \
 	$(PV_TOP)/fileformats/mp4/composer/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/a_impeg4file.h \
 	include/a_isucceedfail.h \
 	include/a_atomdefs.h \
 	include/textsampledescinfo.h \
 	include/../config/opencore/pv_mp4ffcomposer_config.h

include $(BUILD_STATIC_LIBRARY)
