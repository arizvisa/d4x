##
## $Header: /var/cvs/d4x/m4/d4xao.m4,v 1.1 2003/04/24 23:47:04 zaufi Exp $
##
## d4xao.m4
## configure defines for checking libao
## on RedHat-8.0 and other
## written by Maxim Koshelev

AC_DEFUN([D4X_PATH_AO],[
	ifdef([XIPH_PATH_AO],
	      [XIPH_PATH_AO([$1],[$2])],
	      [AM_PATH_AO([$1],[$2])])]
)
