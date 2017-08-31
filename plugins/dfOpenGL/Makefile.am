INCLUDES = -DPIXMAPDIR=\"${datadir}/pixmaps/\" \
        -DLOCALE_DIR=\"${datadir}/locale/\" \
        -DDATADIR=\"${datadir}/psemu/\" \
	$(GTK2_CFLAGS) $(GLADE2_CFLAGS) \
	-g -O0 -Wall -fPIC

libdir = $(prefix)/lib/games/psemu/
lib_LTLIBRARIES = libdfOpenGL.la

libdfOpenGL_la_SOURCES = GPU.c GPU.h \
	primitive_drawing.c primitive_drawing.h \
	fps.c gllog.c cfg.c cfg.h gpu_i.h
libdfOpenGL_la_LDFLAGS = -L/usr/lib -lXxf86vm -lGL -lm

bindir = $(prefix)/lib/games/psemu/
bin_PROGRAMS = cfgDFOpenGL
cfgDFOpenGL_SOURCES = gpucfg-0.1df/main.c
cfgDFOpenGL_LDADD = $(GTK2_LIBS) $(GLADE2_LIBS)

glade_DATA = gpucfg-0.1df/dfopengl.glade2
gladedir = $(datadir)/psemu/

EXTRA_DIST = $(glade_DATA) \
	LICENSE TODO oldMakefile PSEmu_PlugIn_Defs.h

