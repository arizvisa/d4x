##
## $Header: /home/cvs/d4x/m4/d4xao.m4,v 1.1.1.1 2004/07/01 15:37:55 max Exp $
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
