#
# $Header: /var/cvs/d4x/DOC/Makefile.am,v 1.6 2002/03/13 04:33:48 zaufi Exp $
#

MAINTAINERCLEANFILES = Makefile.in

pkgdata_DATA = INSTALL* FAQ* THANKS LICENSE NAMES TROUBLES
EXTRA_DIST = $(pkgdata_DATA)

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