CCPLUS = c++
CFLAGS=-Wall -O2 -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS
EXECC=$(CCPLUS) $(CFLAGS) $(GTKFLAGS) $(NLS) -DLOCALE=\"${LOCALEDIR}\"
GTKFLAGS=`gtk-config --cflags`
GTKLIBS=`gtk-config --libs`

all:list.o colors.o addd.o graph.o edit.o log.o prefs.o lmenu.o buttons.o about.o saveload.o \
limface.o misc.o

list.o: list.cc list.h
	$(EXECC) $(GTKFLAGS) -c list.cc -o list.o

lmenu.o: lmenu.cc lmenu.h
	$(EXECC) $(GTKFLAGS) -c lmenu.cc -o lmenu.o

addd.o: addd.cc addd.h
	$(EXECC) $(GTKFLAGS) -c addd.cc -o addd.o

log.o: log.cc log.h
	$(EXECC) $(GTKFLAGS) -c log.cc -o log.o

misc.o: misc.cc misc.h
	$(EXECC) $(GTKFLAGS) -c misc.cc -o misc.o

colors.o: colors.cc colors.h
	$(EXECC) $(GTKFLAGS) -c colors.cc -o colors.o

edit.o: edit.cc edit.h
	$(EXECC) $(GTKFLAGS) -c edit.cc -o edit.o

graph.o: graph.cc graph.h
	$(EXECC) $(GTKFLAGS) -c graph.cc -o graph.o

buttons.o: buttons.cc buttons.h
	$(EXECC) $(GTKFLAGS) -c buttons.cc -o buttons.o

prefs.o: prefs.cc prefs.h
	$(EXECC) $(GTKFLAGS) -c prefs.cc -o prefs.o

about.o: about.cc about.h
	$(EXECC) $(GTKFLAGS) -c about.cc -o about.o

saveload.o: saveload.cc saveload.h
	$(EXECC) $(GTKFLAGS) -c saveload.cc -o saveload.o

limface.o: limface.cc limface.h
	$(EXECC) $(GTKFLAGS) -c limface.cc -o limface.o

clean:
	rm -f *.o *~ ||test -z ""