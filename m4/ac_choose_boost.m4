##
## $Header: /home/cvs/d4x/m4/ac_choose_boost.m4,v 1.1.2.1 2005/08/08 11:54:39 zaufi Exp $
##
##
## Copyright (c) 2003, 2004 by Zaufi
##
## AC_CHOOSE_BOOST([REQUIRED_VERSION], [INCLUDE_PATH_HINT], [LIB_PATH_HINT], [ACTION-IF-FOUND, [ACTION-IF-NOT-FOUND]]])
##
## Set the following variables:
##  BOOST_VERSION   - same as defined in boost/version.hpp
##  BOOST_CPPFLAGS  - `-I' preprocessor option
##  BOOST_LDFLAGS   - `-L` linker option
##
AC_DEFUN([AC_CHOOSE_BOOST],[
    ac_choose_boost_result='no'

    am_inc_boost_dir_default=ifelse([$2], , "/usr/include /usr/local/include", $2)
    am_lib_boost_dir_default=ifelse([$3], , "/usr/lib /usr/local/lib", $3)

    BOOST_CPPFLAGS=''
    BOOST_VERSION=''
    BOOST_LDFLAGS=''

    AC_ARG_WITH(boost_includedir,
                AS_HELP_STRING([--with-boost-includedir=DIR],
                               [Use Boost library headers installed at specified path(default=check)]),
                [am_inc_boost_dir=$withval],
                [am_inc_boost_dir=$am_inc_boost_dir_default])
    AC_ARG_WITH(boost_libdir,
                AS_HELP_STRING([--with-boost-libdir=DIR],
                               [Use Boost libraries installed at specified path (default=check)]),
                [am_lib_boost_dir=$withval],
                [am_lib_boost_dir=$am_lib_boost_dir_default])

    ## Check headers
    AC_CACHE_CHECK([where boost headers installed], [ac_cv_boost_headers_installed_at],
    [
        ac_cv_boost_headers_installed_at='not found'
        if test -n "$am_inc_boost_dir"; then
            for dir in $am_inc_boost_dir; do
                if test -f "$dir/boost/version.hpp"; then
                    BOOST_CPPFLAGS="-I$dir"
                    ac_cv_boost_headers_installed_at=$dir
                    break
                fi
            done
        fi
    ])

    ## TODO: Fair check libraries needed
    if test "$ac_cv_boost_headers_installed_at" != 'not found'; then
        BOOST_LDFLAGS="-L`dirname $ac_cv_boost_headers_installed_at`/lib"
    fi

    ## Check version
    if test "$ac_cv_boost_headers_installed_at" != 'not found'; then
        AC_CACHE_CHECK([boost version], [ac_cv_boost_version],
        [
            ac_cv_boost_version=`cat $ac_cv_boost_headers_installed_at/boost/version.hpp \
              | grep  '^# *define  *BOOST_VERSION  *[0-9]\+$' \
              | sed 's,^# *define  *BOOST_VERSION  *\([0-9]\+\)$,\1,'`
        ])
        am_required_boost_version=ifelse([$1], , $ac_cv_boost_version, $1)
        ## Try to compile simple program
        AC_CACHE_CHECK([boost version >= $am_required_boost_version], [ac_cv_boost_version_ok],
        [
            ac_cv_boost_version_ok='no'
            old_CPPFLAGS=$CPPFLAGS
            CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
            AC_LANG_PUSH(C++)
            AC_COMPILE_IFELSE(
            [
                #include <boost/version.hpp>
                #if BOOST_VERSION < $am_required_boost_version
                #error Boost version less than required
                #endif
            ], [ac_cv_boost_version_ok='yes'])
            AC_LANG_POP(C++)
            CPPFLAGS=$old_CPPFLAGS
        ])
        if test "$ac_cv_boost_version_ok" = 'yes'; then
            BOOST_VERSION=$ac_cv_boost_version
            ac_choose_boost_result='yes'
        fi
    fi

    if test "$ac_choose_boost_result" = 'yes'; then
        ifelse([$4], , :, [$4])
    else
        ifelse([$5], , :, [$5])
    fi

    AC_SUBST(BOOST_CPPFLAGS)
    AC_SUBST(BOOST_LDFLAGS)
    AC_SUBST(BOOST_VERSION)
])
