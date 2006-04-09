#ifndef _D4X_THEMES_HEADER_
#define _D4X_THEMES_HEADER_
#include <gtk/gtk.h>
#include <string>
#include <map>

GdkPixbuf *pixbuf_from_theme(const std::string &path,const char **default_xpm);

namespace d4x{
	enum THEMABLE_ICONS_IDS{
		LRT_OK,
		LRT_SEND,
		LRT_RECEIVE,
		LRT_ERROR,
		LRT_WARNING,

		OMB_OFFLINE,
		OMB_ONLINE,

		PBM_ONLY_TEXT,
		PBM_MONOLITH,
		PBM_SEGMENTS,
		
		
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
		struct Themable{
			virtual ~Themable(){};
			virtual void change()=0;
		};

		struct Pixbuf:Themable{
			GdkPixbuf *pixbuf;
			std::string ThemePath;
			char **DefaultXPM;
			void change();
			Pixbuf(char **def,const std::string &p);
		};
		
		struct Image:Pixbuf{
			GtkImage *img;
			void change();
			Image(char **def,const std::string &p);
		};

		struct SlaveImage:Themable{
			int pixbuf_id;
			GtkImage *img;
			void change();
			void reinit(int newid);
			SlaveImage(int id);
		};

		Theme();
		~Theme();
		void reload();
		inline int monitor(Themable *item){
			Active[LastUnique]=item;
			return(LastUnique++);
		};
		GdkPixbuf *get_pixbuf(int id){
			std::map<int,Themable *>::iterator it=Active.find(id);
			if (it!=Active.end()){
				return ((Pixbuf*)(it->second))->pixbuf;
			};
			return 0;
		};
	private:
		int LastUnique;
		std::map <int,Themable *> Active;
		void init_lod();
	};

	extern Theme *CUR_THEME;
};

#endif
