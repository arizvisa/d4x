#
# $Header: /var/cvs/d4x/Makefile.am,v 1.41 2003/04/30 10:45:24 vav Exp $
#

ACLOCAL_AMFLAGS = -I m4

SUBDIRS         = DOC intl main po share

# Extra files/dirs to be included in distribution
EXTRA_DIST      = admin/config.rpath admin/mkinstalldirs  admin/config.rpath  admin/config.rpath              \
                  admin/mkinstalldirs             \
                  admin/strip.awk                 \
                  admin/Makefile.addons           \
                  admin/Doxyfile.in               \
                  m4/am_with_rx.m4                \
                  m4/d4xao.m4                     \
                  m4/gcc_check_obsolete_option.m4 \
                  po/Makevars                     \
                  ChangeLog-1.x                   \
                  PLANS                           \
                  VERSION                         \
                  nt.spec

# RPMs we can build
RPM_TARGETS     = rpm src-rpm all-rpms

SPEC_DST        = /usr/src/RPM/SPECS

# Generic files to be removed
CLEANFILES = $(top_srcdir)/*~            \
             $(top_srcdir)/po/*~         \
             $(top_srcdir)/admin/*~      \
             $(top_builddir)/doxygen.log \
             $(top_builddir)/rpm-build.log

# Remove all maintainer specific files
MAINTAINERCLEANFILES =                 \
    $(top_srcdir)/ABOUT-NLS            \
    $(top_srcdir)/ChangeLog.bak        \
    $(top_srcdir)/INSTALL              \
    $(top_srcdir)/Makefile.in          \
    $(top_srcdir)/aclocal.m4           \
    $(top_srcdir)/config.h.in          \
    $(top_srcdir)/configure            \
    $(top_srcdir)/po/Makefile.in.in    \
    $(top_srcdir)/po/Makevars.template \
    $(top_srcdir)/po/Rules-quot        \
    $(top_srcdir)/po/*.sed             \
    $(top_srcdir)/po/*.header          \
    $(top_srcdir)/po/*.sin             \
    $(top_srcdir)/admin/depcomp        \
    $(top_srcdir)/admin/missing        \
    $(top_srcdir)/admin/mkinstalldirs  \
    $(top_srcdir)/admin/maintainer.log \
    $(top_srcdir)/admin/install-sh     \
    $(top_srcdir)/admin/Makefile.am    \
    $(top_srcdir)/admin/Makefile.in    \
    $(top_builddir)/admin/Doxyfile     \
    $(top_srcdir)/m4/*~                \
    $(top_srcdir)/m4/Makefile*         \
    $(shell find $(top_srcdir)/admin -name 'config.*')

# Remove generated directories
maintainer-clean-local:
	rm -rf $(top_srcdir)/intl
	rm -rf $(top_srcdir)/autom4te*.cache

# Goal to create source rpm
# (use copy instead move to allow increment of packet version)
$(RPM_TARGETS):
	@if ! test -d $(SPEC_DST) -a -O $(SPEC_DST); then \
	    echo "make[$(MAKELEVEL)]: You have no permissions to $(SPEC_DST) to produce .rpm package"; \
	    exit 1; \
	fi
	$(MAKE) $(MAKEFLAGS) dist-gzip || exit 1
	case "$(@)" in \
	    all-rpms) rpm_flags="-ta" ;; \
	    src-rpm)  rpm_flags="-ts" ;; \
	    rpm)      rpm_flags="-tb" ;; \
	    *)        exit 1;; \
	esac; \
	$(RPM) $$rpm_flags $(PACKAGE)-$(VERSION).tar.gz 2>&1 | tee $(top_builddir)/rpm-build.log || exit 1; \
	cp -f `cat $(top_builddir)/rpm-build.log | grep '^Wrote:' | sed 's/Wrote: \(.*\)/\1/'` $(top_builddir)

#
# Run doxygen to produce html documentation for source code
#
html-docs: $(top_builddir)/admin/Doxyfile
	if test -f $<; then \
	    cd $(top_builddir)/admin; \
	    $(DOXYGEN); \
	fi
