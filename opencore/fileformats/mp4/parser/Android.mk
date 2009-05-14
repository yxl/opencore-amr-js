LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/3gpp_amrdecoderspecificinfo.cpp \
 	src/3gpp_h263decoderspecificinfo.cpp \
 	src/amrdecoderspecificinfo.cpp \
 	src/amrsampleentry.cpp \
 	src/assetinfoatoms.cpp \
 	src/atom.cpp \
 	src/audiosampleentry.cpp \
 	src/avcconfigurationbox.cpp \
 	src/avcsampleentry.cpp \
 	src/avcsampledependencytype.cpp \
 	src/avcsampledependency.cpp \
 	src/avcsampletogroupbox.cpp \
 	src/avclayerdescriptionentry.cpp \
 	src/avcsubseqdescriptionentry.cpp \
 	src/basedescriptor.cpp \
 	src/boxrecord.cpp \
 	src/chunkoffsetatom.cpp \
 	src/compositionoffsetatom.cpp \
 	src/copyrightatom.cpp \
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
 	src/fontrecord.cpp \
 	src/fonttableatom.cpp \
 	src/fullatom.cpp \
 	src/h263decoderspecificinfo.cpp \
 	src/h263sampleentry.cpp \
 	src/handleratom.cpp \
 	src/hintmediaheaderatom.cpp \
 	src/impeg4file.cpp \
 	src/initialobjectdescriptor.cpp \
 	src/itunesilstatom.cpp \
 	src/mediaatom.cpp \
 	src/mediabuffer.cpp \
 	src/mediabuffermanager.cpp \
 	src/mediadataatom.cpp \
 	src/mediaheaderatom.cpp \
 	src/mediainformationatom.cpp \
 	src/mediainformationheaderatom.cpp \
 	src/metadataatom.cpp \
 	src/movieatom.cpp \
 	src/movieheaderatom.cpp \
 	src/mpeg4file.cpp \
 	src/mpeg4mediaheaderatom.cpp \
 	src/mpeg4sampleentryextensions.cpp \
 	src/mpegsampleentry.cpp \
 	src/moviefragmentheaderatom.cpp \
 	src/moviefragmentatom.cpp \
 	src/movieextendsheaderatom.cpp \
 	src/movieextendsatom.cpp \
 	src/mfraoffsetatom.cpp \
 	src/moviefragmentrandomaccess.cpp \
 	src/objectdescriptor.cpp \
 	src/objectdescriptoratom.cpp \
 	src/oma2boxes.cpp \
 	src/pvuserdataatom.cpp \
 	src/sampledescriptionatom.cpp \
 	src/sampleentry.cpp \
 	src/samplesizeatom.cpp \
 	src/sampletableatom.cpp \
 	src/sampletochunkatom.cpp \
 	src/slconfigdescriptor.cpp \
 	src/soundmediaheaderatom.cpp \
 	src/stylerecord.cpp \
 	src/syncsampleatom.cpp \
 	src/textsampleentry.cpp \
 	src/textsamplemodifiers.cpp \
 	src/timetosampleatom.cpp \
 	src/trackatom.cpp \
 	src/trackheaderatom.cpp \
 	src/trackreferenceatom.cpp \
 	src/trackreferencetypeatom.cpp \
 	src/trackfragmentrunatom.cpp \
 	src/trackfragmentheaderatom.cpp \
 	src/trackfragmentatom.cpp \
 	src/trackextendsatom.cpp \
 	src/trackfragmentrandomaccess.cpp \
 	src/userdataatom.cpp \
 	src/videomediaheaderatom.cpp \
 	src/visualsampleentry.cpp \
 	src/paspatom.cpp


LOCAL_MODULE := libpvmp4ff

LOCAL_CFLAGS := -DOPEN_FILE_ONCE_PER_TRACK $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/fileformats/mp4/parser/src \
 	$(PV_TOP)/fileformats/mp4/parser/include \
 	$(PV_TOP)/fileformats/mp4/parser/utils/mp4recognizer/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/impeg4file.h \
 	include/isucceedfail.h \
 	include/textsampleentry.h \
 	include/sampleentry.h \
 	include/atom.h \
 	include/parentable.h \
 	include/renderable.h \
 	include/fontrecord.h \
 	include/h263decoderspecificinfo.h \
 	include/decoderspecificinfo.h \
 	include/basedescriptor.h \
 	include/expandablebaseclass.h \
 	include/../config/opencore/pv_mp4ffparser_config.h

include $(BUILD_STATIC_LIBRARY)
