#
# $Header: /var/cvs/d4x/share/Makefile.am,v 1.9 2002/11/12 21:29:53 zaufi Exp $
#

MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~

THEMEFILES = $(shell find $(srcdir)/themes -name '*.png' -o -name '*.xml')
SOUNDFILES = $(shell find $(srcdir)/sounds -name '*.wav')

# Files to be included in distribution
EXTRA_DIST = $(subst $(srcdir)/,,$(SOUNDFILES)) $(subst $(srcdir)/,,$(THEMEFILES)) \
             ftpsearch.xml d4x_aqua.png nt.png nt.xpm nt-gray.png \
             nt-mini.xpm nt-wm.png nt.wmconfig nt.desktop

# Files to be installed into share/d4x directory
pkgdata_DATA = ftpsearch.xml

#
# Install/uninstall themes and sounds with original directory structure
#
install-data-local:
	for i in $(SOUNDFILES) $(THEMEFILES); do \
	    $(INSTALL_DATA) -D $$i $(pkgdatadir)/`dirname $$i | sed 's,$(srcdir)/,,'`/`basename $$i`; \
	done

uninstall-local:
	for i in $(SOUNDFILES) $(THEMEFILES); do \
	    rm -f $(pkgdatadir)/$$i; \
	done
