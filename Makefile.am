AUTOMAKE_OPTION = gnu

SUBDIRS = intl main po share DOC

EXTRA_DIST = admin nt-gray.png nt-mini.xpm nt-wm.png nt.desktop nt.png nt.spec nt.wmconfig nt.xpm \
             ABOUT-NLS AUTHORS README ChangeLog ChangeLog-1.x Makefile.am Makefile.in NEWS \
             PACK PLANS TODO configure configure.in reconf nt.spec nt.spec.in

# Remove CVS related dirs during distribute creation
dist-hook:
	find $(distdir) \( -type d -a -name "CVS" \) -print | xargs rm -rf

# Goal to create source rpm
src-rpm: dist
	cp -f $(PACKAGE)-$(VERSION).tar.gz /usr/src/RPM/SOURCES
	mv -f `rpm -bs nt.spec  | sed 's/Wrote: \(.*\)/\1/'` $(top_srcdir)
