
noinst_PROGRAMS = amrnb-dec$(EXEEXT) amrnb-enc-sine$(EXEEXT) amrwb-dec$(EXEEXT) linkboth$(EXEEXT)

EXTRA_DIST = Makefile.alt

INCLUDES = -I$(top_srcdir)/amrnb -I$(top_srcdir)/amrwb

amrnb_dec_LDADD = $(top_builddir)/amrnb/libopencore-amrnb.la
amrnb_enc_sine_LDADD = $(top_builddir)/amrnb/libopencore-amrnb.la
amrnb_enc_sine_LDFLAGS = -lm
amrwb_dec_LDADD = $(top_builddir)/amrwb/libopencore-amrwb.la
linkboth_LDFLAGS = -static
linkboth_LDADD = $(top_builddir)/amrnb/libopencore-amrnb.la $(top_builddir)/amrwb/libopencore-amrwb.la

amrnb_dec_SOURCES = amrnb-dec.cpp wav.cpp
amrnb_enc_sine_SOURCES = amrnb-enc-sine.c
amrwb_dec_SOURCES = amrwb-dec.cpp wav.cpp
linkboth_SOURCES = linkboth.c

noinst_HEADERS = wav.h

