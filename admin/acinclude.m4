##
## $Header: /var/cvs/d4x/admin/acinclude.m4,v 1.6 2002/11/10 04:05:56 zaufi Exp $
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
## * I don't know why, but 1.5 version of rx (latest for nowdays) can't be included
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

## d4xao.m4
## configure defines for checking libao
## on RedHat-8.0 and other
## written by Maxim Koshelev

AC_DEFUN([D4X_PATH_AO],[
	ifdef([XIPH_PATH_AO],
	      [XIPH_PATH_AO([$1],[$2])],
	      [AM_PATH_AO([$1],[$2])])]
)

##
## Checking for obsolete options of gcc
##
## $1 = option to check
## $2 = action if given option is obsolete
## $3 = action if given option is OK
##
## How this working: If option is obsolete gcc will issue the warning.
## Giving -Werror to compiler we force fail status code of compilation
## if any warning exist. Here the pretty simple test program, so nothing
## can happens 'cept obsolete option...
##
AC_DEFUN([D4X_GCC_CHECK_OBSOLETE_OPTION],[
    AC_REQUIRE([AC_PROG_CC])
    AC_CACHE_CHECK([whether $1 is obsolete option], [ac_cv_`echo $1 | tr =- __`_obsolete],
    [
	AC_LANG_SAVE
	AC_LANG_C
	old_CFLAGS=$CFLAGS
	CFLAGS="$CFLAGS -Werror $1"
	AC_TRY_COMPILE(,,
	    [eval "ac_cv_`echo $1 | tr =- __`_obsolete=no"],
	    [eval "ac_cv_`echo $1 | tr =- __`_obsolete=yes"])
	CFLAGS=$old_CFLAGS
	AC_LANG_RESTORE
    ])
    if eval "test \"\${ac_cv_`echo $1 | tr =- __`_obsolete}\" = \"yes\""; then
	[$2]
    else
	[$3]
    fi
])
