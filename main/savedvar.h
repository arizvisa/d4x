#ifndef __DOWLOADER_SAVED_VARS__
#define __DOWLOADER_SAVED_VARS__

struct tSavedVar{
	char *name;
	int type;
	void *where;
};

enum SV_TYPES{
	SV_TYPE_INT,
	SV_TYPE_PSTR,
	SV_TYPE_SPLIT,
	SV_TYPE_CFG,
	SV_TYPE_URL,
	SV_TYPE_TIME,
	SV_TYPE_END
};

#endif
