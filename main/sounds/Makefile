INSTALL_DATA=install -c -m644

ifeq ($(OSTYPE),FreeBSD)
ifdef BSD_INSTALL_DATA
INSTALL_DATA=$(BSD_INSTALL_DATA)
endif
endif

D4X_SOUNDS=$(D4X_SHARE)/sounds

all:


install:
	mkdir -p $(D4X_SOUNDS)
	for i in `ls *.wav` ; do \
		$(INSTALL_DATA) $$i $(D4X_SOUNDS) ; \
	done

uninstall:
	for i in `ls *.wav` ; do \
		rm -f $(D4X_SOUNDS)/$$i ; \
	done
	rmdir $(D4X_SHARE)/sounds
	rmdir $(D4X_SHARE)

clean:
	rm -f *~ *.gmo messages
