#
# $Header: /var/cvs/d4x/Makefile.am,v 1.23 2002/05/21 11:31:22 zaufi Exp $
#

ACLOCAL_AMFLAGS = -I admin

SUBDIRS = intl main po share DOC

EXTRA_DIST = admin/strip.awk nt-gray.png nt-mini.xpm nt-wm.png nt.desktop nt.png \
nt.wmconfig nt.xpm PLANS ChangeLog-1.x reconf nt.spec

# Remove all generated and autoinstalled (linked) files (maintainer mode only)
MAINTAINERCLEANFILES = aclocal.m4 config.log config.status configure Makefile.in \
config.h.in stamp-h.in INSTALL po/Makefile.in.in ABOUT-NLS ChangeLog.bak \
po/ChangeLog po/$(PACKAGE).pot main/*~ face/*~ po/*~ $(top_srcdir)/*~

CLEANFILES = *~ po/*~

maintainer-clean-local:
	rm -rf $(top_srcdir)/intl
	rm -rf $(top_srcdir)/autom4te*.cache

# Remove CVS related dirs during distribute creation
dist-hook:
	find $(distdir) \( -type d -a -name "CVS" \) -print | xargs rm -rf

# Goal to create source rpm
src-rpm: dist
	cp -f $(PACKAGE)-$(VERSION).tar.gz /usr/src/RPM/SOURCES
	mv -f `rpm -bs nt.spec  | sed 's/Wrote: \(.*\)/\1/'` $(top_srcdir)
