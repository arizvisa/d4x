#
# $Header: /var/cvs/d4x/Makefile.am,v 1.26 2002/08/16 07:04:48 max Exp $
#

ACLOCAL_AMFLAGS = -I admin

SUBDIRS = main po share DOC

EXTRA_DIST = admin/config.rpath  admin/strip.awk nt-gray.png nt-mini.xpm nt-wm.png nt.desktop nt.png \
nt.wmconfig nt.xpm PLANS ChangeLog-1.x reconf nt.spec DOC

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
	find $(distdir) \( -type f -a -name ".cvsignore" \) -print | xargs rm -rf

# Goal to create source rpm
# (use copy instead move to allow increment of packet version)
src-rpm: dist
	cp -f $(PACKAGE)-$(VERSION).tar.gz /usr/src/RPM/SOURCES
	cp -f `rpm -bs nt.spec  | sed 's/Wrote: \(.*\)/\1/'` $(top_builddir)
