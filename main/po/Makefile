all:
	for i in `ls *.po` ; do \
		msgfmt $$i -o `echo $$i | sed -e s/.po//`.gmo ; \
	done

install:
	for i in `ls *.gmo` ; do \
		mkdirhier $(LOCALEDIR)/`echo $$i|sed -e s/.gmo//`/LC_MESSAGES;\
		install -c -m644 $$i $(LOCALEDIR)/`echo $$i | sed -e s/.gmo//`/LC_MESSAGES/nt.mo ; \
	done

uninstall:
	for i in `ls *.po` ; do \
		rm -f $(LOCALEDIR)/`echo $$i | sed -e s/.po//`/LC_MESSAGES/nt.mo || test -z "" ; \
	done

clean:
	rm -f *~ *.gmo
