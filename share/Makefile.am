#
# $Header: /var/cvs/d4x/share/Makefile.am,v 1.4 2002/03/12 00:43:55 zaufi Exp $
#

EXTRA_DIST = $(srcdir)/sounds $(srcdir)/themes

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(pkgdatadir)
	find $(srcdir) -type d -not -name "CVS" -exec $(mkinstalldirs) $(pkgdatadir)/'{}' ';'
	for i in `find . -type f -not -regex ".*CVS.*" -not -name "Makefile*" -not -name ".cvsignore"`; do \
	  echo "$(INSTALL_DATA) $$i $(pkgdatadir)/`dirname $$i`;"; \
	  $(INSTALL_DATA) $$i $(pkgdatadir)/`dirname $$i`; \
	done

uninstall-local:
	rm -rf $(pkgdatadir)
