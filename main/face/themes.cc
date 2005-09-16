#include <gtk/gtk.h>
#include "../var.h"
#include "../xml.h"
#include "../locstr.h"
#include "../main.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "themes.h"

GdkPixbuf *pixbuf_from_theme(char *path,const char **default_xpm){
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
	for (unsigned int i=0;i<sizeof(xpm_table)/sizeof(char*);i++){
		char *file=NULL;
		d4xXmlObject *icon=xmlobj?xmlobj->find_obj(xml_names[i]):NULL;
		d4xXmlField *fld=icon?icon->get_attr("file"):NULL;
		if (fld){
			file=sum_strings(CFG.THEMES_DIR,"/",fld->value.get(),NULL);
		};
		GdkPixbuf *pixbuf;
		GError *error=NULL;
		if (file && (pixbuf=gdk_pixbuf_new_from_file(file,&error))){
			lodpix[i]=pixbuf;
		}else
			lodpix[i]=gdk_pixbuf_new_from_xpm_data((const char **)xpm_table[i]);
		if (error) g_error_free(error);
		if (file) delete[] file;
	};
	/* we will use these pixmaps many times */
};


Theme::Theme(){
#include "pixmaps2/ok.xpm"
#include "pixmaps2/from_server.xpm"
#include "pixmaps2/to_server.xpm"
#include "pixmaps2/error.xpm"
#include "pixmaps2/warning.xpm"
	logpix[LRT_OK]=pixbuf_from_theme("log ok>file",(const char **)ok_xpm);
	logpix[LRT_SEND]=pixbuf_from_theme("log send>file",(const char **)to_server_xpm);
	logpix[LRT_RECEIVE]=pixbuf_from_theme("log reciev>file",(const char **)from_server_xpm);
	logpix[LRT_ERROR]=pixbuf_from_theme("log error>file",(const char **)error_xpm);
	logpix[LRT_WARNING]=pixbuf_from_theme("log warning>file",(const char **)warning_xpm);

	init_lod();
};

Theme::~Theme(){
};

static void lod_all_redraw(d4xDownloadQueue *q,void *a){
	q->qv.redraw_icons();
};

void Theme::reload(){
	for (int i=0;i<LPE_UNKNOWN;i++){
		gdk_pixbuf_unref(lodpix[i]);
		lodpix[i]=NULL;
	};
	init_lod();
	d4x_qtree_for_each(lod_all_redraw,NULL);
};



