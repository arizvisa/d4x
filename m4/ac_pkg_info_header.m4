##
## $Header: /home/cvs/d4x/m4/ac_pkg_info_header.m4,v 1.2.2.1 2005/08/08 11:54:39 zaufi Exp $
##
## Mon 24 Jun 2002 01:34:16 AM MSD by Zaufi -- initial implementation
## Tue 22 Mar 2005 02:56:18 AM MSK by Zaufi -- add macros to make cute configuration summary info block
##

##
## AC_REPLICATE_CHAR([CHAR], [COUNT])
##
## Replicate char to produce string with given length
##
AC_DEFUN([AC_REPLICATE_CHAR],
[
    ac_rc_char="m4_default([$1], [-])"
    ac_rc_length="m4_default([$2], [77])"
    
    repstrval=''
    while (( ${#repstrval} < $ac_rc_length )); do
	repstrval="${repstrval}${ac_rc_char}";
    done
])

##
## AC_PKG_INFO_HDR([Package title], )
##
## Draw info box with package title (first arg), version and cvs tag
##
AC_DEFUN([AC_PKG_INFO_HDR],
[
    ac_pih_title="m4_default([$1], AC_MSG_FAILURE([Package name should be given]))"
    ac_pih_length="m4_default([$2], [77])"
    AC_REPLICATE_CHAR([-], [$ac_pih_length])
    ac_pih_hdrline=$repstrval
    AC_MSG_RESULT([$ac_pih_hdrline])
    if test -n "$ac_pih_title"; then
        AC_MSG_RESULT([])
        ac_pih_space_sz=$(( $ac_pih_length - ${#ac_pih_title} ))
        ac_pih_space_sz=$(( ac_pih_space_sz / 2 ))
	AC_REPLICATE_CHAR([ ], [$ac_pih_length])
        ac_pih_space_line_org=$repstrval
        ac_pih_space_line=$ac_pih_space_line_org
        ac_pih_space_line=${ac_pih_space_line:0:$ac_pih_space_sz}
        AC_MSG_RESULT([${ac_pih_space_line}$1])
        AC_MSG_RESULT([])
    fi

    # Check CVS tag
    if test -f $srcdir/CVS/Tag; then
    	ac_pih_cvs_tag='CVS Branch: '`cat $srcdir/CVS/Tag | sed 's,^T\(.*\)$,\1,'`
    else if test -d $srcdir/CVS; then
	    ac_pih_cvs_tag='CVS Branch: HEAD'
    else
	    ac_pih_cvs_tag="$PACKAGE_BUGREPORT"
    fi fi

    ac_pih_version_tag=" Version: $PACKAGE_VERSION "
    if test -n "$ac_pih_cvs_tag"; then
        ac_pih_space_line=$ac_pih_space_line_org
	ac_pih_space_line=${ac_pih_space_line:0:$(( $ac_pih_length - ${#ac_pih_cvs_tag} - ${#ac_pih_version_tag} ))}
    fi
    AC_MSG_RESULT([${ac_pih_version_tag}${ac_pih_space_line}${ac_pih_cvs_tag}])
    AC_MSG_RESULT([$ac_pih_hdrline])
])

##
## AC_SUM_INFO_HDR([LEN], [TEXT])
##
## Show configuration summary
##
AC_DEFUN([AC_SUM_INFO_HDR],
[
    ac_sih_length="m4_default([$1], [77])"
    ac_sih_title="$2"
    if test -z "$ac_sih_title"; then
	ac_sih_dl="${ac_sih_length}"
    else
	ac_sih_dl=$(( ${ac_sih_length} - ${#ac_sih_title} - 5))
    fi
    AC_REPLICATE_CHAR([-], [$ac_sih_dl])
    if test -z "$ac_sih_title"; then
	AC_MSG_RESULT([${repstrval}])
    else
	AC_MSG_RESULT([${repstrval}<${ac_sih_title}>---])
    fi
])

##
## AC_SET_INFO_INDENT([INDENT], [WIDTH])
##
AC_DEFUN([AC_SET_INFO_INDENT],
[
    ac_info_indent="m4_default([$1], [27])"
    ac_info_width="m4_default([$2], [50])"
])

##
## AC_MSG_INFO_LINE([LABEL], [DATA])
##
AC_DEFUN([AC_MSG_INFO_LINE],
[
    AC_REQUIRE([AC_SET_INFO_INDENT])
    ac_mil_label="$1"
    AC_REPLICATE_CHAR([ ], [${ac_info_indent}])
    ac_mil_data="$2"
    if test -n "$ac_mil_data"; then
	if test $(( ${#ac_mil_data} > ${ac_info_width} )); then
            ac_mil_data=`echo "$2" | fold -sw ${ac_info_width} | sed "2~1 s/^/${repstrval}/"`
	fi
    else
	ac_mil_data=''
    fi
    AC_MSG_RESULT([${ac_mil_label}${ac_mil_data}])
])
