#
# $Header: /var/cvs/d4x/share/Makefile.am,v 1.7 2002/08/15 08:21:44 max Exp $
#

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = sounds themes ftpsearch.xml

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(pkgdatadir)
	for i in `find $(srcdir) -type d -not -name "CVS"`; do \
	    cur_dir=$(pkgdatadir)`echo $$i | sed 's,$(srcdir),,'`; \
	    echo "mkdir $$cur_dir"; \
	    if ! test -d $$cur_dir; then mkdir $$cur_dir; fi \
	done
	for i in `find $(srcdir) -type f -not -regex ".*CVS.*" -not -name "Makefile*" -not -name ".cvsignore"`; do \
	  echo "$(pkgdatadir)/`dirname $$i | sed 's,$(srcdir)/,,'`/`basename $$i`;"; \
	  $(INSTALL_DATA) $$i $(pkgdatadir)/`dirname $$i | sed 's,$(srcdir)/,,'`/`basename $$i`; \
	done

uninstall-local:
	rm -rf $(pkgdatadir)

