#
# $Header: /var/cvs/d4x/Makefile.am,v 1.26.2.3 2002/10/12 22:44:33 zaufi Exp $
#

ACLOCAL_AMFLAGS = -I admin

SUBDIRS = admin DOC intl main po share

EXTRA_DIST = admin/strip.awk po/Makevars PLANS ChangeLog-1.x nt.spec DOC

# Generic files to be removed
CLEANFILES = $(top_srcdir)/*~ $(top_srcdir)/po/*~ $(top_srcdir)/admin/*~

# Remove all maintainer specific files
MAINTAINERCLEANFILES = \
    $(top_srcdir)/ABOUT-NLS \
    $(top_srcdir)/ChangeLog.bak \
    $(top_srcdir)/INSTALL \
    $(top_srcdir)/Makefile.in \
    $(top_srcdir)/aclocal.m4 \
    $(top_srcdir)/config.h.in \
    $(top_srcdir)/configure \
    $(top_srcdir)/po/Makefile.in.in \
    $(top_srcdir)/po/Makevars.template \
    $(top_srcdir)/po/Rules-quot \
    $(top_srcdir)/po/*.sed \
    $(top_srcdir)/po/*.header \
    $(top_srcdir)/po/*.sin \
    $(top_srcdir)/admin/depcomp \
    $(top_srcdir)/admin/missing \
    $(top_srcdir)/admin/mkinstalldirs \
    $(top_srcdir)/admin/maintainer.log \
    $(top_srcdir)/admin/install-sh \
    $(top_srcdir)/admin/Makefile.am \
    $(top_srcdir)/admin/Makefile.in \
    $(shell find $(top_srcdir)/admin -name '*.m4' -a -not -name 'acinclude.m4' -o -name 'config.*')

# Remove generated directories
maintainer-clean-local:
	rm -rf $(top_srcdir)/intl
	rm -rf $(top_srcdir)/autom4te*.cache

# Goal to create source rpm
# (use copy instead move to allow increment of packet version)
src-rpm: dist
	cp -f $(PACKAGE)-$(VERSION).tar.gz /usr/src/RPM/SOURCES
	cp -f `rpm -bs nt.spec  | sed 's/Wrote: \(.*\)/\1/'` $(top_builddir)
