#
# $Header: /var/cvs/d4x/DOC/Makefile.am,v 1.8.2.1 2002/10/11 18:58:59 zaufi Exp $
#

MAINTAINERCLEANFILES = Makefile.in

pkgdata_DATA = $(shell ls $(srcdir)/FAQ* $(srcdir)/README*) LICENSE TROUBLES

# Prefix 'dist_' mean that man page gets in a distrubution
dist_man_MANS = nt.1

# Hook to compress man page on the fly... (after install)
man_conf = /etc/man.config
install-data-hook:
	@echo -n "Look for $(man_conf)..."; \
	if test -f $(man_conf); then \
	  echo "found"; \
	  echo -n "Compressed man pages extension... "; \
	  man_ext=`grep "COMPRESS_EXT" $(man_conf) | head -1 | $(AWK) '{print $$2}'`; \
	  echo \'$$man_ext\'; \
	else \
	  echo "not found"; \
	  echo "Assume no compression of manpages required/possible"; \
	fi; \
	case "$$man_ext" in \
	  .bz2) bzip2 -fz $(DESTDIR)/$(man1dir)/$(dist_man_MANS) ;; \
	  .gz)  gzip -f $(DESTDIR)/$(man1dir)/$(dist_man_MANS) ;; \
	  .Z)   compress -f $(DESTDIR)/$(man1dir)/$(dist_man_MANS) ;; \
	esac

uninstall-hook:
	rm -f $(DESTDIR)/$(man1dir)/$(dist_man_MANS).*
