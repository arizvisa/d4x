##
## $Header: /var/cvs/d4x/m4/am_with_rx.m4,v 1.1 2003/04/24 23:47:04 zaufi Exp $
##
##
## 20/02/2002 by Zaufi
##
## 'Bcouse AM_WITH_REGEX too old and can't solve my problem
## with choosing right libraty to compile with (using redistributable
## object files (as original macro do) is not our way :)
## 
## We export (and substitute) LIBRX variable wich should be added to Makefile.am
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
## * I don't lnow why but 1.5 version (latest fow nowdays) can't be included
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
	    AC_MSG_ERROR([*** Required library librx.a not found])
	fi
    else
        AC_MSG_RESULT(regex)
	AC_DEFINE(WITH_REGEX, 1, [Define if using GNU regex])
	AC_CACHE_CHECK([for GNU regex in libc], am_cv_gnu_regex,
	    AC_TRY_LINK([], [extern int re_max_failures; re_max_failures = 1],
	                am_cv_gnu_regex=yes, am_cv_gnu_regex=no))
	if test $am_cv_gnu_regex = no; then
	    AC_MSG_RESULT(failure)
	    AC_MSG_ERROR([*** Can't find regex functions in your libc.\n*** Maybe it is time to update your system? :)])
	fi
    fi
    AC_SUBST(LIBRX)
])
