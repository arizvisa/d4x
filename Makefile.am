#
# $Header: /var/cvs/d4x/Makefile.am,v 1.18 2002/03/20 03:43:27 max Exp $
#

SUBDIRS = intl main po share DOC

EXTRA_DIST = admin nt-gray.png nt-mini.xpm nt-wm.png nt.desktop nt.png nt.wmconfig nt.xpm \
ABOUT-NLS AUTHORS NEWS PLANS README TODO ChangeLog ChangeLog-1.x reconf nt.spec

# Remove CVS related dirs during distribute creation
dist-hook:
	find $(distdir) \( -type d -a -name "CVS" \) -print | xargs rm -rf

# Goal to create source rpm
src-rpm: dist
	cp -f $(PACKAGE)-$(VERSION).tar.gz /usr/src/RPM/SOURCES
	mv -f `rpm -bs nt.spec  | sed 's/Wrote: \(.*\)/\1/'` $(top_srcdir)

# Remove all generated and autoinstalled (linked) files
MAINTAINERCLEANFILES = aclocal.m4 config.log config.status configure Makefile.in \
package_config.h.in stamp-h.in INSTALL po/Makefile.in.in ABOUT-NLS \
po/ChangeLog po/$(PACKAGE).pot main/*~ face/*~ po/*~

maintainer-clean-local:
	rm -rf $(top_srcdir)/intl
