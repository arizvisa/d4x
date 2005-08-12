#
# $Header: /home/cvs/d4x/Makefile.am,v 1.2.2.1 2005/08/08 11:54:38 zaufi Exp $
#

ACLOCAL_AMFLAGS = -I m4

SUBDIRS         = DOC intl main po share

# Extra files/dirs to be included in distribution
EXTRA_DIST = $(top_srcdir)/admin/Makefile.template \
             $(top_srcdir)/admin/config.rpath \
             $(top_srcdir)/admin/mkinstalldirs \
             $(top_srcdir)/admin/strip.awk \
             $(top_srcdir)/support/class.tpl \
             $(top_srcdir)/support/nt.spec \
             $(top_srcdir)/ChangeLog-1.x \
             $(top_srcdir)/PLANS \
             $(top_srcdir)/po/Makevars

# RPMs we can build
RPM_TARGETS     = rpm src-rpm all-rpms

SPEC_DST        = /usr/src/RPM/SPECS

# Generic files to be removed
CLEANFILES = $(top_srcdir)/*~            \
             $(top_srcdir)/po/*~         \
             $(top_srcdir)/admin/*~      \
             $(top_builddir)/doxygen.log \
             $(top_builddir)/rpm-build.log

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
doxy-docs: $(top_builddir)/support/Doxyfile
	if test -f $<; then \
	    $(DOXYGEN) $(top_builddir)/support/Doxyfile; \
	fi

.PHONY: doxy-docs
