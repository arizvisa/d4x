# Solaris Openwin 2.6 headers are not properly casted to for
# compilation with C++/g++. Three files Xlib.h, Xresource.h, Xutil.h
# need to be fixed to compile this. Or add a path to the X11R6 headers
ifeq ($(OSTYPE),Solaris)
SOLARIS_TYPECASTED_X11_HEADERS=-I/var/tmp/src
endif
ifndef CXXFLAGS
CXXFLAGS=-Wall -O2 $(SOLARIS_TYPECASTED_X11_HEADERS)
endif

CXXFLAGS+=-D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS $(SOLARIS_TYPECASTED_X11_HEADERS)
#-DDEBUG_ALL
# -ggdb

export CXXFLAGS LOCALEDIR

NLS=-DENABLE_NLS # Remove or comment this line to disable gettext
ifeq ($(DEST),)
DEST=/usr/local
endif

LOCALEDIR=$(DEST)/share/locale
PTHREAD=-lpthread
GTKCONFIG=gtk-config
GLIBCONFIG=glib-config
MAKE=make

#next strings were added for BSD support
ifeq ($(OSTYPE),BSD)
PTHREAD=-pthread
GTKCONFIG=gtk12-config
MAKE=gmake
endif

ifeq ($(OSTYPE),FreeBSD)
PTHREAD=-pthread
GTKCONFIG=gtk12-config
MAKE=gmake
endif

GTKLIBS=$(shell $(GTKCONFIG) --libs) $(shell $(GLIBCONFIG) --libs)
GTKFLAGS=$(shell $(GTKCONFIG) --cflags) $(shell $(GLIBCONFIG) --cflags)

ifeq ($(OSTYPE),Irix)
CTKFLIBS += lintl
MAKE=gmake
CXXFLAGS += -fpermissive
endif

LDFLAGS=$(PTHREAD) $(GTKLIBS)
CXXFLAGS += $(GTKFLAGS) $(NLS) -DLOCALE=\"${LOCALEDIR}\"

FACE=face/lmenu.o face/list.o face/addd.o face/graph.o face/log.o \
	face/prefs.o face/buttons.o face/about.o face/edit.o \
	face/colors.o face/saveload.o face/limface.o face/misc.o \
	face/lod.o face/columns.o face/dndtrash.o face/passface.o \
	face/mywidget.o face/myclist.o face/fsface.o
MODULES1=base64.o addr.o queue.o liststr.o dlist.o mdlist.o locstr.o \
	log.o  sort.o meter.o socket.o history.o  speed.o srvclt.o html.o \
	client.o ftp.o http.o download.o ftpd.o httpd.o hproxy.o signal.o \
	main.o config.o savelog.o mainlog.o var.o sortstr.o db.o cookie.o \
	pass.o eff.o segments.o memwl.o fsearch.o msgqueue.o ping.o
MODULES = $(FACE) $(MODULES1)

all:
	-rm -f nt
	$(MAKE) nt
	[ -z "$(NLS)" ] || $(MAKE) -C po
	$(MAKE) strip

$(FACE):
	$(MAKE) -C face

nt: $(MODULES)
	$(MAKE) -C face
	$(CXX) -lstdc++ $(MODULES) nt.cc -o nt $(CXXFLAGS) $(GTKLIBS) $(PTHREAD)

strip: nt
	strip nt


install: all
	install -c -m755 nt $(DEST)/bin
	[ -z "$(NLS)" ] || $(MAKE) install -C po

clean: 
	-rm -f *.o nt TAGS *~
	$(MAKE) clean -C face
	$(MAKE) clean -C po

tags:
	etags *.{cc,h} face/*.{cc,h}

uninstall:
	-rm -f $(DEST)/bin/nt
	$(MAKE) uninstall -C po

Makefile.dep:
	$(CXX) $(CXXFLAGS) -MM *.cc > Makefile.dep

-include Makefile.dep
