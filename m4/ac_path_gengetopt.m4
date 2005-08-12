##
## $Header: /home/cvs/d4x/m4/ac_path_gengetopt.m4,v 1.1 2005/03/03 14:25:33 zaufi Exp $
##
##
## 20/03/2002 by Zaufi
##
## AC_PATH_GENGETOPT(MIN-VERSION, [ACTION-IF-FOUND, [ACTION-IF-NOT-FOUND]])
##
## If gengetopt not found or wrong version, GENGETOPT will be equ to missing
## script call. In any case AC_SUBST will be called.
##
##
AC_DEFUN([AC_PATH_GENGETOPT],
[
    ## Spam what we check...
    AC_PATH_PROG(GENGETOPT, gengetopt, no, /usr/local/bin:$PATH)

    if test "x$GENGETOPT" == "xno"; then
        ifelse([$3], , :, [$3])
        GENGETOPT="${am_missing_run}gengetopt"
    else
        ggo_min_version=ifelse([$1], , 2.5, $1)
        AC_MSG_CHECKING(for gengetopt vesion >= $ggo_min_version)
        ggo_version=`$GENGETOPT --version | head -n 1 | sed 's/^GNU gengetopt \([[0-9]]\+\.[[0-9]]\+\.*[[0-9]]*\).*/\1/'`
        ggo_major=`echo $ggo_version | sed 's/\([[0-9]]\+\)\.\([[0-9]]\+\)\.*\([[0-9]]*\)/\1/'`
        ggo_minor=`echo $ggo_version | sed 's/\([[0-9]]\+\)\.\([[0-9]]\+\)\.*\([[0-9]]*\)/\2/'`
        ggo_patch=`echo $ggo_version | sed 's/\([[0-9]]\+\)\.\([[0-9]]\+\)\.*\([[0-9]]*\)/\3/'`
        ggo_major=`test -z $ggo_major && echo 0 || echo $ggo_major`
        ggo_minor=`test -z $ggo_minor && echo 0 || echo $ggo_minor`
        ggo_patch=`test -z $ggo_patch && echo 0 || echo $ggo_patch`
        ##
        ggo_min_major=`echo $ggo_min_version | sed 's/\([[0-9]]\+\)\.\([[0-9]]\+\)\.*\([[0-9]]*\)/\1/'`
        ggo_min_minor=`echo $ggo_min_version | sed 's/\([[0-9]]\+\)\.\([[0-9]]\+\)\.*\([[0-9]]*\)/\2/'`
        ggo_min_patch=`echo $ggo_min_version | sed 's/\([[0-9]]\+\)\.\([[0-9]]\+\)\.*\([[0-9]]*\)/\3/'`
        ggo_min_major=`test -z $ggo_min_major && echo 0 || echo $ggo_min_major`
        ggo_min_minor=`test -z $ggo_min_minor && echo 0 || echo $ggo_min_minor`
        ggo_min_patch=`test -z $ggo_min_patch && echo 0 || echo $ggo_min_patch`
        ##
        if test $ggo_major -ge $ggo_min_major -a $ggo_minor -ge $ggo_min_minor -a $ggo_patch -ge $ggo_min_patch ; then
            AC_MSG_RESULT(yes)
            ifelse([$2], , :, [$2])
        else
            AC_MSG_RESULT(no)
            ifelse([$3], , :, [$3])
            GENGETOPT="${am_missing_run}gengetopt"
        fi
    fi
])
