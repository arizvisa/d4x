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
