# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmp4ffcomposer


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)






SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := amrdecoderspecificinfo3gpp.cpp \
	amrsampleentry.cpp \
    assetinfoatoms.cpp \
	atom.cpp \
	atomutils.cpp \
	audiosampleentry.cpp \
	avcconfiguration.cpp \
	avcsampleentry.cpp \
	basedescriptor.cpp \
	chunkoffsetatom.cpp \
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
	fullatom.cpp \
	h263decoderspecificinfo3gpp.cpp \
	h263sampleentry.cpp \
	handleratom.cpp \
	hintmediaheaderatom.cpp \
	impeg4file.cpp \
	mediaatom.cpp \
	mediadataatom.cpp \
	mediaheaderatom.cpp \
	mediainformationatom.cpp \
	mediainformationheaderatom.cpp \
	movieatom.cpp \
	movieheaderatom.cpp \
	mpeg4file.cpp \
	mpeg4mediaheaderatom.cpp \
	mpegsampleentry.cpp \
	sampledescriptionatom.cpp \
	sampleentry.cpp \
	samplesizeatom.cpp \
	sampletableatom.cpp \
	sampletochunkatom.cpp \
	slconfigdescriptor.cpp \
	soundmediaheaderatom.cpp \
	syncsampleatom.cpp \
	timetosampleatom.cpp \
	trackatom.cpp \
	trackheaderatom.cpp \
	trackreferenceatom.cpp \
	trackreferencetypeatom.cpp \
	userdataatom.cpp \
	videomediaheaderatom.cpp \
	visualsampleentry.cpp \
	textsampleentry.cpp \
	boxrecord.cpp \
	fontrecord.cpp \
	fonttableatom.cpp \
	stylerecord.cpp \
	interleavebuffer.cpp \
	movieextendsatom.cpp \
	movieextendsheaderatom.cpp \
	moviefragmentatom.cpp \
	moviefragmentheaderatom.cpp \
	moviefragmentrandomaccessatom.cpp \
	moviefragmentrandomaccessoffsetatom.cpp \
	trackextendsatom.cpp \
	trackfragmentatom.cpp \
	trackfragmentheaderatom.cpp \
	trackfragmentrandomaccessatom.cpp \
	trackfragmentrunatom.cpp
	
HDRS := a_impeg4file.h \
       a_isucceedfail.h \
       a_atomdefs.h \
       textsampledescinfo.h \
       ../config/opencore/pv_mp4ffcomposer_config.h


include $(MK)/library.mk
