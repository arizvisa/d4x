PACKAGE  = nt
XGETTEXT = xgettext
MSGFMT   = msgfmt
MSGMERGE = msgmerge

INSTALL_DATA=install -c -m644

ifeq ($(OSTYPE),FreeBSD)
ifdef BSD_INSTALL_DATA
INSTALL_DATA=$(BSD_INSTALL_DATA)
endif
endif

all:
	for i in `ls *.po`; do \
		$(MSGFMT) $$i -o `echo $$i | sed -e s/.po//`.gmo ; \
	done


$(PACKAGE).pot:
	$(XGETTEXT) --default-domain=$(PACKAGE) --add-comments \
	  --keyword=_ --keyword=N_ --files-from=POTFILES \
	&& test ! -f $(PACKAGE).po \
	   || ( rm -f $(PACKAGE).pot && mv $(PACKAGE).po $(PACKAGE).pot )


update-po:
	rm -f $(PACKAGE).pot
	$(MAKE) $(PACKAGE).pot
	for i in `ls *.po`; do \
		$(MSGMERGE) $$i $(PACKAGE).pot -o $$i; \
	done



install:
	for i in `ls *.gmo` ; do \
		mkdir -p $(LOCALEDIR)/`echo $$i|sed -e s/.gmo//`/LC_MESSAGES;\
		$(INSTALL_DATA) $$i $(LOCALEDIR)/`echo $$i | sed -e s/.gmo//`/LC_MESSAGES/$(PACKAGE).mo ; \
	done

uninstall:
	for i in `ls *.po` ; do \
		rm -f $(LOCALEDIR)/`echo $$i | sed -e s/.po//`/LC_MESSAGES/$(PACKAGE).mo || test -z "" ; \
	done

clean:
	rm -f *~ *.gmo messages
