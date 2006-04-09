#include "../var.h"
#include "../xml.h"
#include "../locstr.h"
#include "../main.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "themes.h"
#include <algorithm>

GdkPixbuf *pixbuf_from_theme(const std::string &path,const char **default_xpm){
	char *themeicon=d4x_xml_find_obj_value(D4X_THEME_DATA,path);
	char *iconfile=NULL;
	if (themeicon)
		iconfile=sum_strings(CFG.THEMES_DIR,"/",themeicon,NULL);
	GdkPixbuf *pixbuf=NULL;
	GError *error=NULL;
	if (iconfile && (pixbuf=gdk_pixbuf_new_from_file(iconfile,&error))){
		if (error) g_error_free(error);
		delete[] iconfile;
		return(pixbuf);
	};
	delete[] iconfile;
	return(gdk_pixbuf_new_from_xpm_data(default_xpm));
};


using namespace d4x;

Theme *d4x::CUR_THEME=0;

void Theme::Pixbuf::change(){
	gdk_pixbuf_unref(pixbuf);
	pixbuf=pixbuf_from_theme(ThemePath.c_str(),(const char**)DefaultXPM);
};

Theme::Pixbuf::Pixbuf(char **def,const std::string &p):DefaultXPM(def),ThemePath(p){
	pixbuf=pixbuf_from_theme(ThemePath.c_str(),(const char**)DefaultXPM);
};

void Theme::Image::change(){
	Pixbuf::change();
	gtk_image_set_from_pixbuf(img,pixbuf);
};

Theme::Image::Image(char **def,const std::string &p):Theme::Pixbuf(def,p){
	img = GTK_IMAGE(gtk_image_new());
	gtk_image_set_from_pixbuf(img,pixbuf);
};

GtkImage *img;

Theme::SlaveImage::SlaveImage(int id):pixbuf_id(id){
	img = GTK_IMAGE(gtk_image_new());
	gtk_image_set_from_pixbuf(img,CUR_THEME->get_pixbuf(pixbuf_id));
};

void Theme::SlaveImage::change(){
	gtk_image_set_from_pixbuf(img,CUR_THEME->get_pixbuf(pixbuf_id));
};

void Theme::SlaveImage::reinit(int newid){
	pixbuf_id=newid;
	change();
};



void Theme::init_lod(){
#include "pixmaps/wait_xpm.xpm"
#include "pixmaps/run_xpm.xpm"
#include "pixmaps/run1.xpm"
#include "pixmaps/run2.xpm"
#include "pixmaps/run3.xpm"
#include "pixmaps/run4.xpm"
#include "pixmaps/run5.xpm"
#include "pixmaps/run6.xpm"
#include "pixmaps/run7.xpm"
#include "pixmaps/run8.xpm"
#include "pixmaps/run_bad.xpm"
#include "pixmaps/run_bad1.xpm"
#include "pixmaps/run_bad2.xpm"
#include "pixmaps/run_bad3.xpm"
#include "pixmaps/run_bad4.xpm"
#include "pixmaps/run_bad5.xpm"
#include "pixmaps/run_bad6.xpm"
#include "pixmaps/run_bad7.xpm"
#include "pixmaps/run_bad8.xpm"
#include "pixmaps/run_part.xpm"
#include "pixmaps/run_part1.xpm"
#include "pixmaps/run_part2.xpm"
#include "pixmaps/run_part3.xpm"
#include "pixmaps/run_part4.xpm"
#include "pixmaps/run_part5.xpm"
#include "pixmaps/run_part6.xpm"
#include "pixmaps/run_part7.xpm"
#include "pixmaps/run_part8.xpm"
#include "pixmaps/stop_xpm.xpm"
#include "pixmaps/stop_wait.xpm"
#include "pixmaps/paused.xpm"
#include "pixmaps/complete.xpm"
#include "pixmaps/size.xpm"
	char *xml_names[]={
		"waitpix",
		"failedpix",
		"stopwaitpix",
		"runpix",
		"runpix1",
		"runpix2",
		"runpix3",
		"runpix4",
		"runpix5",
		"runpix6",
		"runpix7",
		"runpix8",
		"runbadpix",
		"runbadpix1",
		"runbadpix2",
		"runbadpix3",
		"runbadpix4",
		"runbadpix5",
		"runbadpix6",
		"runbadpix7",
		"runbadpix8",
		"runpartpix",
		"runpartpix1",
		"runpartpix2",
		"runpartpix3",
		"runpartpix4",
		"runpartpix5",
		"runpartpix6",
		"runpartpix7",
		"runpartpix8",
		"completepix",
		"pausedpix",
		"sizepix"
	};
	char **xpm_table[]={
		wait_xpm,
		stop_xpm,
		stop_wait_xpm,
		run_xpm,
		run1_xpm,
		run2_xpm,
		run3_xpm,
		run4_xpm,
		run5_xpm,
		run6_xpm,
		run7_xpm,
		run8_xpm,
		run_bad_xpm,
		run_bad1_xpm,
		run_bad2_xpm,
		run_bad3_xpm,
		run_bad4_xpm,
		run_bad5_xpm,
		run_bad6_xpm,
		run_bad7_xpm,
		run_bad8_xpm,
		run_part_xpm,
		run_part1_xpm,
		run_part2_xpm,
		run_part3_xpm,
		run_part4_xpm,
		run_part5_xpm,
		run_part6_xpm,
		run_part7_xpm,
		run_part8_xpm,
		complete_xpm,
		paused_xpm,
		size_xpm
	};
	d4xXmlObject *xmlobj=d4x_xml_find_obj(D4X_THEME_DATA,"queue");
	std::string tmp="queue ";
	for (unsigned int i=0;i<sizeof(xpm_table)/sizeof(char**);i++){
		Active[LPE_WAIT+i]=new Pixbuf(xpm_table[i],tmp+xml_names[i]+">file");
	};
	/* we will use these pixmaps many times */
};


Theme::Theme():LastUnique(LPE_UNKNOWN){
#include "pixmaps2/ok.xpm"
#include "pixmaps2/from_server.xpm"
#include "pixmaps2/to_server.xpm"
#include "pixmaps2/error.xpm"
#include "pixmaps2/warning.xpm"
	
#include "pixmaps2/offline.xpm"
#include "pixmaps2/offline1.xpm"

#include "pixmaps/percent1.xpm"
#include "pixmaps/percent2.xpm"
#include "pixmaps/percent3.xpm"

	Active[LRT_OK]=new Pixbuf(ok_xpm,"log ok>file");
	Active[LRT_SEND]=new Pixbuf(to_server_xpm,"log send>file");
	Active[LRT_RECEIVE]=new Pixbuf(from_server_xpm,"log receiv>file");
	Active[LRT_ERROR]=new Pixbuf(error_xpm,"log error>file");
	Active[LRT_WARNING]=new Pixbuf(warning_xpm,"log warning>file");

	Active[OMB_ONLINE]=new Pixbuf(offline_xpm,"main online>file");
	Active[OMB_OFFLINE]=new Pixbuf(offline1_xpm,"main offline>file");

	Active[PBM_ONLY_TEXT]=new Pixbuf(percent1_xpm,"buttonsbar progress1>file");
	Active[PBM_MONOLITH]=new Pixbuf(percent2_xpm,"buttonsbar progress2>file");
	Active[PBM_SEGMENTS]=new Pixbuf(percent3_xpm,"buttonsbar progress3>file");

	init_lod();
};

Theme::~Theme(){
};

static void lod_all_redraw(d4xDownloadQueue *q,void *a){
	q->qv.redraw_icons();
};

static void themable_reload(const std::pair<int,Theme::Themable *> &p){
	p.second->change();
};

void Theme::reload(){
	std::for_each(Active.begin(),Active.end(),themable_reload);
	d4x_qtree_for_each(lod_all_redraw,NULL);
};

