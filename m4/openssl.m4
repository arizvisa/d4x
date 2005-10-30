##
## $Header: /home/cvs/d4x/m4/openssl.m4,v 1.1.2.2 2005/08/11 12:25:57 zaufi Exp $
##
## Wed 29 Oct 2003 02:11:44 PM MSK by Zaufi
##
## AC_FIND_OPENSSL([REQUIRED-VERSION, [ACTION-IF-FOUND, [ACTION-IF-NOT-FOUND]]])
##
## Try to find the SSL headers and libraries
##

AC_DEFUN([AC_FIND_OPENSSL],
[
    SSL_LIBS="-lssl"
    ac_ssl_includes=no
    ac_ssl_libraries=no
    ssl_libraries=""
    ssl_includes=""

    ac_ssl_needed_version="ifelse([$1], , ">= 0.9.7f", $1)"

    # try pkg-config to locate OpenSSL
    AC_MSG_NOTICE([try pkg-config to locate OpenSSL])
    PKG_CHECK_MODULES(SSL, openssl $ac_ssl_needed_version, ac_ssl_found='yes')
    if test "$ac_ssl_found" = 'yes'; then
        SSL_CFLAGS=`$PKG_CONFIG --cflags openssl`
        SSL_LIBS=`$PKG_CONFIG --libs-only-l openssl`
        SSL_LDFLAGS=`$PKG_CONFIG --libs-only-L --libs-only-other openssl`
        have_ssl=yes
    fi

    if test "$have_ssl" != yes; then
        SSL_CFLAGS="";
        SSL_LDFLAGS="";
        SSL_LIBS="";
    else
        AC_DEFINE(HAVE_SSL, 1, [If we are going to use OpenSSL])
    fi

    AC_SUBST(SSL_CFLAGS)
    AC_SUBST(SSL_LDFLAGS)
    AC_SUBST(SSL_LIBS)

    if test "$have_ssl" == yes; then
        ifelse([$2], , :, [$2])
    else
        ifelse([$3], , :, [$3])
    fi
])
