##
## $Header: /var/cvs/d4x/admin/acinclude.m4,v 1.3.2.1 2002/10/25 07:17:23 max Exp $
##
## 20/02/2002 by Zaufi
##
## 'Bcouse AM_WITH_REGEX can't solve my problem with choosing
## right libraty to compile with (using redistributable
## object files (as original macro do) is not our way :)
## 
## We export (and substitute) LIBRX variable wich can be added to Makefile.am
## into LDADD string for component wich is needed to be linked with librx.a...
## Also you must include the following code when choose that header to include
##
##	#ifdef WITH_REGEX
##	# include <regex.h>
##	#else
##	# ifdef __cplusplus
##	extern "C" {
##	# endif
##	# include <rxposix.h>
##	# ifdef __cplusplus
##	}
##	# endif
##	#endif
##
## * I don't know why but 1.5 version of rx (latest for nowdays) can't be included
##   into C++ code w/o extern "C"...
##

AC_DEFUN(AM_WITH_RX,
[
    ## Spam what we check...
    AC_MSG_CHECKING(which of GNU rx or gawk's regex is wanted)

    ## Set initial value
    am_with_rx="no"
    LIBRX=""

    ## Get configurable parameter
    AC_ARG_WITH(rx,
	AC_HELP_STRING([--with-rx], [Use fastest GNU rx instead of gawk's regex for matching]),
	[am_with_rx=$withval])
    if test "$am_with_rx" = "yes"; then
	AC_MSG_RESULT(rx)
	AC_PATH_PROGS(LIBRX, librx.a, not_found, [/usr/local/lib:/usr/lib])
	if test "$LIBRX" = "not_found"; then
	    AC_ERROR([*** Required library librx.a not found])
	fi
    else
        AC_MSG_RESULT(regex)
	AC_DEFINE(WITH_REGEX, 1, [Define if using GNU regex])
	AC_CACHE_CHECK([for GNU regex in libc], am_cv_gnu_regex,
	    AC_TRY_LINK([], [extern int re_max_failures; re_max_failures = 1],
	                am_cv_gnu_regex=yes, am_cv_gnu_regex=no))
	if test $am_cv_gnu_regex = no; then
	    AC_MSG_RESULT(failure)
	    AC_MSG_ERROR([*** Can't find regex functions in your libc])
	fi
    fi
    AC_SUBST(LIBRX)
])

# d4xao.m4
# configure defines for checking libao
# on RedHat-8.0 and other
# written by Maxim Koshelev

AC_DEFUN([D4X_PATH_AO],[
	ifdef([XIPH_PATH_AO],
	      [XIPH_PATH_AO([$1],[$2])],
	      [AM_PATH_AO([$1],[$2])])]
)
