#
# $Header: /var/cvs/d4x/main/face/Makefile.am,v 1.7.2.2 2002/09/06 06:47:49 max Exp $
#

MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~

EXTRA_DIST           = pixmaps pixmaps2
noinst_LIBRARIES     = libface.a

libface_a_SOURCES = $(strip           \
  about.cc     fsched.cc  misc.cc     \
  addd.cc      fsface.cc  mywidget.cc \
  buttons.cc   graph.cc   passface.cc \
  colors.cc    list.cc    prefs.cc    \
  filtrgui.cc  lmenu.cc   qtree.cc    \
  dndtrash.cc  lod.cc     saveload.cc \
  edit.cc      log.cc                 \
)

noinst_HEADERS = $(strip              \
  about.h     fsched.h  misc.h        \
  addd.h      fsface.h  mywidget.h    \
  buttons.h   graph.h   passface.h    \
  colors.h    list.h    prefs.h       \
  filtrgui.h  lmenu.h   qtree.h       \
  dndtrash.h  lod.h     saveload.h    \
  edit.h      log.h                   \
)

# HINT:
# We can use fact that all files in this directory
# have the same name for 2 extensions (*.cc | *.h)
# and use this in the following trick:
#
# noinst_HEADERS = $(patsubst %.cc, %.h, $(libface_a_SOURCES))
#
# this helps us to avoid maintain 2 lists (long lists :) of files
# and exclude possible errors (i.e. unsyncronized lists)
#
