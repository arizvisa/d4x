#include <gtk/gtk.h>
#include "../var.h"
#include "../xml.h"
#include "../locstr.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

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
