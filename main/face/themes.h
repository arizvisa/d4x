#ifndef _D4X_THEMES_HEADER_
#define _D4X_THEMES_HEADER_

GdkPixbuf *pixbuf_from_theme(char *path,const char **default_xpm);

namespace d4x{
	enum LOG_ROW_TYPES{
		LRT_OK,
		LRT_SEND,
		LRT_RECEIVE,
		LRT_ERROR,
		LRT_WARNING,
		LRT_LAST
	};
	
	enum LOD_PIX_ENUM{
		LPE_WAIT,
		LPE_STOP,
		LPE_STOP_WAIT,
		LPE_RUN,
		LPE_RUN1,
		LPE_RUN2,
		LPE_RUN3,
		LPE_RUN4,
		LPE_RUN5,
		LPE_RUN6,
		LPE_RUN7,
		LPE_RUN8,
		LPE_RUN_BAD,
		LPE_RUN_BAD1,
		LPE_RUN_BAD2,
		LPE_RUN_BAD3,
		LPE_RUN_BAD4,
		LPE_RUN_BAD5,
		LPE_RUN_BAD6,
		LPE_RUN_BAD7,
		LPE_RUN_BAD8,
		LPE_RUN_PART,
		LPE_RUN_PART1,
		LPE_RUN_PART2,
		LPE_RUN_PART3,
		LPE_RUN_PART4,
		LPE_RUN_PART5,
		LPE_RUN_PART6,
		LPE_RUN_PART7,
		LPE_RUN_PART8,
		LPE_COMPLETE,
		LPE_PAUSE,
		LPE_SIZE,
		LPE_UNKNOWN
	};
	
	struct Theme{
		GdkPixbuf *logpix[LRT_LAST];
		GdkPixbuf *lodpix[LPE_UNKNOWN];
		
		Theme();
		~Theme();
		void reload();
	private:
		void init_lod();
	};

	extern Theme *CUR_THEME;
};

#endif
