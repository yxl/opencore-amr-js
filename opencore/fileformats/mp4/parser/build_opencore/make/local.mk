# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmp4ff


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

XCPPFLAGS += -DOPEN_FILE_ONCE_PER_TRACK  


XINCDIRS := ../../utils/mp4recognizer/include


SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := 3gpp_amrdecoderspecificinfo.cpp \
	3gpp_h263decoderspecificinfo.cpp \
	amrdecoderspecificinfo.cpp \
	amrsampleentry.cpp \
	assetinfoatoms.cpp \
	atom.cpp \
	audiosampleentry.cpp \
	avcconfigurationbox.cpp \
        avcsampleentry.cpp \
	avcsampledependencytype.cpp \
	avcsampledependency.cpp \
	avcsampletogroupbox.cpp \
	avclayerdescriptionentry.cpp \
	avcsubseqdescriptionentry.cpp \
	basedescriptor.cpp \
	boxrecord.cpp \
	chunkoffsetatom.cpp \
	compositionoffsetatom.cpp \
	copyrightatom.cpp \
	dataentryatom.cpp \
	dataentryurlatom.cpp \
	datainformationatom.cpp \
	datareferenceatom.cpp \
	decoderconfigdescriptor.cpp \
	decoderspecificinfo.cpp \
	editatom.cpp \
	editlistatom.cpp \
	esdatom.cpp \
	esdescriptor.cpp \
	es_id_inc.cpp \
	es_id_ref.cpp \
	expandablebaseclass.cpp \
	filetypeatom.cpp \
	fontrecord.cpp \
	fonttableatom.cpp \
	fullatom.cpp \
	h263decoderspecificinfo.cpp \
	h263sampleentry.cpp \
	handleratom.cpp \
	hintmediaheaderatom.cpp \
	impeg4file.cpp \
	initialobjectdescriptor.cpp \
	itunesilstatom.cpp \
	mediaatom.cpp \
	mediabuffer.cpp \
	mediabuffermanager.cpp \
	mediadataatom.cpp \
	mediaheaderatom.cpp \
	mediainformationatom.cpp \
	mediainformationheaderatom.cpp \
	metadataatom.cpp \
	movieatom.cpp \
	movieheaderatom.cpp \
	mpeg4file.cpp \
	mpeg4mediaheaderatom.cpp \
	mpeg4sampleentryextensions.cpp \
	mpegsampleentry.cpp \
	moviefragmentheaderatom.cpp \
	moviefragmentatom.cpp \
	movieextendsheaderatom.cpp \
	movieextendsatom.cpp \
	mfraoffsetatom.cpp \
	moviefragmentrandomaccess.cpp \
	objectdescriptor.cpp \
	objectdescriptoratom.cpp \
	oma2boxes.cpp \
	pvuserdataatom.cpp \
	sampledescriptionatom.cpp \
	sampleentry.cpp \
	samplesizeatom.cpp \
	sampletableatom.cpp \
	sampletochunkatom.cpp \
	slconfigdescriptor.cpp \
	soundmediaheaderatom.cpp \
	stylerecord.cpp \
	syncsampleatom.cpp \
	textsampleentry.cpp \
	textsamplemodifiers.cpp \
	timetosampleatom.cpp \
	trackatom.cpp \
	trackheaderatom.cpp \
	trackreferenceatom.cpp \
	trackreferencetypeatom.cpp \
	trackfragmentrunatom.cpp \
	trackfragmentheaderatom.cpp \
	trackfragmentatom.cpp \
	trackextendsatom.cpp \
	trackfragmentrandomaccess.cpp \
	userdataatom.cpp \
	videomediaheaderatom.cpp \
	visualsampleentry.cpp \
	paspatom.cpp \

HDRS := impeg4file.h \
	isucceedfail.h \
        textsampleentry.h \
        sampleentry.h \
        atom.h \
        parentable.h \
        renderable.h \
        fontrecord.h \
        h263decoderspecificinfo.h \
        decoderspecificinfo.h \
        basedescriptor.h \
        expandablebaseclass.h \
        ../config/opencore/pv_mp4ffparser_config.h



include $(MK)/library.mk
