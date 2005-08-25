# -*-m4-*-
# $Header: /home/cvs/d4x/configure.ac,v 1.2.2.11 2005/08/24 09:08:58 max Exp $
#
# Process this file with autoconf to produce a configure script.
#
AC_REVISION($Revision: 1.2.2.11 $)
AC_PREREQ(2.52)

# -----------------------------------------------------------------------------
# Initialize environment
# -----------------------------------------------------------------------------
m4_include([support/version.m4])
AC_INIT([d4x], [VERSION_STR], [chuchelo@krasu.ru])

AC_PKG_INFO_HDR([$PACKAGE_NAME -- Web Downloader for X])

AC_CONFIG_SRCDIR([admin/strip.awk])
AC_CONFIG_AUX_DIR([admin])
AC_CONFIG_HEADERS([config.h])

# This ksh/zsh (and bash >= 2.05) feature conflict with `cd blah; pwd`
unset CDPATH

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([1.7 gnu dist-bzip2])

AC_PREFIX_DEFAULT(/usr/local)

# -----------------------------------------------------------------------------
# Parse some configuration keys
# -----------------------------------------------------------------------------

# NOTE: Automatic Makefiles rebuild works ONLY in maintainer mode!!!
AM_MAINTAINER_MODE

# --disable-release flags... i.e. no optimized code
# (to allow debuggers show/trace source code properly)
# ATTENTION: We shouldn't redefine CXXFLAGS! Possible config.site already loaded!
OPTFLAGS="-O0"


# Optimization options
AC_ARG_ENABLE(release, 
    AC_HELP_STRING([--enable-release], [Compile with usual optimization level without debug code (default)]),,
    [enable_release='yes'])
if test "$enable_release" = 'yes'; then
    OPTFLAGS="-O2"
fi
AC_ARG_ENABLE(extra-optimize,
    AC_HELP_STRING([--enable-extra-optimize],
                   [Such as --enable-release + compile with maximum optimizations]))
if test "$enable_extra_optimize" = 'yes'; then
    OPTFLAGS="-O3 -fexpensive-optimizations -fomit-frame-pointer"
fi
AM_CONDITIONAL(EXTRA_OPTIMIZE, test "$enable_extra_optimize" = 'yes')
CXXFLAGS="$CXXFLAGS $OPTFLAGS"


# Maintainer helpers
AC_ARG_ENABLE(debug,
    AC_HELP_STRING([--enable-debug], [Enable debug info and some debugging code to be compiled]),
    [if test "$enable_debug" = 'yes'; then
	CXXFLAGS="$CXXFLAGS -ggdb"
        AC_DEFINE(DEBUG_ALL, , [Activate debug code])
    fi])
AC_ARG_ENABLE(warnings,
    AC_HELP_STRING([--enable-warnings], [Enable all warnings at compile time]),
    [if test "$enable_warnings" = 'yes'; then
	CXXFLAGS="$CXXFLAGS -Wall"
    fi])
AC_ARG_ENABLE(profile,
    AC_HELP_STRING([--enable-profile], [Enable profiling info]),
    [if test "$enable_profile" == 'yes'; then
        CXXFLAGS="$CXXFLAGS -pg"
    fi])

# Extra features options
AC_ARG_ENABLE(esd,
    AC_HELP_STRING([--disable-esd], [Disable esound output (default=enabled)]),,
    enable_esd='yes')
AC_ARG_ENABLE(oss,
    AC_HELP_STRING([--disable-oss], [Disable OSS sound output (default=enabled)]),,
    enable_oss='yes')
AC_ARG_ENABLE(libao,
    AC_HELP_STRING([--enable-libao],
    [Enable libao sound output and disable all others sound libs (default=disabled)]),,
    enable_libao='no')

AC_ARG_ENABLE(openssl,
    AC_HELP_STRING([--disable-openssl], [Disable Open SSL support (default=autodetect)]),,
    [enable_openssl='yes'])

# -----------------------------------------------------------------------------
# Checks for OS depended options
# -----------------------------------------------------------------------------

ac_pthread_strict_check='yes'
# Check for OS
case "$host" in
    *-solaris*)
	# From original Makefile: SOLARIS_TYPECASTED_X11_HEADERS
	OS_INCLUDES="-I/var/tmp/src"
	AC_DEFINE(_POSIX_THREADS_SEMANTICS, ,[Needed for Solaris reentrant code])
    ;;
    *-freebsd*)
	OS_CXXFLAGS="-pthread"
	ac_pthread_strict_check=no
    ;;
    *-irix*)
	OS_CXXFLAGS="-fpermissive"
    ;;
esac


# -----------------------------------------------------------------------------
# Checks for programs (1)
# -----------------------------------------------------------------------------
# Need to do following 2 check unconditionaly _before_
# _conditional_ CPU optimizations checks.
AC_PROG_CXX
AC_PROG_CC

# -----------------------------------------------------------------------------
# Check for CPU.
# -----------------------------------------------------------------------------
#
# I know exactly that on Intel and ADM CPUs this improve code
# but 'bout others I can say nothing...
#
# Note that you can add more options in config.site file
# (c info automake)
#
if test "$enable_extra_optimize" = 'yes'; then
    AC_MSG_CHECKING(host CPU architecture) 

    case "$host" in
	i586-* | i686-* | x86_64-* )
	    AC_MSG_RESULT([AMD/Intel 32 or 64 bit])
    	    if test "$enable_extra_optimize" = 'yes'; then
		OS_CXXFLAGS="$OS_CXXFLAGS -fforce-addr -maccumulate-outgoing-args -ffast-math -fprefetch-loop-arrays"
	    fi
	;;
        *)
	    AC_MSG_RESULT([unknown])
	;;
    esac
fi

# -----------------------------------------------------------------------------
# Checks for programs (2)
# -----------------------------------------------------------------------------
AC_PROG_INSTALL
AM_PROG_INSTALL_STRIP
AC_PROG_AWK
#AC_PROG_LIBTOOL
AC_PROG_RANLIB
AM_GNU_GETTEXT
AM_GNU_GETTEXT_VERSION(0.14.1)
AC_PATH_PROG(AUTOGEN, autogen, [${am_missing_run}autogen], /usr/local/bin:$PATH)
AC_PATH_PROG(DOXYGEN, doxygen, [${am_missing_run}doxygen], /usr/local/bin:$PATH)
AC_PATH_PROG(RPM, rpm, [${am_missing_run}rpm], /usr/local/bin:$PATH)
HAVE_DOT=NO
AC_PATH_PROG(DOT, dot,, /usr/local/bin:$PATH)
AC_PROG_LN_S

# Is doxygen can generate graphics diagrams using graphviz's dot?
test -n "$DOT" && HAVE_DOT=YES && DOT=`dirname "$DOT"`
AC_SUBST(HAVE_DOT)

AC_PATH_GENGETOPT([2.11], [ac_gengetopt=yes],[ac_gengetopt=no])
AC_PATH_PROG(AUTOGEN, autogen, [${am_missing_run}autogen], /usr/local/bin:$PATH)

# -----------------------------------------------------------------------------
# Checks for libraries.
# -----------------------------------------------------------------------------
# Should we check for ao?
if test "$enable_libao" = 'yes'; then
    D4X_PATH_AO([AC_DEFINE(D4X_WITH_AO,,[Define if libao should be used])])
    enable_esd=no
    enable_oss=no
fi
# Should we check for esound?
if test "$enable_esd" = 'yes'; then
    AM_PATH_ESD(,[AC_DEFINE(HAVE_ESD,,[Define if have libesd])])
    AC_SUBST(ESD_CFLAGS)
    AC_SUBST(ESD_LIBS)
fi

AM_PATH_GLIB_2_0(2.4.0,, AC_MSG_ERROR([*** GLIB >= 2.4.0 not installed! ***]))
AM_PATH_GTK_2_0(2.4.0,, AC_MSG_ERROR([*** GTK >= 2.4.0 not installed! ***]))
if test "$enable_openssl" = 'yes'; then
    AC_FIND_OPENSSL([>= 0.9.7])
fi

# POSIX threads should be always present...
if test "$ac_pthread_strict_check" = 'yes'; then
    AC_CHECK_LIB([pthread], [pthread_testcancel],, AC_MSG_ERROR([*** POSIX threads not found! ***]))
fi

AM_PATH_CPPUNIT(1.8.0, [ac_cppunit=yes],[ac_cppunit=no])

# Choose regex library
# AM_WITH_RX

# -----------------------------------------------------------------------------
# Checks for header files.
# -----------------------------------------------------------------------------
AC_PATH_X
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h errno.h fcntl.h netdb.h])
AC_CHECK_HEADERS([netinet/in.h sys/file.h regex.h])
AC_CHECK_HEADERS([sys/ioctl.h sys/socket.h])
AC_CHECK_HEADERS([utime.h pthread.h sys/timeb.h])
if test "$enable_oss" = 'yes'; then
    AC_CHECK_HEADERS([sys/soundcard.h], [AC_DEFINE(D4X_WITH_OSS,, [Define if OSS enabled])])
fi

# Test C++ header files...
AC_LANG_PUSH(C++)

# do not test CppUnit headers if no gengetopt found or cppunit-config not found/failed
if test "$ac_cppunit" = 'yes' -a "$ac_gengetopt" = 'yes'; then
    AC_CHECK_HEADERS([cppunit/extensions/HelperMacros.h cppunit/extensions/TestFactoryRegistry.h \
                      cppunit/ui/text/TestRunner.h],
                     [ac_cppunit_hdrs=yes], [ac_cppunit_hdrs=no])
fi

AC_LANG_POP(C++)

# -----------------------------------------------------------------------------
# Checks for typedefs, structures, and compiler characteristics.
# -----------------------------------------------------------------------------
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_STRUCT_TIMEZONE
AC_TYPE_UID_T

# -----------------------------------------------------------------------------
# Checks for library functions.
# -----------------------------------------------------------------------------
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_UTIME_NULL
AC_CHECK_FUNCS([ftime ftruncate gethostbyname gettimeofday localtime_r])
AC_CHECK_FUNCS([memmove memset mkdir regcomp select socket])
AC_CHECK_FUNCS([strerror strncasecmp strstr utime flock pthread_testcancel])

# -----------------------------------------------------------------------------
# Additianal actions before produce output files
# -----------------------------------------------------------------------------

AM_CONDITIONAL([WITH_UNIT_TESTS], test "$ac_cppunit" = 'yes' -a "$ac_gengetopt" = 'yes' -a "$ac_cppunit_hdrs" = 'yes')

# It's time to define $prefix if it was ommited...
if test "$prefix" = "NONE"; then
  prefix=$ac_default_prefix
  ac_configure_args="$ac_configure_args --prefix $prefix"
fi

# Define locales directory
localedir=`eval "echo ${datadir}/locale"`
AC_SUBST(localedir)
AC_DEFINE_UNQUOTED(LOCALEDIR,
                   "$localedir",
                   [Location of locales])

# Define package directory
sharedir=`eval "echo ${datadir}"`
AC_DEFINE_UNQUOTED(D4X_SHARE_PATH,
                   "$sharedir/$PACKAGE",
                   [Location of architecture independed files])

# Other needed defines
AC_DEFINE(_REENTRANT, , [Enable reenrant code of glibc])
AC_DEFINE(GTK_DISABLE_DEPRECATED, , [Enable warnings about deprecated functions usage])
AC_DEFINE(_FILE_OFFSET_BITS, 64, [To support files over 2GB])

# Prepare all requiered flags
CXXFLAGS="$CXXFLAGS $OS_CXXFLAGS"
CPPFLAGS="$CPPFLAGS $ESD_CFLAGS $ESD_CFLAGS $GLIB_CFLAGS $GTK_CFLAGS $OS_INCLUDES $AO_CFLAGS $SSL_CFLAGS"
LIBS="$LIBS $ESD_LIBS $GLIB_LIBS $GTK_LIBS $LIBRX $AO_LIBS $SSL_LIBS"
LDFLAGS="$LDFLAGS $SSL_LDFLAGS"

# Remove duplicates from options
CPPFLAGS=`echo $CPPFLAGS | $AWK -f ${srcdir}/admin/strip.awk`
CXXFLAGS=`echo $CXXFLAGS | $AWK -f ${srcdir}/admin/strip.awk`
LIBS=`echo $LIBS | $AWK -f ${srcdir}/admin/strip.awk`

# Remove -lpthread for some OS'es
if test "$ac_pthread_strict_check" = 'no'; then
    LIBS=`echo $LIBS | sed 's/\(-lpthread\)//'`
fi

# Force config.h be included before every module compilation
if test "x$ac_config_headers" != "x"; then
    CPPFLAGS="$CPPFLAGS -include \$(CONFIG_HEADER)"
fi

# Search (and remove) some `-D' compiler options which we are already have in our config.h
new_CPPFLAGS=''
for i in $CPPFLAGS; do
    case $i in
    -D_REENTRANT) ;;
    *) new_CPPFLAGS="$new_CPPFLAGS $i" ;;
    esac
done
CPPFLAGS=$new_CPPFLAGS

# Schedule commands to execute after Makefiles creation
# (included gettext doesn't need our CPPFLAGS, moreover
# it is not define CONFIG_HEADER in itsown Makefile so
# we get a compile errors 'bcouse incorrect command line)
AC_CONFIG_COMMANDS(fixer,
[
    # The following command will be executed by config.status
    # with `pwd` == $top_builddir...
    if test -f intl/Makefile; then
	cat intl/Makefile | sed 's/^CPPFLAGS *=.*/CPPFLAGS = /' > intl/Makefile.tmp
	mv -f intl/Makefile.tmp intl/Makefile
    fi
])

# Make #include guards for config.h
AH_TOP([
#ifdef __D4X_CONFIG_H
# error Do not include this file directly! All needed actions already done by configure script.
#else /* __D4X_CONFIG_H */
# define __D4X_CONFIG_H
])
AH_BOTTOM([#endif /* !__D4X_CONFIG_H */])

# -----------------------------------------------------------------------------
# Create output files
# -----------------------------------------------------------------------------
AC_CONFIG_FILES([      \
    Makefile           \
    po/Makefile.in     \
    DOC/Makefile       \
    intl/Makefile      \
    main/Makefile      \
    main/face/Makefile \
    share/Makefile     \
    support/Doxyfile   \
    support/nt.spec    \
])
AC_OUTPUT

# -----------------------------------------------------------------------------
# Show our configuration
# -----------------------------------------------------------------------------
# 1) calculate all what we need to display
cxx_version=`${CXX} --version | head -n 1`
if test "$enable_esd" = 'no'; then
    esd_spam="disabled"
else
    esd_spam="enabled"
fi
if test "$enable_oss" = 'yes'; then
    oss_spam="enabled"
else
    oss_spam="disabled"
fi
if test "$enable_libao" = 'yes'; then
    ao_spam="enabled"
else
    ao_spam="disabled"
fi
acm="debug"
if test "$enable_release" = 'yes'; then
    acm="release"
fi
if test "$enable_extra_optimize" = 'yes'; then
    acm="release (extra optimize)"
fi
if test "$USE_MAINTAINER_MODE" = 'yes'; then
    acm="$acm (maintainer mode)"
fi
if test "$ac_cppunit" = 'yes' -a "$ac_gengetopt" = 'yes'; then
    unit_tests='yes (use `make check`)'
else
    unit_tests='no ('
    c=''
    if test "$ac_cppunit" = 'no' -o "$ac_cppunit_hdrs" = 'no'; then
        unit_tests="${unit_tests}no CppUnit found"
        c=', '
    fi
    if test "$ac_gengetopt" = 'no'; then
        unit_tests="${unit_tests}${c}no gengetopt found"
    fi
    unit_tests="${unit_tests})"
fi

# 2) show config
AC_SUM_INFO_HDR(77, [Summary])
    AC_SET_INFO_INDENT(21, 56)
    AC_MSG_INFO_LINE([      Configuration: ], [$acm])
    AC_MSG_INFO_LINE
    AC_MSG_INFO_LINE([   Host system type: ], [$host])
    AC_MSG_INFO_LINE([       C++ compiler: ], [$cxx_version])
    AC_MSG_INFO_LINE([       Install path: ], [$prefix])
    AC_MSG_INFO_LINE
    AC_MSG_INFO_LINE([         ESD output: ], [$esd_spam])
    AC_MSG_INFO_LINE([         OSS output: ], [$oss_spam])
    AC_MSG_INFO_LINE([       libao output: ], [$ao_spam])
    AC_MSG_INFO_LINE
    AC_MSG_INFO_LINE([        SSL enabled: ], [$enable_openssl])
    AC_MSG_INFO_LINE
if test "$USE_MAINTAINER_MODE" = 'yes'; then
    AC_MSG_INFO_LINE([ Preprocessor flags: ], [$CPPFLAGS])
    AC_MSG_INFO_LINE([     Compiler flags: ], [$CXXFLAGS])
    AC_MSG_INFO_LINE([       Linker flags: ], [$LDFLAGS])
    AC_MSG_INFO_LINE([    Addon libraries: ], [$LIBS])
    AC_MSG_INFO_LINE
    AC_MSG_INFO_LINE([  Can compile tests: ], [$unit_tests])
    AC_MSG_INFO_LINE
fi
AC_SUM_INFO_HDR(77)
