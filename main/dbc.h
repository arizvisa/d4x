/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2001 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _MY_DBC_ASSERT_DEFINES_
#define _MY_DBC_ASSERT_DEFINES_

#if defined(DEBUG_ALL)

#define DBC_RETURN_IF_FAIL(x) {					\
	if (!(x)){						\
		printf("WARNING! Assertion [%s] failed.\n" 	\
		"\tfile %s :line %d\n"				\
		"\tfunction [%s]\n",				\
		#x,						\
		__FILE__,					\
		__LINE__,					\
		__PRETTY_FUNCTION__);				\
		return;						\
	};							\
}

#define DBC_RETVAL_IF_FAIL(x,val) {				\
	if (!(x)){						\
		printf("WARNING! Assertion [%s] failed.\n" 	\
		"\tfile %s :line %d\n"				\
		"\tfunction [%s]\n",				\
		#x,						\
		__FILE__,					\
		__LINE__,					\
		__PRETTY_FUNCTION__);				\
		return val;					\
	};							\
}
#else

#define DBC_RETURN_IF_FAIL(x) {}
#define DBC_RETVAL_IF_FAIL(x,val) {}

#endif

#endif
