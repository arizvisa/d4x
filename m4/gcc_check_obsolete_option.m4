##
## $Header: /home/cvs/d4x/m4/gcc_check_obsolete_option.m4,v 1.1.1.1 2004/07/01 15:37:55 max Exp $
##
## Sun 10 Nov 2002 12:45:02 AM MSK by Zaufi
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
AC_DEFUN([GCC_CHECK_OBSOLETE_OPTION],[
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
