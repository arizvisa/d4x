CC = gcc
CCPLUS = c++

NLS=-DENABLE_NLS # Remove or comment this line to disable gettext
DEST=/usr/local
LOCALEDIR=$(DEST)/share/locale
LIBS=-lpthread
CFLAGS=-Wall -O2 -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS
GTKLIBS=`gtk-config --libs` 
GTKFLAGS=`gtk-config --cflags` `glib-config --cflags`

EXECC=$(CCPLUS) $(CFLAGS) $(GTKFLAGS) $(NLS) -DLOCALE=\"${LOCALEDIR}\"

MODULES=base64.o queue.o liststr.o dlist.o mdlist.o locstr.o log.o  sort.o meter.o socket.o history.o hproxy.o speed.o srvclt.o
MODULES2=client.o ftp.o http.o download.o ftpd.o httpd.o signal.o main.o config.o savelog.o mainlog.o var.o sortstr.o db.o
FACE=face/lmenu.o face/list.o face/addd.o face/graph.o face/log.o face/prefs.o face/buttons.o \
face/about.o face/edit.o face/colors.o face/saveload.o face/limface.o face/misc.o face/lod.o \
face/columns.o face/dndtrash.o

MAKE=make

all:
	cd face && NLS=$(NLS) $(MAKE)
	rm -f nt || test -z ""
	$(MAKE) nt
	if [ "$(NLS)" != "" ] ; then \
		cd po && make ; \
	fi
	$(MAKE) strip


nt: nt.cc $(MODULES) $(MODULES2) var.h
	$(EXECC)  $(LIBS) nt.cc $(MODULES) $(MODULES2) \
	 $(FACE) -o nt $(GTKLIBS)

strip: nt
	strip nt


install: all
	install -c -m755 nt $(DEST)/bin
	if [ "$(NLS)" != "" ] ; then \
		cd po && LOCALEDIR=$(LOCALEDIR) make install; \
	fi

main.o: main.cc main.h var.h
	$(EXECC) -c main.cc -o main.o

mainlog.o: mainlog.cc mainlog.h
	$(EXECC) -c mainlog.cc -o mainlog.o
signal.o: signal.cc signal.h
	$(EXECC) -c signal.cc -o signal.o

httpd.o: httpd.cc httpd.h http.o
	$(EXECC) -c httpd.cc -o httpd.o

ftpd.o: ftpd.cc ftpd.h ftp.o
	$(EXECC) -c ftpd.cc -o ftpd.o

download.o: download.cc download.h
	$(EXECC) -c download.cc -o download.o

hproxy.o: hproxy.cc hproxy.h
	$(EXECC) -c hproxy.cc -o hproxy.o

http.o: http.cc http.h 
	$(EXECC) -c http.cc -o http.o

ftp.o: ftp.cc ftp.h 
	$(EXECC) -c ftp.cc -o ftp.o

client.o: client.cc client.h
	$(EXECC) -c client.cc -o client.o

socket.o: socket.cc socket.h
	$(EXECC) -c socket.cc -o socket.o

mdlist.o: mdlist.cc mdlist.h
	$(EXECC) -c mdlist.cc -o mdlist.o

dlist.o: dlist.cc dlist.h
	$(EXECC) -c dlist.cc -o dlist.o

log.o: log.cc log.h
	$(EXECC) -c log.cc -o log.o

speed.o: speed.cc speed.h
	$(EXECC) -c speed.cc -o speed.o

var.o: var.cc var.h
	$(EXECC) -c var.cc -o var.o

history.o: history.cc history.h 
	$(EXECC) -c history.cc -o history.o

liststr.o: liststr.cc liststr.h
	$(EXECC) -c liststr.cc -o liststr.o

queue.o: queue.cc queue.h
	$(EXECC) -c queue.cc -o queue.o

savelog.o:savelog.cc savelog.h
	$(EXECC) -c savelog.cc -o savelog.o

config.o: config.cc config.h
	$(EXECC) -c config.cc -o config.o

locstr.o: locstr.cc locstr.h
	$(EXECC) -c locstr.cc -o locstr.o

meter.o: meter.cc meter.h sort.o
	$(EXECC) -c meter.cc -o meter.o

sort.o: sort.cc sort.h
	$(EXECC) -c sort.cc -o sort.o

sortstr.o: sortstr.cc sortstr.h
	$(EXECC) -c sortstr.cc -o sortstr.o

base64.o: base64.cc base64.h
	$(EXECC) -c base64.cc -o base64.o

srvclt.o: srvclt.cc srvclt.h
	$(EXECC) -c srvclt.cc -o srvclt.o

db.o: db.cc db.h
	$(EXECC) -c db.cc -o db.o

clean: 
	rm -f *.o nt *~ ||test -z ""
	cd face && make clean
	cd po && make clean
