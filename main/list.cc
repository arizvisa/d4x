;/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2000 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include "../dlist.h"
#include "../main.h"
#include "../var.h"
#include "../savelog.h"
#include "../config.h"
#include "../locstr.h"
#include "../ntlocale.h"
#include "log.h"
#include "addd.h"
#include "list.h"
#include "prefs.h"
#include "buttons.h"
#include "about.h"
#include "graph.h"
#include "edit.h"
#include "lmenu.h"
#include "saveload.h"
#include "limface.h"
#include "misc.h"
#include "dndtrash.h"
#include "passface.h"
#include "colors.h"


GtkWidget *MainMenu;
GtkAdjustment *ProgressBarValues;
GtkWidget *ProgressOfDownload;
GtkWidget *MainStatusBar,*ReadedBytesStatusBar;
GtkWidget *MainWindow=(GtkWidget *)NULL;
GtkWidget *MainHBox;
GtkWidget *ContainerForCList=(GtkWidget *)NULL;
GdkGC *MainWindowGC=(GdkGC *)NULL;
GtkWidget *BoxForGraph;
GtkItemFactory *main_menu_item_factory=NULL;
GtkWidget *MainLogList,*MAIN_PANED=(GtkWidget *)NULL;
int main_log_mask;
unsigned int ScrollShift[2];
int mainwin_title_state;
gfloat main_log_value;
GtkAdjustment *main_log_adj;

GtkItemFactory *list_menu_itemfact;

tDialogWidget *AskDelete=(tDialogWidget *)NULL;
tDialogWidget *AskDeleteCompleted=(tDialogWidget *)NULL;
tDialogWidget *AskDeleteFataled=(tDialogWidget *)NULL;
tDialogWidget *AskExit=(tDialogWidget *)NULL;

tFaceLimits *FaceForLimits=(tFaceLimits *)NULL;
tFacePass *FaceForPasswords=(tFacePass *)NULL;

gint StatusBarContext,RBStatusBarContext;
int MainTimer,LogsTimer,GraphTimer,ListTimer;
int SAVE_LIST_INTERVAL,EXIT_COMPLETE_INTERVAL;
pthread_mutex_t MAIN_GTK_MUTEX=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

int FirstConfigureEvent;
int UpdateTitleCycle=0;
int MAIN_PANED_HEIGHT=0;
char *OLD_CLIPBOARD_CONTENT=NULL;
char *LOAD_ACCELERATORS[6]={(char *)NULL,(char *)NULL,(char *)NULL,(char*)NULL,(char *)NULL};
enum{
	ROLL_STAT=0,
	ROLL_INFO,
	ROLL_LAST
};

enum MAIN_MENU_ENUM{
	MM_FILE, MM_FILE_SAVE, MM_FILE_LOAD, MM_FILE_TXT, MM_FILE_NEW, MM_FILE_PASTE, MM_FILE_EXIT, MM_FILE_SEP,
	MM_DOWNLOAD, MM_DOWNLOAD_LOG, MM_DOWNLOAD_STOP, MM_DOWNLOAD_EDIT, MM_DOWNLOAD_DEL, MM_DOWNLOAD_RUN, MM_DOWNLOAD_DEL_C,
	MM_DOWNLOAD_DEL_F,MM_DOWNLOAD_RERUN, MM_DOWNLOAD_UNSELECT_ALL,MM_DOWNLOAD_SELECT_ALL ,MM_DOWNLOAD_INVERT, MM_DOWNLOAD_SEP,
	MM_OPTIONS, MM_OPTIONS_LIMITS, MM_OPTIONS_PASSWORDS, MM_OPTIONS_COMMON,
	MM_OPTIONS_SPEED, MM_OPTIONS_SPEED_1, MM_OPTIONS_SPEED_2, MM_OPTIONS_SPEED_3,
	MM_OPTIONS_BUTTONS, MM_OPTIONS_BUTTONS_ADD, MM_OPTIONS_BUTTONS_MAN, MM_OPTIONS_BUTTONS_SPEED, MM_OPTIONS_BUTTONS_MISC,
	MM_HELP, MM_HELP_ABOUT
};

char *main_menu_inames[]={
	"/_File",
	"/File/_Save List",
	"/File/_Load List",
	"/File/Find links in file",
	"/File/_New Download",
	"/File/_Paste Download",
	"/File/Exit",
	"/File/sep1",
	"/_Download",
	"/Download/View _Log",
	"/Download/_Stop downloads",
	"/Download/Edit download",
	"/Download/_Delete downloads",
	"/Download/Continue downloads",
	"/Download/Delete completed",
	"/Download/Delete failed",
	"/Download/Rerun failed",
	"/Download/Unselect all",
	"/Download/Select all",
	"/Download/Invert selection",
	"/Download/-",
	"/_Options",
	"/Options/Limitations",
	"/Options/Passwords",
	"/Options/General",
	"/Options/Speed",
	"/Options/Speed/Lowest",
	"/Options/Speed/Middle",
	"/Options/Speed/Unlimited",
	"/Options/Buttons",
	"/Options/Buttons/Add buttons",
	"/Options/Buttons/Manipulating",
	"/Options/Buttons/Speed buttons",
	"/Options/Buttons/Misc buttons",
	"/_Help",
	"/_Help/About"
};


static void open_passwords_window(...) {
	FaceForPasswords->init();
};

static void open_limits_window(...) {
	FaceForLimits->init();
};

void util_item_factory_popup(GtkItemFactory *ifactory,guint x, guint y,guint mouse_button,guint32 time) {
	static GQuark quark_user_menu_pos=0;
	struct Pos {
		gint x;
		gint y;
	}
	*pos;

	if(!quark_user_menu_pos)
		quark_user_menu_pos=g_quark_from_static_string("user_menu_pos");
	pos=(Pos *)gtk_object_get_data_by_id(GTK_OBJECT(ifactory),quark_user_menu_pos);
	if(!pos) {
		pos=(Pos *)g_malloc0(sizeof(struct Pos));
		gtk_object_set_data_by_id_full(GTK_OBJECT(ifactory->widget),quark_user_menu_pos,pos,g_free);
	}
	pos->x=x;
	pos->y=y;
	gtk_menu_popup(GTK_MENU(ifactory->widget),
		       (GtkWidget *)NULL,
		       (GtkWidget *)NULL,
		       (GtkMenuPositionFunc)NULL,
		       pos,
		       mouse_button,time);
};

void _rerun_failed_downloads(){
	aa.rerun_failed();
};

void main_menu_speed_calback(gpointer data,guint action,GtkWidget *widget){
	if (action>0 && action<4){
		CFG.SPEED_LIMIT=action;
		set_speed_buttons();
	};
};

void main_menu_buttons_calback(gpointer data,guint action,GtkWidget *widget){
	GtkWidget *menu_item=NULL;
	switch (action){
	case 1:
		menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_OPTIONS_BUTTONS_ADD]));
		CFG.BUTTONS_ADD=GTK_CHECK_MENU_ITEM(menu_item)->active;
		break;
	case 2:
		menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_OPTIONS_BUTTONS_MAN]));
		CFG.BUTTONS_MAN=GTK_CHECK_MENU_ITEM(menu_item)->active;
		break;
	case 3:
		menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_OPTIONS_BUTTONS_SPEED]));
		CFG.BUTTONS_SPEED=GTK_CHECK_MENU_ITEM(menu_item)->active;
		break;
	case 4:
		menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_OPTIONS_BUTTONS_MISC]));
		CFG.BUTTONS_MISC=GTK_CHECK_MENU_ITEM(menu_item)->active;
		break;
	};
	buttons_cfg_init();
};

void main_menu_buttons_prepare(){
	GtkWidget *menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_OPTIONS_BUTTONS_ADD]));
	GTK_CHECK_MENU_ITEM(menu_item)->active=CFG.BUTTONS_ADD?TRUE:FALSE;
	if (menu_item)
		menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_OPTIONS_BUTTONS_SPEED]));
	GTK_CHECK_MENU_ITEM(menu_item)->active=CFG.BUTTONS_SPEED?TRUE:FALSE;
	if (menu_item)
		menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_OPTIONS_BUTTONS_MISC]));
	GTK_CHECK_MENU_ITEM(menu_item)->active=CFG.BUTTONS_MISC?TRUE:FALSE;
	if (menu_item)
		menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_OPTIONS_BUTTONS_MAN]));
	GTK_CHECK_MENU_ITEM(menu_item)->active=CFG.BUTTONS_MAN?TRUE:FALSE;
};

void main_menu_speed_prepare(){
	GtkWidget *menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_OPTIONS_SPEED_1]));
	if (menu_item)
		GTK_CHECK_MENU_ITEM(menu_item)->active=CFG.SPEED_LIMIT==1?TRUE:FALSE;
	menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_OPTIONS_SPEED_2]));
	if (menu_item)
		GTK_CHECK_MENU_ITEM(menu_item)->active=CFG.SPEED_LIMIT==2?TRUE:FALSE;
	menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_OPTIONS_SPEED_3]));
	if (menu_item)
		GTK_CHECK_MENU_ITEM(menu_item)->active=CFG.SPEED_LIMIT==3?TRUE:FALSE;
};

void load_accelerated(gpointer *p,gint realnum){
	tString *str=ALL_HISTORIES[SAVE_HISTORY]->last();
	int num=realnum-128;
	for (int i=0;i<=num;i++){
		if (str==NULL) return;
		if (i==num) read_list_from_file(str->body);
		str=ALL_HISTORIES[SAVE_HISTORY]->next();
	};
};

static void _remove_underscore(char *where){
	char *p,*q;
	p=q=where;
	while (*p){
		if (*p!='_')
			*q++ = *p;
		p++;
	};
	*q=0;
};

void init_load_accelerators(){
	tString *str=ALL_HISTORIES[SAVE_HISTORY]->last();
	char *path=copy_string(_(main_menu_inames[MM_FILE_SEP]));
	char *sep=index(path+1,'/');
	if (LOAD_ACCELERATORS[5]==NULL){
		LOAD_ACCELERATORS[5]=copy_string(_(main_menu_inames[MM_FILE_EXIT]));
		_remove_underscore(LOAD_ACCELERATORS[5]);
	};
	gtk_item_factory_delete_item(main_menu_item_factory,LOAD_ACCELERATORS[5]);
	if (LOAD_ACCELERATORS[4]==NULL){
		LOAD_ACCELERATORS[4]=sum_strings(_(main_menu_inames[MM_FILE_SEP]),"1",NULL);
	}else{
		gtk_item_factory_delete_item(main_menu_item_factory,
					     LOAD_ACCELERATORS[4]);
	};
	for (int j=0;j<3;j++)
		if (LOAD_ACCELERATORS[j]){
			gtk_item_factory_delete_item(main_menu_item_factory,
						     LOAD_ACCELERATORS[j]);
			delete(LOAD_ACCELERATORS[j]);
		};
	if (sep){
		*sep=0;
		for (int i=0;i<3;i++){
			if (str==NULL) break;
			GtkItemFactoryEntry tmp;
			sep=rindex(str->body,'/');
			char data[MAX_LEN];
			g_snprintf(data,MAX_LEN,"%i",i+1);
			if (sep)
				tmp.path=sum_strings(path,"/",data,". ",sep+1,NULL);
			else
				tmp.path=sum_strings(path,"/",data,". ",str->body,NULL);
			tmp.accelerator=sum_strings("<control>",data,NULL);
			tmp.callback=(GtkItemFactoryCallback)load_accelerated;
			tmp.callback_action=i+128;
			tmp.item_type="<Item>";
			LOAD_ACCELERATORS[i]=tmp.path;
			tmp.path=escape_char(tmp.path,'_','_');
			gtk_item_factory_create_item(main_menu_item_factory,
						     &tmp,
						     NULL,1);
			delete(tmp.path);
			delete(tmp.accelerator);
			/*adding tooltip*/
			GtkWidget *menu_item=gtk_item_factory_get_widget_by_action(main_menu_item_factory,
										   tmp.callback_action);
			if (menu_item){
				GtkTooltips *tooltip=gtk_tooltips_new();
				gtk_tooltips_force_window(tooltip);
				GtkStyle *current_style=gtk_style_copy(gtk_widget_get_style(tooltip->tip_window));
				current_style->bg[GTK_STATE_NORMAL] = LYELLOW;
				gdk_font_unref(current_style->font);
				current_style->font = MainWindow->style->font;
				gtk_widget_set_style(tooltip->tip_window, current_style);
				gtk_tooltips_set_tip(tooltip,menu_item,str->body,(const gchar *)NULL);
				gtk_tooltips_enable(tooltip);
			};
			/* modify path for deleting it successfully
			 */
			_remove_underscore(LOAD_ACCELERATORS[i]);
			str=ALL_HISTORIES[SAVE_HISTORY]->next();
		};
	};
	delete(path);
	GtkItemFactoryEntry sep_item={LOAD_ACCELERATORS[4],
				       (gchar *)NULL,
				       (GtkItemFactoryCallback)NULL,
				       0,
				       "<Separator>"};
	GtkItemFactoryEntry exit_item={_(main_menu_inames[MM_FILE_EXIT]),
				       "<alt>X",
				       (GtkItemFactoryCallback)ask_exit,
				       0,
				       (gchar *)NULL};
	gtk_item_factory_create_item(main_menu_item_factory,
				     &sep_item,NULL,1);
	gtk_item_factory_create_item(main_menu_item_factory,
				     &exit_item,NULL,1);
};

static gint main_menu_enable_all(){
	GtkWidget *menu_item;
	menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_DOWNLOAD_DEL_F]));
	if (menu_item)	gtk_widget_set_sensitive(menu_item,TRUE);
	menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_DOWNLOAD_RERUN]));
	if (menu_item)	gtk_widget_set_sensitive(menu_item,TRUE);
	for (int i=MM_DOWNLOAD_LOG;i<=MM_DOWNLOAD_RUN;i++){
		menu_item=gtk_item_factory_get_item_by_action(main_menu_item_factory,
							      i+100);
		if (menu_item) gtk_widget_set_sensitive(menu_item,TRUE);
	};
	for (int i=MM_DOWNLOAD_UNSELECT_ALL;i<=MM_DOWNLOAD_INVERT;i++){
		menu_item=gtk_item_factory_get_item_by_action(main_menu_item_factory,
							      i+100);
		if (menu_item) gtk_widget_set_sensitive(menu_item,TRUE);
	};
	return FALSE;
};

void init_main_menu() {
	GtkItemFactoryEntry menu_items[] = {
		{_(main_menu_inames[MM_FILE]),		(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<Branch>"},
		{_(main_menu_inames[MM_FILE_SAVE]),	"<control>S",	(GtkItemFactoryCallback)init_save_list,			0, (gchar *)NULL},
		{_(main_menu_inames[MM_FILE_LOAD]),	"<control>L",	(GtkItemFactoryCallback)init_load_list,			0, (gchar *)NULL},
		{_(main_menu_inames[MM_FILE_TXT]),	"<control><alt>L",	(GtkItemFactoryCallback)init_load_txt_list,	0, (gchar *)NULL},
		{_(main_menu_inames[MM_FILE_SEP]),	(gchar *)NULL,  (GtkItemFactoryCallback)NULL,	0, "<Separator>"},
		{_(main_menu_inames[MM_FILE_NEW]),	"<control>N",	(GtkItemFactoryCallback)init_add_window,		0, (gchar *)NULL},
		{_(main_menu_inames[MM_FILE_PASTE]),	"<control>P",	(GtkItemFactoryCallback)init_add_clipboard_window,	0, (gchar *)NULL},
		{_(main_menu_inames[MM_FILE_SEP]),	(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<Separator>"},
		{_(main_menu_inames[MM_FILE_EXIT]),	"<alt>X",	(GtkItemFactoryCallback)ask_exit,			0, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD]),     	(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<Branch>"},
		{_(main_menu_inames[MM_DOWNLOAD_LOG]), 	(gchar *)NULL,	(GtkItemFactoryCallback)list_of_downloads_open_logs,	100+MM_DOWNLOAD_LOG, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_STOP]),	"<alt>S",	(GtkItemFactoryCallback)stop_downloads,		        100+MM_DOWNLOAD_STOP, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_EDIT]),	"<alt>E",	(GtkItemFactoryCallback)open_edit_for_selected,		100+MM_DOWNLOAD_EDIT, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_DEL]),	"<alt>C",	(GtkItemFactoryCallback)ask_delete_download,		100+MM_DOWNLOAD_DEL, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_RUN]),	"<alt>A",	(GtkItemFactoryCallback)continue_downloads,		100+MM_DOWNLOAD_RUN, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_SEP]),(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<Separator>"},
		{_(main_menu_inames[MM_DOWNLOAD_DEL_C]),(gchar *)NULL,	(GtkItemFactoryCallback)ask_delete_completed_downloads,	0, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_DEL_F]),(gchar *)NULL,	(GtkItemFactoryCallback)ask_delete_fataled_downloads,	0, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_RERUN]),(gchar *)NULL,	(GtkItemFactoryCallback)_rerun_failed_downloads,	0, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_SEP]),(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<Separator>"},
		{_(main_menu_inames[MM_DOWNLOAD_UNSELECT_ALL]),(gchar *)NULL, (GtkItemFactoryCallback)list_of_downloads_unselect_all,	100+MM_DOWNLOAD_UNSELECT_ALL, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_SELECT_ALL]),(gchar *)NULL, (GtkItemFactoryCallback)list_of_downloads_select_all,	100+MM_DOWNLOAD_SELECT_ALL, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_INVERT]),(gchar *)NULL, (GtkItemFactoryCallback)list_of_downloads_invert_selection,	100+MM_DOWNLOAD_INVERT, (gchar *)NULL},
		{_(main_menu_inames[MM_OPTIONS]),	(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<Branch>"},
		{_(main_menu_inames[MM_OPTIONS_LIMITS]),(gchar *)NULL,	(GtkItemFactoryCallback)open_limits_window,		0, (gchar *)NULL},
		{_(main_menu_inames[MM_OPTIONS_PASSWORDS]),(gchar *)NULL,	(GtkItemFactoryCallback)open_passwords_window,		0, (gchar *)NULL},
		{_(main_menu_inames[MM_OPTIONS_COMMON]),"<control>C",	(GtkItemFactoryCallback)init_options_window,		0, (gchar *)NULL},
		{_(main_menu_inames[MM_OPTIONS_SPEED]),	(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<Branch>"},
		{_(main_menu_inames[MM_OPTIONS_SPEED_1]),(gchar *)NULL,	(GtkItemFactoryCallback)main_menu_speed_calback,	1, "<RadioItem>"},
		{_(main_menu_inames[MM_OPTIONS_SPEED_2]),(gchar *)NULL,	(GtkItemFactoryCallback)main_menu_speed_calback,	2, _(main_menu_inames[MM_OPTIONS_SPEED_1])},
		{_(main_menu_inames[MM_OPTIONS_SPEED_3]),(gchar *)NULL,	(GtkItemFactoryCallback)main_menu_speed_calback,	3, _(main_menu_inames[MM_OPTIONS_SPEED_2])},
		{_(main_menu_inames[MM_OPTIONS_BUTTONS]),(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<Branch>"},
		{_(main_menu_inames[MM_OPTIONS_BUTTONS_ADD]),	(gchar *)NULL,	(GtkItemFactoryCallback)main_menu_buttons_calback,	1, "<ToggleItem>"},
		{_(main_menu_inames[MM_OPTIONS_BUTTONS_MAN]),	(gchar *)NULL,	(GtkItemFactoryCallback)main_menu_buttons_calback,	2, "<ToggleItem>"},
		{_(main_menu_inames[MM_OPTIONS_BUTTONS_SPEED]),	(gchar *)NULL,	(GtkItemFactoryCallback)main_menu_buttons_calback,	3, "<ToggleItem>"},
		{_(main_menu_inames[MM_OPTIONS_BUTTONS_MISC]),	(gchar *)NULL,	(GtkItemFactoryCallback)main_menu_buttons_calback,	4, "<ToggleItem>"},
		{_(main_menu_inames[MM_HELP]),		(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<LastBranch>"},
		{_(main_menu_inames[MM_HELP_ABOUT]),	(gchar *)NULL,	(GtkItemFactoryCallback)init_about_window,		0, (gchar *)NULL},
	};
	int nmenu_items = sizeof(menu_items) / sizeof(menu_items[0]);
	GtkAccelGroup *accel_group = gtk_accel_group_new();
	main_menu_item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>",accel_group);
	gtk_item_factory_create_items(main_menu_item_factory, nmenu_items, menu_items, NULL);
	init_load_accelerators();
	MainMenu= gtk_item_factory_get_widget(main_menu_item_factory, "<main>");
	gtk_accel_group_attach(accel_group,GTK_OBJECT(MainWindow));
	gtk_signal_connect (GTK_OBJECT (MainMenu),
			    "button_press_event",
			    GTK_SIGNAL_FUNC (main_menu_prepare),
			    NULL);
	/* to avoid losing accelerators key */
	gtk_signal_connect (GTK_OBJECT (MainMenu),
			    "deactivate",
			    GTK_SIGNAL_FUNC (main_menu_enable_all),
			    NULL);
	main_menu_speed_prepare();
	main_menu_buttons_prepare();
};

void main_menu_completed_empty(){
	GtkWidget *menu_item;
	menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_DOWNLOAD_DEL_C]));
	if (menu_item)
		gtk_widget_set_sensitive(menu_item,FALSE);
	gtk_widget_set_sensitive(ListMenuArray[LM_DELC],FALSE);
	gtk_widget_set_sensitive(buttons_array[BUTTON_DEL_COMPLETED],FALSE);
	dnd_trash_set_del_completed(FALSE);
};

void main_menu_completed_nonempty(){
	GtkWidget *menu_item;
	menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_DOWNLOAD_DEL_C]));
	if (menu_item)
		gtk_widget_set_sensitive(menu_item,TRUE);
	gtk_widget_set_sensitive(ListMenuArray[LM_DELC],TRUE);
	gtk_widget_set_sensitive(buttons_array[BUTTON_DEL_COMPLETED],TRUE);
	dnd_trash_set_del_completed(TRUE);
};

void main_menu_failed_empty(){
	GtkWidget *menu_item;
	menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_DOWNLOAD_DEL_F]));
	if (menu_item)
		gtk_widget_set_sensitive(menu_item,FALSE);
	menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_DOWNLOAD_RERUN]));
	if (menu_item)
		gtk_widget_set_sensitive(menu_item,FALSE);
	gtk_widget_set_sensitive(ListMenuArray[LM_DELF],FALSE);
};

void main_menu_failed_nonempty(){
	GtkWidget *menu_item;
	menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_DOWNLOAD_DEL_F]));
	if (menu_item)
		gtk_widget_set_sensitive(menu_item,TRUE);
	menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_DOWNLOAD_RERUN]));
	if (menu_item)
		gtk_widget_set_sensitive(menu_item,TRUE);
	gtk_widget_set_sensitive(ListMenuArray[LM_DELF],TRUE);
};

gint main_menu_prepare(){
	GtkWidget *menu_item;
	if (GTK_CLIST(ListOfDownloads)->selection){
		for (int i=MM_DOWNLOAD_LOG;i<=MM_DOWNLOAD_RUN;i++){
			menu_item=gtk_item_factory_get_item_by_action(main_menu_item_factory,
								      i+100);
			if (menu_item) gtk_widget_set_sensitive(menu_item,TRUE);
		};
	}else{
		for (int i=MM_DOWNLOAD_LOG;i<=MM_DOWNLOAD_RUN;i++){
			menu_item=gtk_item_factory_get_item_by_action(main_menu_item_factory,
								      i+100);
			if (menu_item) gtk_widget_set_sensitive(menu_item,FALSE);
		};
	};
	if (GTK_CLIST(ListOfDownloads)->rows==0){
		for (int i=MM_DOWNLOAD_UNSELECT_ALL;i<=MM_DOWNLOAD_INVERT;i++){
			menu_item=gtk_item_factory_get_item_by_action(main_menu_item_factory,
								      i+100);
			if (menu_item) gtk_widget_set_sensitive(menu_item,FALSE);
		};
	}else{
		for (int i=MM_DOWNLOAD_UNSELECT_ALL;i<=MM_DOWNLOAD_INVERT;i++){
			menu_item=gtk_item_factory_get_item_by_action(main_menu_item_factory,
								      i+100);
			if (menu_item) gtk_widget_set_sensitive(menu_item,TRUE);
		};
	};
	return FALSE;
};

void my_main_quit(...) {
	if (CFG.WITHOUT_FACE==0){
		gtk_timeout_remove(MainTimer);
		gtk_timeout_remove(LogsTimer);
		gtk_timeout_remove(GraphTimer);
	};
	save_list();
	save_limits();
	save_passwords(PasswordsForHosts);
	aa.done();
	save_config();
	if (FaceForLimits)
		delete (FaceForLimits);
	if (FaceForPasswords)
		delete (FaceForPasswords);
	if (CFG.WITHOUT_FACE==0){
		dnd_trash_real_destroy();
		delete(list_for_adding);
		options_window_cancel();
		destroy_about_window();
		gtk_widget_destroy(MainWindow);
		if (AskDelete) delete(AskDelete);
		if (AskDeleteCompleted) delete(AskDeleteCompleted);
		if (AskDeleteFataled) delete(AskDeleteFataled);
		if (AskExit) delete(AskExit);
		load_save_list_cancel();
	};
	for (int i=0;i<LAST_HISTORY;i++)
		delete(ALL_HISTORIES[i]);
	if (CFG.WITHOUT_FACE==0)
		gtk_main_quit();
	else{
		aa.run_after_quit();
		exit(0);
	};
};

void set_limit_to_download() {
	tDownload *temp=list_of_downloads_last_selected();
	if (temp)
		FaceForLimits->add(temp->info->host.get(),temp->info->port);
};

void open_edit_for_selected(...) {
	tDownload *temp=list_of_downloads_last_selected();
	init_edit_window(temp);
};

void del_completed_downloads(...) {
	list_of_downloads_freeze();
	aa.del_completed();
	list_of_downloads_unfreeze();
	if (AskDeleteCompleted) AskDeleteCompleted->done();
};

void del_fataled_downloads(...) {
	list_of_downloads_freeze();
	aa.del_fataled();
	list_of_downloads_unfreeze();
	if (AskDeleteFataled) AskDeleteFataled->done();
};

void stop_downloads(...) {
	GList *select=((GtkCList *)ListOfDownloads)->selection;
	while (select) {
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
		                    GTK_CLIST(ListOfDownloads),GPOINTER_TO_INT(select->data));
		aa.stop_download(temp);
		select=select->next;
	};
	prepare_buttons();
};

void ask_exit(...) {
	if (CFG.CONFIRM_EXIT) {
		if (!AskExit) AskExit=new tDialogWidget;
		if (AskExit->init(_("Do you really want to quit?"),_("Quit?")))
			gtk_signal_connect(GTK_OBJECT(AskExit->ok_button),"clicked",GTK_SIGNAL_FUNC(my_main_quit),NULL);
		AskExit->set_modal(MainWindow);
	} else
		my_main_quit();
};


gint ask_exit2() {
	if (CFG.WINDOW_LOWER)// gdk_window_lower(MainWindow->window);
		main_window_iconify();
	else ask_exit();
	return TRUE;
};


void ask_delete_download(...) {
	if (CFG.CONFIRM_DELETE) {
		if (!AskDelete) AskDelete=new tDialogWidget;
		if (AskDelete->init(_("Delete selected downloads?"),_("Delete?")))
			gtk_signal_connect(GTK_OBJECT(AskDelete->ok_button),"clicked",GTK_SIGNAL_FUNC(delete_downloads),NULL);
		AskDelete->set_modal(MainWindow);
	} else
		delete_downloads();
};

void ask_delete_completed_downloads(...) {
	if (CFG.CONFIRM_DELETE_COMPLETED) {
		if (!AskDeleteCompleted) AskDeleteCompleted=new tDialogWidget;
		if (AskDeleteCompleted->init(_("Do you wish delete completed downloads?"),_("Delete?")))
			gtk_signal_connect(GTK_OBJECT(AskDeleteCompleted->ok_button),"clicked",GTK_SIGNAL_FUNC(del_completed_downloads),NULL);
		AskDeleteCompleted->set_modal(MainWindow);
	} else
		del_completed_downloads();
};

void ask_delete_fataled_downloads(...) {
	if (CFG.CONFIRM_DELETE_FATALED) {
		if (!AskDeleteFataled) AskDeleteFataled=new tDialogWidget;
		if (AskDeleteFataled->init(_("Do you wish delete failed downloads?"),_("Delete?")))
			gtk_signal_connect(GTK_OBJECT(AskDeleteFataled->ok_button),"clicked",GTK_SIGNAL_FUNC(del_fataled_downloads),NULL);
		AskDeleteFataled->set_modal(MainWindow);
	} else
		del_completed_downloads();
};

void delete_downloads(...) {
	GList *select=((GtkCList *)ListOfDownloads)->selection;
	while (select) {
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
		                    GTK_CLIST(ListOfDownloads),GPOINTER_TO_INT(select->data));
		aa.delete_download(temp);
		select=select->next;
	};
	list_of_downloads_freeze();
	aa.go_to_delete(); //real deleting from the list
	gtk_clist_unselect_all(GTK_CLIST(ListOfDownloads));
	list_of_downloads_unfreeze();
	if (AskDelete) AskDelete->done();
};

void continue_downloads(...) {
	GList *select=((GtkCList *)ListOfDownloads)->selection;
	while (select) {
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
		                    GTK_CLIST(ListOfDownloads),GPOINTER_TO_INT(select->data));
		aa.continue_download(temp);
		select=select->next;
	};
	prepare_buttons();
};

/* ******************************************************************** */



/* ******************************************************************** */
void init_status_bar() {
	ProgressBarValues = (GtkAdjustment *) gtk_adjustment_new (0, 1, 0 , 0, 0, 0);
	ProgressOfDownload = gtk_progress_bar_new_with_adjustment(ProgressBarValues);
	/* Set the format of the string that can be displayed in the
	 * trough of the progress bar:
	 * %p - percentage
	 * %v - value
	 * %l - lower range value
	 * %u - upper range value */
	gtk_widget_set_usize(ProgressOfDownload,180,-1);
	gtk_progress_set_format_string (GTK_PROGRESS (ProgressOfDownload),
	                                "%p%%(%v/%u)");
	gtk_progress_set_show_text(GTK_PROGRESS(ProgressOfDownload),FALSE);
	MainStatusBar=gtk_statusbar_new();
	ReadedBytesStatusBar=gtk_statusbar_new();
	StatusBarContext=gtk_statusbar_get_context_id(
	                     GTK_STATUSBAR(MainStatusBar),"Main window context");
	RBStatusBarContext=gtk_statusbar_get_context_id(
	                       GTK_STATUSBAR(ReadedBytesStatusBar),"Readed Bytes");
	gtk_statusbar_push(GTK_STATUSBAR(MainStatusBar),StatusBarContext,_("Ready to go ????"));
	gtk_statusbar_push(GTK_STATUSBAR(ReadedBytesStatusBar),RBStatusBarContext,"0");
	gtk_widget_set_usize(ReadedBytesStatusBar,150,-1);
};


void update_progress_bar() {
	tDownload *temp=list_of_downloads_last_selected();
	GtkAdjustment *adj=GTK_PROGRESS(ProgressOfDownload)->adjustment;
	if (adj){
		if(temp && (temp->finfo.size>0 || temp->Size.curent>0)){
			adj->lower=0;
			adj->upper = temp->finfo.size>temp->Size.curent ? temp->finfo.size : temp->Size.curent;
			gtk_progress_set_value(GTK_PROGRESS(ProgressOfDownload),temp->Size.curent);
			gtk_progress_set_show_text(GTK_PROGRESS(ProgressOfDownload),TRUE);
		}else{
			adj->lower=0;
			adj->upper = 1;
			gtk_progress_set_value(GTK_PROGRESS(ProgressOfDownload),1);
			gtk_progress_set_show_text(GTK_PROGRESS(ProgressOfDownload),FALSE);
		};
	};
	gtk_widget_show(ProgressOfDownload);
	char data[MAX_LEN];
	char data1[MAX_LEN];
	make_number_nicel(data,GVARS.READED_BYTES);
	sprintf(data1,"%s(%iB/s)",data,GlobalMeter->last_value());
	gtk_statusbar_pop(GTK_STATUSBAR(ReadedBytesStatusBar),RBStatusBarContext);
	gtk_statusbar_push(GTK_STATUSBAR(ReadedBytesStatusBar),RBStatusBarContext,data1);
};
/* ******************************************************************** */
static void main_window_normalize_coords(){
	int temp,w,h;
	gdk_window_get_geometry((GdkWindow *)NULL,&temp,&temp,&w,&h,&temp);
	if (CFG.WINDOW_X_POSITION<0){
		while (CFG.WINDOW_X_POSITION<0)
			CFG.WINDOW_X_POSITION+=w;
	}else{
		while (CFG.WINDOW_X_POSITION>w)
			CFG.WINDOW_X_POSITION-=w;
	};
	if (CFG.WINDOW_Y_POSITION<0){
		while (CFG.WINDOW_Y_POSITION<0)
			CFG.WINDOW_Y_POSITION+=h;
	}else{
		while (CFG.WINDOW_Y_POSITION>h)
			CFG.WINDOW_Y_POSITION-=h;
	};
};

static void cb_page_size( GtkAdjustment *get) {
	if (get==NULL) return;
	if (main_log_value==get->value && get->value<get->upper-get->page_size) {
		//added 0.01 to prevent interface lockups
		get->value=get->upper-get->page_size;
		main_log_value=get->value;
		gtk_signal_emit_by_name (GTK_OBJECT (get), "changed");
	} else
		main_log_value=get->value;
}

void list_of_downloads_allocation(GtkWidget *paned,GtkAllocation *allocation){
	if (MAIN_PANED_HEIGHT && allocation->height!=MAIN_PANED_HEIGHT){
		float ratio=(float)CFG.WINDOW_CLIST_HEIGHT/(float)(MAIN_PANED_HEIGHT-GTK_PANED(MAIN_PANED)->gutter_size);
		CFG.WINDOW_CLIST_HEIGHT=int(ratio*(float)(allocation->height-GTK_PANED(MAIN_PANED)->gutter_size));
		list_of_downloads_set_height();
	};
	MAIN_PANED_HEIGHT=allocation->height;
	list_of_downloads_get_height();
};

void init_main_window() {
	GtkWidget *hbox=gtk_hbox_new(FALSE,1);
	MainHBox=hbox;
	gtk_box_pack_start (GTK_BOX (hbox), MainStatusBar, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), ProgressOfDownload, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), ReadedBytesStatusBar, FALSE, FALSE, 0);
	gtk_widget_set_usize(hbox,-1,21);

	MainLogList=gtk_clist_new(ML_COL_LAST);
	gtk_clist_set_column_width (GTK_CLIST(MainLogList),ML_COL_NUM,1);
	gtk_clist_set_column_width (GTK_CLIST(MainLogList),ML_COL_TIME,100);
	gtk_clist_set_column_width (GTK_CLIST(MainLogList),ML_COL_STRING,800);
	for (int i=0;i<ML_COL_LAST;i++)
		gtk_clist_set_column_auto_resize(GTK_CLIST(MainLogList),i,TRUE);
	gtk_clist_set_column_justification (GTK_CLIST(MainLogList), ML_COL_NUM, GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification (GTK_CLIST(MainLogList), ML_COL_TIME, GTK_JUSTIFY_LEFT);
	gtk_clist_set_column_justification (GTK_CLIST(MainLogList), ML_COL_DATE, GTK_JUSTIFY_LEFT);
	gtk_clist_set_column_justification (GTK_CLIST(MainLogList), ML_COL_STRING, GTK_JUSTIFY_LEFT);

	GtkWidget *hpaned=gtk_vpaned_new();
	MAIN_PANED=hpaned;
	main_log_adj = (GtkAdjustment *)gtk_adjustment_new (0.0, 0.0, 0.0, 0.1, 1.0, 1.0);
	main_log_value=0.0;
	gtk_signal_connect (GTK_OBJECT (main_log_adj), "changed",
	                    GTK_SIGNAL_FUNC (cb_page_size), NULL);
	GtkWidget *scroll_window=gtk_scrolled_window_new((GtkAdjustment *)NULL,main_log_adj);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll_window),MainLogList);
	gtk_paned_add1(GTK_PANED(hpaned),ContainerForCList);
	gtk_paned_add2(GTK_PANED(hpaned),scroll_window);

	GtkWidget *TEMP=my_gtk_graph_new();//gtk_statusbar_new();
	GLOBAL_GRAPH=(MyGtkGraph *)TEMP;
	gtk_widget_set_usize(TEMP,104,-1);
	gtk_box_pack_end (GTK_BOX (hbox), TEMP, FALSE, FALSE, 0);

	GtkWidget *vbox=gtk_vbox_new(FALSE,1);
	gtk_box_pack_start (GTK_BOX (vbox), MainMenu, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), ButtonsBar, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(MainWindow),vbox);

	gtk_widget_show(MainStatusBar);
	gtk_widget_show(ProgressOfDownload);
	gtk_widget_show(TEMP);
	gtk_widget_show(ReadedBytesStatusBar);
	gtk_progress_bar_update((GtkProgressBar *)ProgressOfDownload,0);
	gtk_widget_show(ListOfDownloads);

	gtk_widget_show(ContainerForCList);
	gtk_widget_show(MainLogList);
	gtk_widget_show(scroll_window);
	gtk_widget_show(hpaned);
	gtk_widget_show(MainMenu);
	gtk_widget_show(hbox);
	gtk_widget_show(vbox);
	gtk_widget_show(MainWindow);
	gdk_window_move_resize(MainWindow->window,	gint(CFG.WINDOW_X_POSITION),gint(CFG.WINDOW_Y_POSITION),
												gint(CFG.WINDOW_WIDTH),gint(CFG.WINDOW_HEIGHT));
	gtk_signal_connect (GTK_OBJECT (MAIN_PANED), "size_allocate",
	                    GTK_SIGNAL_FUNC (list_of_downloads_allocation), NULL);
};

/* ******************************************************************* */
static void tmp_scroll_title(char *title,int index){
	if (CFG.SCROLL_MAINWIN_TITLE){
		ScrollShift[index]+=1;
		if (ScrollShift[index]>=strlen(title))	ScrollShift[index]=0;
		scroll_string_left(title,ScrollShift[index]);
	};	
};

void update_mainwin_title() {
	if (CFG.USE_MAINWIN_TITLE) {
		mainwin_title_state=1;
		tDownload *temp=list_of_downloads_last_selected();
		char data[MAX_LEN];
		if (temp && (CFG.USE_MAINWIN_TITLE2==0 || UpdateTitleCycle % 3)) {
			char data2[MAX_LEN];
			char data3[MAX_LEN];
			make_number_nice(data2,temp->Size.curent);
			if (temp->finfo.size>=0)
				make_number_nice(data3,temp->finfo.size);
			else
				sprintf(data3,"???");
			sprintf(data,"%i%c %s/%s %s ",temp->Percent.curent,'%',data2,data3,temp->info->file.get());
			dnd_trash_set_tooltip(data);
			tmp_scroll_title(data,ROLL_STAT);
			gtk_window_set_title(GTK_WINDOW (MainWindow), data);
		} else {
			if (CFG.USE_MAINWIN_TITLE2) {
				int total=amount_of_downloads_in_queues();
				sprintf(data,_("%i-running %i-completed %i-total "),DOWNLOAD_QUEUES[DL_RUN]->count(),DOWNLOAD_QUEUES[DL_COMPLETE]->count(),total);
				tmp_scroll_title(data,ROLL_INFO);
				gtk_window_set_title(GTK_WINDOW (MainWindow), data);
			} else{
				gtk_window_set_title(GTK_WINDOW (MainWindow), VERSION_NAME);
				dnd_trash_set_tooltip(_("Drop link here"));
			};
		};
	} else{
		if (mainwin_title_state){
			gtk_window_set_title(GTK_WINDOW (MainWindow), VERSION_NAME);
			dnd_trash_set_tooltip(_("Drop link here"));
		};
		mainwin_title_state=0;
	};
};

int time_for_refresh(void *a) {
	list_of_downloads_freeze();
	aa.main_circle();
	list_of_downloads_unfreeze();
	update_progress_bar();
	update_mainwin_title();
	UpdateTitleCycle+=1;
	tDownload *tmp=list_of_downloads_last_selected();
	if (tmp && DOWNLOAD_QUEUES[DL_RUN]->owner(tmp))
		LocalMeter->add(tmp->NanoSpeed);
	else
		LocalMeter->add(0);
	my_gtk_graph_recalc(GLOBAL_GRAPH);
	return 1;
};

int time_for_logs_refresh(void *a) {
	aa.redraw_logs();
	aa.check_for_remote_commands();
	MainTimer-=1;
//	dnd_trash_refresh();
	if (MainTimer==0) {
		time_for_refresh(NULL);
		MainTimer=(GLOBAL_SLEEP_DELAY*1000)/100;
//		get_mainwin_sizes(MainWindow);
	};
	return 1;
};

int get_mainwin_sizes(GtkWidget *window) {
	if (FirstConfigureEvent) {
		FirstConfigureEvent=0;
		return FALSE;
	};
	if (window!=NULL && window->window!=NULL) {
		gint x,y,w,h;
		gdk_window_get_position(window->window,&x,&y);
		gdk_window_get_size(window->window,&w,&h);
		/*
		if (CFG.WINDOW_HEIGHT != int(h)){
			printf("changed from %i to %i!\n",CFG.WINDOW_HEIGHT,h);
		};
		*/
		CFG.WINDOW_X_POSITION=int(x);
		CFG.WINDOW_Y_POSITION=int(y);
		CFG.WINDOW_HEIGHT=int(h);
		CFG.WINDOW_WIDTH=int(w);
	};
	return FALSE;
};

int check_for_clipboard_skiping(char *buf){
	char *extension,*temp;
	if (CFG.CLIPBOARD_SKIP_OR_CATCH){
		extension=new char[strlen(CFG.CATCH_IN_CLIPBOARD)+1];
		temp=CFG.CATCH_IN_CLIPBOARD;
		do{
			temp=extract_string(temp,extension);
			if (string_ended(extension,buf)==0){
				delete(extension);
				return 1;
			};
		}while(temp!=NULL && strlen(temp)>0);
		delete(extension);
		return 0;
	};
	extension=new char[strlen(CFG.SKIP_IN_CLIPBOARD)+1];
	temp=CFG.SKIP_IN_CLIPBOARD;
	do{
		temp=extract_string(temp,extension);
		if (string_ended(extension,buf)==0){
			delete(extension);
			return 0;
		};
	}while(temp!=NULL && strlen(temp)>0);
	delete(extension);
	return 1;
};

void my_get_xselection(GtkWidget *window, GdkEvent *event) {
	char *buf;
	GdkAtom atom;
	gint format, length;

	length=gdk_selection_property_get(window->window, (guchar **)&buf, &atom, &format);
	if (length) {
		if (!equal(buf, OLD_CLIPBOARD_CONTENT)) {
			if ((begin_string_uncase(buf,"ftp://") || begin_string_uncase(buf,"http://"))
			    && OLD_CLIPBOARD_CONTENT!=NULL && check_for_clipboard_skiping(buf))
				init_add_dnd_window(buf);
			if (OLD_CLIPBOARD_CONTENT) g_free(OLD_CLIPBOARD_CONTENT);
			OLD_CLIPBOARD_CONTENT = buf;
		}else
			g_free(buf);
	} else {
		if (OLD_CLIPBOARD_CONTENT==NULL) OLD_CLIPBOARD_CONTENT=copy_string("");
		if (buf) g_free(buf);
	}
}

int time_for_draw_graph(void *a) {
/* clipboard monitoring */
	if (CFG.CLIPBOARD_MONITOR) {
		gtk_selection_convert(MainWindow, GDK_SELECTION_PRIMARY,
				      GDK_TARGET_STRING,
				      GDK_CURRENT_TIME);
	}
	return 1;
};

int time_for_save_list(void *a) {
	SAVE_LIST_INTERVAL-=1;
	if (!SAVE_LIST_INTERVAL) {
		if (CFG.SAVE_LIST) {
			save_list();
		};
		SAVE_LIST_INTERVAL=CFG.SAVE_LIST_INTERVAL;
	};
	if (CFG.EXIT_COMPLETE && aa.complete()){
		EXIT_COMPLETE_INTERVAL-=1;
		if (EXIT_COMPLETE_INTERVAL<0){
			my_main_quit();
			return 0;
		};
	}else{
		EXIT_COMPLETE_INTERVAL=CFG.EXIT_COMPLETE_TIME;
	}
	return 1;
};

void init_timeouts() {
	SAVE_LIST_INTERVAL=CFG.SAVE_LIST_INTERVAL;
	EXIT_COMPLETE_INTERVAL=CFG.EXIT_COMPLETE_TIME;
	ListTimer = gtk_timeout_add (60000, time_for_save_list , NULL);
	GraphTimer = gtk_timeout_add (250, time_for_draw_graph , NULL);
	LogsTimer = gtk_timeout_add (100, time_for_logs_refresh , NULL);
	MainTimer=(GLOBAL_SLEEP_DELAY*1000)/100;
	FirstConfigureEvent=1;
	gtk_signal_connect(GTK_OBJECT(MainWindow), "configure_event",
	                   GTK_SIGNAL_FUNC(get_mainwin_sizes),
	                   MainWindow);
};

void main_window_iconify(){
	if (MainWindow)
		my_gdk_window_iconify(MainWindow->window);
};

void main_window_popup(){
	if (MainWindow)
		gdk_window_show(MainWindow->window);
};

void main_window_toggle(){
	if (MainWindow){
		if (gdk_window_is_visible(MainWindow->window)){
			gdk_window_hide(MainWindow->window);
		}else{
			gdk_window_show(MainWindow->window);
		};
	};
};

void init_face(int argc, char *argv[]) {
	gtk_set_locale();
	gtk_init(&argc, &argv);
	gdk_rgb_init();
	init_columns_info();
	main_window_normalize_coords();
	MainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_uposition(MainWindow,gint(CFG.WINDOW_X_POSITION),gint(CFG.WINDOW_Y_POSITION));
	gtk_window_set_default_size(GTK_WINDOW(MainWindow),gint(CFG.WINDOW_WIDTH),gint(CFG.WINDOW_HEIGHT));
	gtk_window_set_title(GTK_WINDOW (MainWindow), VERSION_NAME);
	gtk_widget_set_usize( GTK_WIDGET (MainWindow), 400, 200);
	gtk_widget_realize(MainWindow);
	MainWindowGC=gdk_gc_new(MainWindow->window);

        dnd_trash_init_menu();
	init_main_menu();
	init_list_menu();
	init_status_bar();
	list_of_downloads_init_pixmaps();
	list_of_downloads_init();
	init_buttons_bar();
	init_main_window();
	init_pixmaps_for_log();
/* initing table of shifts
 */
	for (int i=0;i<ROLL_LAST;i++)
		ScrollShift[i]=0;
	prepare_buttons();
	FaceForLimits=new tFaceLimits;
	FaceForPasswords=new tFacePass;
#include "pixmaps/dndtrash.xpm"
	GdkBitmap *bitmap;
	GdkPixmap *pixmap=make_pixmap_from_xpm(&bitmap,dndtrash_xpm);
	gdk_window_set_icon(MainWindow->window,(GdkWindow *)NULL,pixmap,bitmap);
	gtk_signal_connect(GTK_OBJECT(MainWindow), "delete_event",
	                   GTK_SIGNAL_FUNC(ask_exit2),
	                   NULL);
	gtk_signal_connect(GTK_OBJECT(MainWindow), "selection_received",
			   GTK_SIGNAL_FUNC(my_get_xselection),NULL);
	gtk_signal_connect (GTK_OBJECT (MainWindow),
			    "key_press_event",
			    GTK_SIGNAL_FUNC (main_menu_prepare),
			    NULL);
	if (CFG.DND_TRASH) dnd_trash_init();
	main_log_mask=0;
};
