/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2002 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <package_config.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../dlist.h"
#include "../main.h"
#include "../var.h"
#include "../savelog.h"
#include "../config.h"
#include "../locstr.h"
#include "../ntlocale.h"
#include "../sndserv.h"
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
#include "misc.h"
#include "dndtrash.h"
#include "passface.h"
#include "colors.h"
#include "fsface.h"
#include "fsched.h"
#include "filtrgui.h"
#include "../xml.h"

#undef FLT_ROUNDS
#define FLT_ROUNDS 3

GtkWidget *MainMenu;
GtkAdjustment *ProgressBarValues;
GtkWidget *ProgressOfDownload;
GtkWidget *MainStatusBar,*ReadedBytesStatusBar;
GtkWidget *MainWindow=(GtkWidget *)NULL;
GtkWidget *MainHBox;
GtkWidget *ContainerForCList=(GtkWidget *)NULL;
GdkGC *MainWindowGC=(GdkGC *)NULL;
GtkCList *FSearchCList;
GtkWidget *BoxForGraph;
GtkItemFactory *main_menu_item_factory=NULL;
GtkWidget *MainLogList,*MAIN_PANED=(GtkWidget *)NULL,*MAIN_PANED2=(GtkWidget *)NULL;
GtkWidget *MAIN_PANED1=(GtkWidget *)NULL;
d4xQsTree *D4X_QVT;
int main_log_mask;
unsigned int ScrollShift[2];
int mainwin_title_state;
gfloat main_log_value;
GtkAdjustment *main_log_adj;

GtkItemFactory *list_menu_itemfact;

tConfirmedDialog *AskDelete=(tConfirmedDialog *)NULL;
tConfirmedDialog *AskDeleteCompleted=(tConfirmedDialog *)NULL;
tConfirmedDialog *AskDeleteFataled=(tConfirmedDialog *)NULL;
tConfirmedDialog *AskExit=(tConfirmedDialog *)NULL;

gint StatusBarContext,RBStatusBarContext;
int MainTimer,LogsTimer,GraphTimer,ListTimer;
int SAVE_LIST_INTERVAL,EXIT_COMPLETE_INTERVAL;
//pthread_mutex_t MAIN_GTK_MUTEX=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

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
	MM_FILE, MM_FILE_SAVE, MM_FILE_LOAD, MM_FILE_TXT, MM_FILE_NEW, MM_FILE_PASTE, MM_FILE_AUTO, MM_FILE_EXIT, MM_FILE_SEP,
	MM_DOWNLOAD, MM_DOWNLOAD_LOG, MM_DOWNLOAD_STOP, MM_DOWNLOAD_EDIT, MM_DOWNLOAD_DEL, MM_DOWNLOAD_RUN, MM_DOWNLOAD_DEL_C,
	MM_DOWNLOAD_DEL_F,MM_DOWNLOAD_RERUN, MM_DOWNLOAD_PROTECT,MM_DOWNLOAD_UNSELECT_ALL,MM_DOWNLOAD_SELECT_ALL ,MM_DOWNLOAD_INVERT, MM_DOWNLOAD_SEP,
	MM_OPTIONS, MM_OPTIONS_SCHEDULER, MM_OPTIONS_PASSWORDS, MM_OPTIONS_COMMON, MM_OPTIONS_FILTERS,
	MM_OPTIONS_SPEED, MM_OPTIONS_SPEED_1, MM_OPTIONS_SPEED_2, MM_OPTIONS_SPEED_3,
	MM_OPTIONS_BUTTONS, MM_OPTIONS_BUTTONS_ADD, MM_OPTIONS_BUTTONS_MAN, MM_OPTIONS_BUTTONS_SPEED, MM_OPTIONS_BUTTONS_MISC,
	MM_HELP, MM_HELP_ABOUT
};

char *main_menu_kb[MM_HELP_ABOUT+1];

char *main_menu_kbnames[]={
	"file",
	"save",
	"load",
	"findlinks",
	"new",
	"paste",
	"auto",
	"exit",
	"",
	"download",
	"log",
	"stop",
	"edit",
	"delete",
	"continue",
	"delcompleted",
	"delfailed",
	"rerunfailed",
	"protect",
	"unselect",
	"select",
	"invert",
	"",
	"options",
	"scheduler",
	"urlmanager",
	"general",
	"filters",
	"speed",
	"low",
	"medium",
	"unlimited",
	"buttons",
	"",
	"",
	"",
	"",
	"help",
	"about"
};

char *main_menu_inames[]={
	N_("/_File"),
	N_("/File/_Save List"),
	N_("/File/_Load List"),
	N_("/File/Find links in file"),
	N_("/File/_New Download"),
	N_("/File/_Paste Download"),
	N_("/File/_Automated adding"),
	N_("/File/Exit"),
	N_("/File/sep1"),
	N_("/_Download"),
	N_("/Download/View _Log"),
	N_("/Download/_Stop downloads"),
	N_("/Download/Edit download"),
	N_("/Download/_Delete downloads"),
	N_("/Download/Continue downloads"),
	N_("/Download/Delete completed"),
	N_("/Download/Delete failed"),
	N_("/Download/Rerun failed"),
	N_("/Download/(Un)Protect"),
	N_("/Download/Unselect all"),
	N_("/Download/Select all"),
	N_("/Download/Invert selection"),
	N_("/Download/-"),
	N_("/_Options"),
	N_("/Options/Scheduler"),
	N_("/Options/URL-manager"),
	N_("/Options/General"),
	N_("/Options/Filters"),
	N_("/Options/Speed"),
	N_("/Options/Speed/Low"),
	N_("/Options/Speed/Medium"),
	N_("/Options/Speed/Unlimited"),
	N_("/Options/Buttons"),
	N_("/Options/Buttons/Add buttons"),
	N_("/Options/Buttons/Manipulating"),
	N_("/Options/Buttons/Speed buttons"),
	N_("/Options/Buttons/Misc buttons"),
	N_("/_Help"),
	N_("/_Help/About")
};

void d4x_load_accelerators();

char *old_clipboard_content(){
	if (CFG.CLIPBOARD_MONITOR)
		return(OLD_CLIPBOARD_CONTENT);
	return(NULL);
};


static void open_passwords_window(...) {
	FaceForPasswords->init();
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

void main_menu_speed_prepare(){
	GtkWidget *menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_OPTIONS_SPEED_1]));
	if (menu_item){
		GTK_CHECK_MENU_ITEM(menu_item)->active=CFG.SPEED_LIMIT==1?TRUE:FALSE;
		if (GTK_WIDGET_VISIBLE(menu_item)) gtk_widget_queue_draw(menu_item);
	};
	menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_OPTIONS_SPEED_2]));
	if (menu_item){
		GTK_CHECK_MENU_ITEM(menu_item)->active=CFG.SPEED_LIMIT==2?TRUE:FALSE;
		if (GTK_WIDGET_VISIBLE(menu_item)) gtk_widget_queue_draw(menu_item);
	};
	menu_item=gtk_item_factory_get_widget(main_menu_item_factory,_(main_menu_inames[MM_OPTIONS_SPEED_3]));
	if (menu_item){
		GTK_CHECK_MENU_ITEM(menu_item)->active=CFG.SPEED_LIMIT==3?TRUE:FALSE;
		if (GTK_WIDGET_VISIBLE(menu_item)) gtk_widget_queue_draw(menu_item);
	};
};

void load_accelerated(gpointer *p,gint realnum){
	tString *str=ALL_HISTORIES[SAVE_HISTORY]->last();
	int num=realnum-128;
	for (int i=0;i<=num;i++){
		if (str==NULL) return;
		if (i==num) read_list_from_file_current(str->body);
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

static char *copy_without_underscore(char *what){
	char *d=copy_string(what);
	_remove_underscore(d);
	return(d);
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
			delete[] LOAD_ACCELERATORS[j];
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
			delete[] tmp.path;
			delete[] tmp.accelerator;
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
	delete[] path;
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

void mmenu_open_logs(){
	D4X_QUEUE->qv.open_logs();
};
void mmenu_unselect_all(){
	D4X_QUEUE->qv.unselect_all();
};
void mmenu_select_all(){
	D4X_QUEUE->qv.select_all();
};
void mmenu_invert_selection(){
	D4X_QUEUE->qv.invert_selection();
};


void init_main_menu() {
	d4x_load_accelerators();
	GtkItemFactoryEntry menu_items[] = {
		{_(main_menu_inames[MM_FILE]),		(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<Branch>"},
		{_(main_menu_inames[MM_FILE_SAVE]),	main_menu_kb[MM_FILE_SAVE],	(GtkItemFactoryCallback)init_save_list,			0, (gchar *)NULL},
		{_(main_menu_inames[MM_FILE_LOAD]),	main_menu_kb[MM_FILE_LOAD],	(GtkItemFactoryCallback)init_load_list,			0, (gchar *)NULL},
		{_(main_menu_inames[MM_FILE_TXT]),	main_menu_kb[MM_FILE_TXT],	(GtkItemFactoryCallback)init_load_txt_list,	0, (gchar *)NULL},
		{_(main_menu_inames[MM_FILE_SEP]),	(gchar *)NULL,  (GtkItemFactoryCallback)NULL,	0, "<Separator>"},
		{_(main_menu_inames[MM_FILE_NEW]),	main_menu_kb[MM_FILE_NEW],	(GtkItemFactoryCallback)init_add_window,		0, (gchar *)NULL},
		{_(main_menu_inames[MM_FILE_PASTE]),	main_menu_kb[MM_FILE_PASTE],	(GtkItemFactoryCallback)init_add_clipboard_window,	0, (gchar *)NULL},
		{_(main_menu_inames[MM_FILE_AUTO]),	main_menu_kb[MM_FILE_AUTO],	(GtkItemFactoryCallback)d4x_automated_add,	0, (gchar *)NULL},
		{_(main_menu_inames[MM_FILE_SEP]),	(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<Separator>"},
		{_(main_menu_inames[MM_FILE_EXIT]),	main_menu_kb[MM_FILE_EXIT],	(GtkItemFactoryCallback)ask_exit,			0, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD]),     	(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<Branch>"},
		{_(main_menu_inames[MM_DOWNLOAD_LOG]), 	main_menu_kb[MM_DOWNLOAD_LOG],	(GtkItemFactoryCallback)mmenu_open_logs,	100+MM_DOWNLOAD_LOG, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_STOP]),	main_menu_kb[MM_DOWNLOAD_STOP],	(GtkItemFactoryCallback)stop_downloads,		        100+MM_DOWNLOAD_STOP, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_EDIT]),	main_menu_kb[MM_DOWNLOAD_EDIT],	(GtkItemFactoryCallback)open_edit_for_selected,		100+MM_DOWNLOAD_EDIT, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_DEL]),	main_menu_kb[MM_DOWNLOAD_DEL],	(GtkItemFactoryCallback)ask_delete_download,		100+MM_DOWNLOAD_DEL, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_RUN]),	main_menu_kb[MM_DOWNLOAD_RUN],	(GtkItemFactoryCallback)continue_downloads,		100+MM_DOWNLOAD_RUN, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_SEP]),(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<Separator>"},
		{_(main_menu_inames[MM_DOWNLOAD_DEL_C]),main_menu_kb[MM_DOWNLOAD_DEL_C],	(GtkItemFactoryCallback)ask_delete_completed_downloads,	0, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_DEL_F]),main_menu_kb[MM_DOWNLOAD_DEL_F],	(GtkItemFactoryCallback)ask_delete_fataled_downloads,	0, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_RERUN]),main_menu_kb[MM_DOWNLOAD_RERUN],	(GtkItemFactoryCallback)_rerun_failed_downloads,	0, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_SEP]),(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<Separator>"},
		{_(main_menu_inames[MM_DOWNLOAD_PROTECT]),main_menu_kb[MM_DOWNLOAD_PROTECT],	(GtkItemFactoryCallback)lm_inv_protect_flag,	100+MM_DOWNLOAD_PROTECT, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_SEP]),(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<Separator>"},
		{_(main_menu_inames[MM_DOWNLOAD_UNSELECT_ALL]),main_menu_kb[MM_DOWNLOAD_UNSELECT_ALL], (GtkItemFactoryCallback)mmenu_unselect_all,	100+MM_DOWNLOAD_UNSELECT_ALL, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_SELECT_ALL]),main_menu_kb[MM_DOWNLOAD_SELECT_ALL], (GtkItemFactoryCallback)mmenu_select_all,	100+MM_DOWNLOAD_SELECT_ALL, (gchar *)NULL},
		{_(main_menu_inames[MM_DOWNLOAD_INVERT]),main_menu_kb[MM_DOWNLOAD_INVERT], (GtkItemFactoryCallback)mmenu_invert_selection,	100+MM_DOWNLOAD_INVERT, (gchar *)NULL},
		{_(main_menu_inames[MM_OPTIONS]),	(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<Branch>"},
		{_(main_menu_inames[MM_OPTIONS_SCHEDULER]),main_menu_kb[MM_OPTIONS_SCHEDULER],	(GtkItemFactoryCallback)d4x_scheduler_init,		0, (gchar *)NULL},
		{_(main_menu_inames[MM_OPTIONS_PASSWORDS]),main_menu_kb[MM_OPTIONS_PASSWORDS],	(GtkItemFactoryCallback)open_passwords_window,		0, (gchar *)NULL},
		{_(main_menu_inames[MM_OPTIONS_COMMON]),main_menu_kb[MM_OPTIONS_COMMON],	(GtkItemFactoryCallback)d4x_prefs_init,			0, (gchar *)NULL},
		{_(main_menu_inames[MM_OPTIONS_FILTERS]),main_menu_kb[MM_OPTIONS_FILTERS],	(GtkItemFactoryCallback)d4x_filters_window_init,			0, (gchar *)NULL},
		{_(main_menu_inames[MM_OPTIONS_SPEED]),	(gchar*)NULL,	(GtkItemFactoryCallback)NULL,	0, "<Branch>"},
		{_(main_menu_inames[MM_OPTIONS_SPEED_1]),main_menu_kb[MM_OPTIONS_SPEED_1],	(GtkItemFactoryCallback)main_menu_speed_calback,	1, "<RadioItem>"},
		{_(main_menu_inames[MM_OPTIONS_SPEED_2]),main_menu_kb[MM_OPTIONS_SPEED_2],	(GtkItemFactoryCallback)main_menu_speed_calback,	2, _(main_menu_inames[MM_OPTIONS_SPEED_1])},
		{_(main_menu_inames[MM_OPTIONS_SPEED_3]),main_menu_kb[MM_OPTIONS_SPEED_3],	(GtkItemFactoryCallback)main_menu_speed_calback,	3, _(main_menu_inames[MM_OPTIONS_SPEED_2])},
		{_(main_menu_inames[MM_OPTIONS_BUTTONS]),main_menu_kb[MM_OPTIONS_BUTTONS], (GtkItemFactoryCallback)buttons_configure,	0, (gchar *)NULL},
		{_(main_menu_inames[MM_HELP]),		(gchar *)NULL,	(GtkItemFactoryCallback)NULL,	0, "<LastBranch>"},
		{_(main_menu_inames[MM_HELP_ABOUT]),	main_menu_kb[MM_HELP_ABOUT],	(GtkItemFactoryCallback)init_about_window,		0, (gchar *)NULL},
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
	for (int i=0;i<=MM_HELP_ABOUT;i++)
		if (main_menu_kb[i]){
			delete[] main_menu_kb[i];
			main_menu_kb[i]=NULL;
		};
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
	if (!D4X_QUEUE->qv.sel()){
		for (int i=MM_DOWNLOAD_LOG;i<=MM_DOWNLOAD_RUN;i++){
			menu_item=gtk_item_factory_get_item_by_action(main_menu_item_factory,
								      i+100);
			if (menu_item) gtk_widget_set_sensitive(menu_item,TRUE);
		};
		menu_item=gtk_item_factory_get_item_by_action(main_menu_item_factory,
							      MM_DOWNLOAD_PROTECT+100);
		if (menu_item) gtk_widget_set_sensitive(menu_item,TRUE);
	}else{
		for (int i=MM_DOWNLOAD_LOG;i<=MM_DOWNLOAD_RUN;i++){
			menu_item=gtk_item_factory_get_item_by_action(main_menu_item_factory,
								      i+100);
			if (menu_item) gtk_widget_set_sensitive(menu_item,FALSE);
		};
		menu_item=gtk_item_factory_get_item_by_action(main_menu_item_factory,
							      MM_DOWNLOAD_PROTECT+100);
		if (menu_item) gtk_widget_set_sensitive(menu_item,FALSE);
	};
	if (D4X_QUEUE->qv.rows()==0){
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

static int _fd_kb_=-1;

static void d4x_menuitem_foreach(GtkWidget *widget,gpointer data){
	if (GTK_IS_ACCEL_LABEL (widget) && (GTK_ACCEL_LABEL (widget))->accel_string){
		char *s=unparse_percents(skip_spaces((GTK_ACCEL_LABEL (widget))->accel_string));
		f_wstr(_fd_kb_,s);
		delete[] s;
	};
};

static void d4x_save_kb(int menu){
	char *name=copy_without_underscore(_(main_menu_inames[menu]));
	GtkWidget *menu_item=gtk_item_factory_get_widget(main_menu_item_factory,name);
	delete[] name;
	if (menu_item){
		f_wstr(_fd_kb_,"\t<");
		f_wstr(_fd_kb_,main_menu_kbnames[menu]);
		f_wchar(_fd_kb_,'>');
		gtk_container_foreach(GTK_CONTAINER (menu_item),d4x_menuitem_foreach,NULL);
		f_wstr(_fd_kb_,"</");
		f_wstr(_fd_kb_,main_menu_kbnames[menu]);
		f_wstr(_fd_kb_,">\n");
	};
};

static void d4x_lap(tQueue *q,char *menu,int name){
	char *s=sum_strings(menu," ",main_menu_kbnames[name],NULL);
	d4xXmlObject *xml=d4x_xml_find_obj(q,s);
	delete[] s;
	if (xml){
		s=xml->value.get();
		if (s && strlen(s)){
			int alt=0,ctrl=0,shift=0;
			if (strstr(s,"Ctl+")) ctrl=1;
			if (strstr(s,"Alt+")) alt=1;
			if (strstr(s,"Shft+")) shift=1;
			char c[2]={0,0};
			c[0]=s[strlen(s)-1];
			main_menu_kb[name]=sum_strings(alt?"<alt>":"",ctrl?"<control>":"",shift?"<shift>":"",c,NULL);
		};
	};
};

void d4x_load_accelerators(){
	for (int i=0;i<=MM_HELP_ABOUT;i++) main_menu_kb[i]=NULL;
	char *path=sum_strings(HOME_VARIABLE,"/",CFG_DIR,"/keybinds.xml",NULL);
	tQueue *q=d4x_xml_parse_file(path);
	delete[] path;
	if (q==NULL){
		main_menu_kb[MM_FILE_SAVE]=copy_string("<control>S");
		main_menu_kb[MM_FILE_LOAD]=copy_string("<control>L");
		main_menu_kb[MM_FILE_TXT]=copy_string("<control><alt>L");
		main_menu_kb[MM_FILE_NEW]=copy_string("<control>N");
		main_menu_kb[MM_FILE_PASTE]=copy_string("<control>P");
		main_menu_kb[MM_FILE_AUTO]=copy_string("<control><alt>A");
		main_menu_kb[MM_FILE_EXIT]=copy_string("<alt>X");
		main_menu_kb[MM_DOWNLOAD_STOP]=copy_string("<alt>S");
		main_menu_kb[MM_DOWNLOAD_EDIT]=copy_string("<alt>E");
		main_menu_kb[MM_DOWNLOAD_DEL]=copy_string("<alt>C");
		main_menu_kb[MM_DOWNLOAD_RUN]=copy_string("<alt>A");
		main_menu_kb[MM_DOWNLOAD_PROTECT]=copy_string("<alt><control>P");
		main_menu_kb[MM_OPTIONS_COMMON]=copy_string("<control>C");
		main_menu_kb[MM_OPTIONS_FILTERS]=copy_string("<control>F");
	}else{
		d4x_lap(q,"file",MM_FILE_SAVE);
		d4x_lap(q,"file",MM_FILE_LOAD);
		d4x_lap(q,"file",MM_FILE_TXT);
		d4x_lap(q,"file",MM_FILE_NEW);
		d4x_lap(q,"file",MM_FILE_PASTE);
		d4x_lap(q,"file",MM_FILE_AUTO);
		d4x_lap(q,"file",MM_FILE_EXIT);
		d4x_lap(q,"download",MM_DOWNLOAD_LOG);
		d4x_lap(q,"download",MM_DOWNLOAD_STOP);
		d4x_lap(q,"download",MM_DOWNLOAD_EDIT);
		d4x_lap(q,"download",MM_DOWNLOAD_DEL);
		d4x_lap(q,"download",MM_DOWNLOAD_RUN);
		d4x_lap(q,"download",MM_DOWNLOAD_DEL_C);
		d4x_lap(q,"download",MM_DOWNLOAD_DEL_F);
		d4x_lap(q,"download",MM_DOWNLOAD_RERUN);
		d4x_lap(q,"download",MM_DOWNLOAD_PROTECT);
		d4x_lap(q,"download",MM_DOWNLOAD_UNSELECT_ALL);
		d4x_lap(q,"download",MM_DOWNLOAD_SELECT_ALL);
		d4x_lap(q,"download",MM_DOWNLOAD_INVERT);
		d4x_lap(q,"options",MM_OPTIONS_SCHEDULER);
		d4x_lap(q,"options",MM_OPTIONS_PASSWORDS);
		d4x_lap(q,"options",MM_OPTIONS_COMMON);
		d4x_lap(q,"options",MM_OPTIONS_FILTERS);
		d4x_lap(q,"options",MM_OPTIONS_SPEED_1);
		d4x_lap(q,"options",MM_OPTIONS_SPEED_2);
		d4x_lap(q,"options",MM_OPTIONS_SPEED_3);
		d4x_lap(q,"options",MM_OPTIONS_BUTTONS);
		d4x_lap(q,"options",MM_HELP_ABOUT);
		delete(q);
	};
};

void d4x_save_accelerators(){
	char *path=sum_strings(HOME_VARIABLE,"/",CFG_DIR,"/keybinds.xml",NULL);
	int fd=_fd_kb_=open(path,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	delete[] path;
	if (fd<0) return;
	f_wstr(fd,"<file>\n");
	d4x_save_kb(MM_FILE_SAVE);
	d4x_save_kb(MM_FILE_LOAD);
	d4x_save_kb(MM_FILE_TXT);
	d4x_save_kb(MM_FILE_NEW);
	d4x_save_kb(MM_FILE_PASTE);
	d4x_save_kb(MM_FILE_AUTO);
	d4x_save_kb(MM_FILE_EXIT);
	f_wstr(fd,"</file>\n<download>\n");
	d4x_save_kb(MM_DOWNLOAD_LOG);
	d4x_save_kb(MM_DOWNLOAD_STOP);
	d4x_save_kb(MM_DOWNLOAD_EDIT);
	d4x_save_kb(MM_DOWNLOAD_DEL);
	d4x_save_kb(MM_DOWNLOAD_RUN);
	d4x_save_kb(MM_DOWNLOAD_DEL_C);
	d4x_save_kb(MM_DOWNLOAD_DEL_F);
	d4x_save_kb(MM_DOWNLOAD_RERUN);
	d4x_save_kb(MM_DOWNLOAD_PROTECT);
	d4x_save_kb(MM_DOWNLOAD_UNSELECT_ALL);
	d4x_save_kb(MM_DOWNLOAD_SELECT_ALL);
	d4x_save_kb(MM_DOWNLOAD_INVERT);
	f_wstr(fd,"</download>\n<options>\n");
	d4x_save_kb(MM_OPTIONS_SCHEDULER);
	d4x_save_kb(MM_OPTIONS_PASSWORDS);
	d4x_save_kb(MM_OPTIONS_COMMON);
	d4x_save_kb(MM_OPTIONS_FILTERS);
	d4x_save_kb(MM_OPTIONS_SPEED_1);
	d4x_save_kb(MM_OPTIONS_SPEED_2);
	d4x_save_kb(MM_OPTIONS_SPEED_3);
	d4x_save_kb(MM_OPTIONS_BUTTONS);
	f_wstr(fd,"</options>\n<help>\n");
	d4x_save_kb(MM_HELP_ABOUT);
	f_wstr(fd,"</help>\n");
	close(fd);
};

void my_main_quit(...) {
	if (CFG.WITHOUT_FACE==0){
		CFG.HIDE_MAIN_WINDOW=!gdk_window_is_visible(MainWindow->window);
		gtk_timeout_remove(MainTimer);
		gtk_timeout_remove(LogsTimer);
		gtk_timeout_remove(GraphTimer);
		D4X_QUEUE->qv.get_adj();
		d4x_save_accelerators();
	};
	save_list();
	FaceForPasswords->save();
	save_config();
	aa.done();
	if (FaceForPasswords)
		delete (FaceForPasswords);
	if (CFG.WITHOUT_FACE==0){
		dnd_trash_real_destroy();
		if (list_for_adding){
			/* free list item by item to avoid
			   searching downloads in global tDB sructure
			 */
			tDownload *tmp=list_for_adding->last();
			while (tmp){
				list_for_adding->del(tmp);
				delete(tmp);
				tmp=list_for_adding->last();
			};
			delete(list_for_adding);
		};
		d4x_prefs_cancel();
		destroy_about_window();
		gtk_widget_destroy(MainWindow);
		buttons_configure_close();
		d4x_scheduler_close();
		d4x_filters_window_destroy();
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
		var_free(&CFG);
		exit(0);
	};
};


void open_edit_for_selected(...) {
	tDownload *temp=D4X_QUEUE->qv.last_selected();
	init_edit_window(temp);
};

void del_completed_downloads(...) {
	D4X_QUEUE->qv.freeze();
	aa.del_completed();
	D4X_QUEUE->qv.unfreeze();
	if (AskDeleteCompleted) AskDeleteCompleted->done();
};

void del_fataled_downloads(...) {
	D4X_QUEUE->qv.freeze();
	aa.del_fataled();
	D4X_QUEUE->qv.unfreeze();
	if (AskDeleteFataled) AskDeleteFataled->done();
};

void stop_downloads(...) {
	D4X_QUEUE->qv.stop_downloads();
	prepare_buttons();
	aa.try_to_run_wait(D4X_QUEUE);
};

static void _my_main_quit_ask_exit_(GtkWidget *widget,tConfirmedDialog *parent){
	CFG.CONFIRM_EXIT=!(GTK_TOGGLE_BUTTON(parent->check)->active);
	my_main_quit();
};

void ask_exit(...) {
	if (CFG.CONFIRM_EXIT) {
		if (!AskExit) AskExit=new tConfirmedDialog;
		if (AskExit->init(_("Do you really want to quit?"),_("Quit?")))
			gtk_signal_connect(GTK_OBJECT(AskExit->ok_button),"clicked",
					   GTK_SIGNAL_FUNC(_my_main_quit_ask_exit_),
					   AskExit);
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

static void a_delete_downloads(GtkWidget *widget, gpointer data){
	CFG.CONFIRM_DELETE=!(GTK_TOGGLE_BUTTON(AskDelete->check)->active);
	delete_downloads(GPOINTER_TO_INT(data));
};


void ask_delete_download(...) {
	GdkModifierType mask;
	gint x,y;
	gdk_window_get_pointer(MainWindow->window,&x,&y,&mask);
	if (CFG.CONFIRM_DELETE) {
		if (!AskDelete) AskDelete=new tConfirmedDialog;
		if (AskDelete->init(_("Delete selected downloads?"),_("Delete?")))
			gtk_signal_connect(GTK_OBJECT(AskDelete->ok_button),
					   "clicked",
					   GTK_SIGNAL_FUNC(a_delete_downloads),
					   GINT_TO_POINTER(mask & GDK_SHIFT_MASK));
		AskDelete->set_modal(MainWindow);
	} else
		delete_downloads(mask & GDK_SHIFT_MASK);
};

static void _ask_delete_completed_check_(GtkWidget *widget, tConfirmedDialog *parent){
	CFG.CONFIRM_DELETE_COMPLETED=!(GTK_TOGGLE_BUTTON(parent->check)->active);
	del_completed_downloads();
};

void ask_delete_completed_downloads(...) {
	if (CFG.CONFIRM_DELETE_COMPLETED) {
		if (!AskDeleteCompleted) AskDeleteCompleted=new tConfirmedDialog;
		if (AskDeleteCompleted->init(_("Do you wish delete completed downloads?"),_("Delete?")))
			gtk_signal_connect(GTK_OBJECT(AskDeleteCompleted->ok_button),
					   "clicked",
					   GTK_SIGNAL_FUNC(_ask_delete_completed_check_),
					   AskDeleteCompleted);
		AskDeleteCompleted->set_modal(MainWindow);
	} else
		del_completed_downloads();
};

static void _ask_delete_failed_check_(GtkWidget *widget, tConfirmedDialog *parent){
	CFG.CONFIRM_DELETE_FATALED=!(GTK_TOGGLE_BUTTON(parent->check)->active);
	del_fataled_downloads();
};

void ask_delete_fataled_downloads(...) {
	if (CFG.CONFIRM_DELETE_FATALED) {
		if (!AskDeleteFataled) AskDeleteFataled=new tConfirmedDialog;
		if (AskDeleteFataled->init(_("Do you wish delete failed downloads?"),_("Delete?")))
			gtk_signal_connect(GTK_OBJECT(AskDeleteFataled->ok_button),
					   "clicked",
					   GTK_SIGNAL_FUNC(_ask_delete_failed_check_),
					   AskDeleteFataled);
		AskDeleteFataled->set_modal(MainWindow);
	} else
		del_fataled_downloads();
};

void delete_downloads(gint flag) {
	D4X_QUEUE->qv.delete_downloads(flag);
	if (AskDelete) AskDelete->done();
	aa.try_to_run_wait(D4X_QUEUE);
};

void continue_downloads(...) {
	D4X_QUEUE->qv.continue_downloads();
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
	gtk_widget_set_usize(ProgressOfDownload,-1,-1);
	gtk_progress_set_format_string (GTK_PROGRESS (ProgressOfDownload),
	                                "0%%(%v/%u)");
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
	tDownload *temp=D4X_QUEUE->qv.last_selected();
	GtkAdjustment *adj=GTK_PROGRESS(ProgressOfDownload)->adjustment;
	char data[MAX_LEN];
	if (adj){
		if(temp && (temp->finfo.size>0 || temp->Size.curent>0)){
			char b[100];
			d4x_percent_str(temp->Percent, b, sizeof(b));
			sprintf(data, "%s%%%%(%%v/%%u)",b);
			adj->lower=0;
			adj->upper = temp->finfo.size>temp->Size.curent ? temp->finfo.size : temp->Size.curent;
			gtk_progress_set_value(GTK_PROGRESS(ProgressOfDownload),temp->Size.curent);
			gtk_progress_set_format_string (GTK_PROGRESS (ProgressOfDownload),
							data);
			gtk_progress_set_show_text(GTK_PROGRESS(ProgressOfDownload),TRUE);
		}else{
			adj->lower=0;
			adj->upper = 1;
			gtk_progress_set_value(GTK_PROGRESS(ProgressOfDownload),1);
			gtk_progress_set_show_text(GTK_PROGRESS(ProgressOfDownload),FALSE);
		};
	};
	gtk_widget_show(ProgressOfDownload);
	char data1[MAX_LEN];
	make_number_nicel(data,GVARS.READED_BYTES,D4X_QUEUE->NICE_DEC_DIGITALS);
	sprintf(data1,"%s(%iB/s)",data,GlobalMeter->last_speed());
	gtk_statusbar_pop(GTK_STATUSBAR(ReadedBytesStatusBar),RBStatusBarContext);
	gtk_statusbar_push(GTK_STATUSBAR(ReadedBytesStatusBar),RBStatusBarContext,data1);
};
/* ******************************************************************** */
void d4x_normalize_coords(gint *x,gint *y,gint width=0,gint heigh=0){
	int temp,w,h;
	gdk_window_get_geometry((GdkWindow *)NULL,&temp,&temp,&w,&h,&temp);
	if (*x+width<w && *y+heigh<h && *x+width>0 && *y+width>0) return;
	if (*x<0){
		while (*x<0)
			*x+=w;
	}else{
		while (*x>w)
			*x-=w;
	};
	if (*y<0){
		while (*y<0)
			*y+=h;
	}else{
		while (*y>h)
			*y-=h;
	};
};

static void cb_page_size( GtkAdjustment *get) {
	if (get==NULL) return;
	if (main_log_value==get->value && get->value<get->upper-get->page_size) {
		//added 0.01 to prevent interface lockups
		get->value=get->upper-get->page_size;
		main_log_value=get->value;
		gtk_signal_emit_by_name (GTK_OBJECT (get), "value_changed");
	} else
		main_log_value=get->value;
}

/******************************************************************
    This part of code for DnD (Drag-n-Drop) support added by
		     Justin Bradford
 ******************************************************************/

// for drag-drop support

// drop handler
// define a target entry listing the mime-types we'll acknowledge
GtkTargetEntry download_drop_types[] = {
	{ "x-url/http",		0, TARGET_URL},
	{ "x-url/ftp",		0, TARGET_URL},
	{ "_NETSCAPE_URL",	0, TARGET_URL},
	{ "x-url/*",		0, TARGET_URL},
	{ "text/uri-list",	0, TARGET_URL},
	{ "text/plain",		0, TARGET_DND_TEXT },
	{ "text/html", 		0, TARGET_DND_TEXT }
};

// calculate the number of mime-types listed
gint n_download_drop_types = sizeof(download_drop_types) / sizeof(download_drop_types[0]);

/*********************************************************************
    End of first part of DnD's code
 *********************************************************************/

/**********************************************************
    Handler for DnD event
 **********************************************************/
// this the drag-drop even handler
// just add the url, and assume default download location
void list_dnd_drop_internal(GtkWidget *widget,
			    GdkDragContext *context,
			    gint x, gint y,
			    GtkSelectionData *selection_data,
			    guint info, guint time) {
	g_return_if_fail(widget!=NULL);

	switch (info) {
		// covers all single URLs
		// a uri-list mime-type will need special handling
	case TARGET_DND_TEXT:
	case TARGET_URL:{
		// make sure our url (in selection_data->data) is good
		/*
		printf("%s\n",gdk_atom_name(selection_data->type));
		printf("%s\n",gdk_atom_name(selection_data->target));
		printf("%s\n",gdk_atom_name(selection_data->selection));
		*/
		if (selection_data->data != NULL) {
			int is_dnd_basket=0;
			if (!GTK_IS_SCROLLED_WINDOW(widget)){
				dnd_trash_animation();
				is_dnd_basket=1;
			};
			SOUND_SERVER->add_event(SND_DND_DROP);
			int len = strlen((char*)selection_data->data);
			if (len && selection_data->data[len-1] == '\n')
				selection_data->data[len-1] = 0;
			// add the new download
			char *str = (char*)selection_data->data;
			int sbd=0;//should be deleted flag
			char *ent=index(str,'\n');
			if (ent) *ent=0;
			unsigned char *a=(unsigned char *)str;
			while (*a){ // to avoid invalid signs
				if (*a<' '){
					*a=0;
					break;
				};
				a++;
			};
			/* check for gmc style URL */
			const char *gmc_url="file:/#ftp:";
			if (begin_string((char*)selection_data->data,gmc_url)){
				str = sum_strings("ftp://",
						  (char*)selection_data->data + strlen(gmc_url),
						  (char*)NULL);
				sbd=1;
			};
			char *desc=ent?ent+1:(char *)NULL;
			d4xDownloadQueue *tmpq=D4X_QUEUE;
			if (is_dnd_basket && dnd_trash_target_queue)
				D4X_QUEUE=dnd_trash_target_queue;
			if (CFG.NEED_DIALOG_FOR_DND){
				init_add_dnd_window(str,desc);
			}else{
				aa.add_downloading(str, (char*)CFG.GLOBAL_SAVE_PATH,(char*)NULL,desc);
			};
			D4X_QUEUE=tmpq;
			if (sbd) delete[] str;
		}
	}
	}
}

/**********************************************************
    End of handler for DnD 
 **********************************************************/

void list_of_downloads_allocation(GtkWidget *paned,GtkAllocation *allocation){
	if (MAIN_PANED_HEIGHT && allocation->height!=MAIN_PANED_HEIGHT){
		float ratio=(float)CFG.WINDOW_CLIST_HEIGHT/(float)(MAIN_PANED_HEIGHT-GTK_PANED(MAIN_PANED)->gutter_size);
		CFG.WINDOW_CLIST_HEIGHT=int(ratio*(float)(allocation->height-GTK_PANED(MAIN_PANED)->gutter_size));
		lod_set_height();
	};
	MAIN_PANED_HEIGHT=allocation->height;
	lod_get_height();
};

void init_main_window() {
	GtkWidget *hbox=gtk_hbox_new(FALSE,1);
	MainHBox=hbox;
	gtk_box_pack_start (GTK_BOX (hbox), MainStatusBar, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), ProgressOfDownload, TRUE, TRUE, 0);
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

	FSearchCList=fs_list_init();
	ContainerForCList=gtk_scrolled_window_new((GtkAdjustment *)NULL,(GtkAdjustment *)NULL);
	GtkWidget *hpaned=gtk_vpaned_new();
	GtkWidget *vpaned=gtk_hpaned_new();
	MAIN_PANED=hpaned;
	MAIN_PANED1=gtk_hpaned_new();
	MAIN_PANED2=vpaned;
	main_log_adj = (GtkAdjustment *)gtk_adjustment_new (0.0, 0.0, 0.0, 0.1, 1.0, 1.0);
	main_log_value=0.0;
	gtk_signal_connect (GTK_OBJECT (main_log_adj), "changed",
	                    GTK_SIGNAL_FUNC (cb_page_size), NULL);
	GtkWidget *scroll_window=gtk_scrolled_window_new((GtkAdjustment *)NULL,main_log_adj);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll_window),MainLogList);
	GtkWidget *scroll_window2=gtk_scrolled_window_new((GtkAdjustment *)NULL,(GtkAdjustment *)NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window2),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	D4X_QVT=new d4xQsTree;
	D4X_QVT->init();
	gtk_container_add(GTK_CONTAINER(scroll_window2),GTK_WIDGET(D4X_QVT->tree));
	gtk_paned_add1(GTK_PANED(MAIN_PANED1),scroll_window2);
	gtk_paned_add2(GTK_PANED(MAIN_PANED1),ContainerForCList);
	gtk_paned_add1(GTK_PANED(hpaned),MAIN_PANED1);
	gtk_paned_add2(GTK_PANED(hpaned),vpaned);
	gtk_paned_add1(GTK_PANED(vpaned),scroll_window);
	
	GtkWidget *scroll_window1=gtk_scrolled_window_new((GtkAdjustment *)NULL,(GtkAdjustment *)NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window1),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll_window1),(GtkWidget *)FSearchCList);
	gtk_paned_add2(GTK_PANED(vpaned),scroll_window1);
	
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

	gtk_widget_show(ContainerForCList);
	gtk_widget_show(MainLogList);
	gtk_widget_show((GtkWidget *)FSearchCList);
	gtk_widget_show(scroll_window);
	gtk_widget_show(scroll_window1);
	gtk_widget_show_all(scroll_window2);
	gtk_widget_show(MAIN_PANED1);
	gtk_widget_show(hpaned);
	gtk_widget_show(vpaned);
	gtk_widget_show(MainMenu);
	gtk_widget_show(hbox);
	gtk_widget_show(vbox);
	gtk_widget_show(MainWindow);
	if (!CFG.DONOTSET_WINPOS)
		gdk_window_move_resize(MainWindow->window,
				       gint(CFG.WINDOW_X_POSITION),
				       gint(CFG.WINDOW_Y_POSITION),
				       gint(CFG.WINDOW_WIDTH),
				       gint(CFG.WINDOW_HEIGHT));
	gtk_signal_connect (GTK_OBJECT (MAIN_PANED), "size_allocate",
	                    GTK_SIGNAL_FUNC (list_of_downloads_allocation), NULL);
	gtk_signal_connect (GTK_OBJECT (MAIN_PANED2), "size_allocate",
	                    GTK_SIGNAL_FUNC (fs_list_allocation), NULL);


	/****************************************************************
	  Initing signals' handlers for DnD support (added by Justin Bradford)
	 ****************************************************************/
	// connect the drag-drop signal
	gtk_signal_connect(GTK_OBJECT(ContainerForCList),
	                   "drag_data_received",
	                   GTK_SIGNAL_FUNC(list_dnd_drop_internal),
	                   NULL);
	// set the list container as a drop destination
	gtk_drag_dest_set(GTK_WIDGET(ContainerForCList),
	                  (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION |
	                                    GTK_DEST_DEFAULT_HIGHLIGHT |
	                                    GTK_DEST_DEFAULT_DROP),
	                  download_drop_types, n_download_drop_types,
	                  (GdkDragAction)(GDK_ACTION_COPY|GDK_ACTION_MOVE));

	/****************************************************************
	    End of second part of DnD code
	 ****************************************************************/
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
		tDownload *temp=D4X_QUEUE->qv.last_selected();
		char data[MAX_LEN];
		char data2[MAX_LEN];
		char data3[MAX_LEN];
		if (temp){
			make_number_nice(data2,temp->Size.curent,D4X_QUEUE->NICE_DEC_DIGITALS);
			if (temp->finfo.size>=0)
				make_number_nice(data3,temp->finfo.size,D4X_QUEUE->NICE_DEC_DIGITALS);
			else
				sprintf(data3,"???");
			char b[100];
			d4x_percent_str(temp->Percent,b,sizeof(b));
			sprintf(data,"%s%% %s/%s %s ",b,data2,data3,temp->info->file.get());
			dnd_trash_set_tooltip(data);
		}else
			dnd_trash_set_tooltip(NULL);
		if (temp && (CFG.USE_MAINWIN_TITLE2==0 || UpdateTitleCycle % 3)) {
			tmp_scroll_title(data,ROLL_STAT);
			gtk_window_set_title(GTK_WINDOW (MainWindow), data);
		} else {
			if (CFG.USE_MAINWIN_TITLE2) {
				sprintf(data,_("[%s] %i-running %i-completed %i-total "),
					D4X_QUEUE->name.get(),
					D4X_QUEUE->count(DL_RUN),
					D4X_QUEUE->count(DL_COMPLETE),
					D4X_QUEUE->count());
				tmp_scroll_title(data,ROLL_INFO);
				gtk_window_set_title(GTK_WINDOW (MainWindow), data);
			} else{
				gtk_window_set_title(GTK_WINDOW (MainWindow), VERSION_NAME);
			};
		};
	} else{
		if (mainwin_title_state){
			gtk_window_set_title(GTK_WINDOW (MainWindow), VERSION_NAME);
			dnd_trash_set_tooltip(NULL);
		};
		mainwin_title_state=0;
	};
};

int time_for_refresh(void *a) {
	D4X_QUEUE->qv.freeze();
	aa.main_circle();
	D4X_QUEUE->qv.unfreeze();
	update_progress_bar();
	update_mainwin_title();
	UpdateTitleCycle+=1;
	my_gtk_graph_recalc(GLOBAL_GRAPH);
	return 1;
};

static int _nano_step_=0;

int time_for_logs_refresh(void *a) {
	if (_nano_step_)
		aa.main_circle_nano1();
	else
		aa.main_circle_nano2();
	_nano_step_=~_nano_step_;
	aa.redraw_logs();
	aa.check_for_remote_commands();
	MainTimer-=1;
	if (MainTimer==0) {
		time_for_refresh(NULL);
		MainTimer=(GLOBAL_SLEEP_DELAY*1000)/100;
	};
	return 1;
};

int get_mainwin_sizes(GtkWidget *window) {
	gint x,y,w,h;
	if (FirstConfigureEvent && !CFG.DONOTSET_WINPOS) {
		gdk_window_get_root_origin (window->window, &x, &y);
		FirstConfigureEvent=0;
		if (y!=CFG.WINDOW_Y_POSITION ||
		    x!=CFG.WINDOW_X_POSITION){
			gdk_window_move(MainWindow->window,
					gint(CFG.WINDOW_X_POSITION+(CFG.WINDOW_X_POSITION-x)),
					gint(CFG.WINDOW_Y_POSITION+
					     (CFG.WINDOW_Y_POSITION-y)));
//			FirstConfigureEvent-=1;
		};
		return FALSE;
	};
	FirstConfigureEvent=0;
	if (window!=NULL && window->window!=NULL &&
	    gdk_window_is_visible(window->window)) {
//		gdk_window_get_position(window->window,&x,&y);
//		gdk_window_get_origin (window->window, &x, &y);
		gdk_window_get_root_origin (window->window, &x, &y);
//		gdk_window_get_deskrelative_origin(window->window, &x, &y);
//		printf("%i %i\n--\n",x,y);
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
			if (string_ended_uncase(extension,buf)==0){
				delete[] extension;
				return 1;
			};
		}while(temp!=NULL && strlen(temp)>0);
		delete[] extension;
		return 0;
	};
	extension=new char[strlen(CFG.SKIP_IN_CLIPBOARD)+1];
	temp=CFG.SKIP_IN_CLIPBOARD;
	do{
		temp=extract_string(temp,extension);
		if (string_ended_uncase(extension,buf)==0){
			delete[] extension;
			return 0;
		};
	}while(temp!=NULL && strlen(temp)>0);
	delete[] extension;
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
				init_add_dnd_window(buf,NULL);
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
	if (CFG.EXIT_COMPLETE && d4x_run_or_wait_downloads()==0){
		EXIT_COMPLETE_INTERVAL-=1;
		if (EXIT_COMPLETE_INTERVAL<0  &&
		    (list_for_adding==NULL || list_for_adding->count()==0)){
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

void main_window_resize(int w,int h){
	if (MainWindow){
		FirstConfigureEvent=0;
		w=w<400?400:w;
		h=h<200?200:h;
		gdk_window_resize(MainWindow->window,w,h);
	};
};

void main_window_move(int x,int y){
	if (MainWindow){
		FirstConfigureEvent=0;
		gdk_window_move(MainWindow->window,x,y);
	};
};

static char *D4X_CLIPBOARD=NULL;

char *d4x_mw_clipboard_get(){
	if (gdk_selection_owner_get(GDK_SELECTION_PRIMARY)==MainWindow->window){
		return(D4X_CLIPBOARD);
	};
	return(NULL);
};

void d4x_mw_clipboard_set(char *str){
	if (D4X_CLIPBOARD) delete[] D4X_CLIPBOARD;
	D4X_CLIPBOARD=copy_string(str);
	gtk_selection_owner_set (MainWindow,
				 GDK_SELECTION_PRIMARY,
				 GDK_CURRENT_TIME);
};

enum {
  TARGET_STRING,
  TARGET_TEXT,
  TARGET_COMPOUND_TEXT
};

static void d4x_mw_selection_get (GtkWidget        *widget,
				  GtkSelectionData *selection_data,
				  guint             info,
				  guint             time){
	gchar *str;
	gint length;
	
	DBC_RETURN_IF_FAIL(widget != NULL);  
	if (!D4X_CLIPBOARD) return;
	str = copy_string(D4X_CLIPBOARD);
	length = strlen (str);
	if (info == TARGET_STRING){
		gtk_selection_data_set (selection_data,
					GDK_SELECTION_TYPE_STRING,
					8*sizeof(gchar),
					(guchar *)str,
					length);
	}else{
		if ((info == TARGET_TEXT) || (info == TARGET_COMPOUND_TEXT)){
			guchar *text;
			gchar c;
			GdkAtom encoding;
			gint format;
			gint new_length;
			
			c = str[length];
			str[length] = '\0';
			gdk_string_to_compound_text (str, &encoding, &format, &text, &new_length);
			gtk_selection_data_set (selection_data, encoding, format, text, new_length);
			gdk_free_compound_text (text);
			str[length] = c;
		}
	};
	g_free (str);
}

static void d4x_mw_set_targets(){
	static const GtkTargetEntry targets[] = {
		{ "STRING", 0, TARGET_STRING },
		{ "TEXT",   0, TARGET_TEXT }, 
		{ "COMPOUND_TEXT", 0, TARGET_COMPOUND_TEXT }
	};
	static const gint n_targets = sizeof(targets) / sizeof(targets[0]);
	gtk_selection_add_targets (MainWindow,
				   GDK_SELECTION_PRIMARY,
				   targets, n_targets);
};


void init_face(int argc, char *argv[]) {
	gtk_set_locale();
	gtk_init(&argc, &argv);
	gdk_rgb_init();
	d4x_normalize_coords(&(CFG.WINDOW_X_POSITION),&(CFG.WINDOW_Y_POSITION),CFG.WINDOW_WIDTH,CFG.WINDOW_HEIGHT);
	MainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_wmclass(GTK_WINDOW(MainWindow),"D4X_Main", "D4X");
	d4x_mw_set_targets();
	if (!CFG.DONOTSET_WINPOS)
		gtk_widget_set_uposition(MainWindow,
					 gint(CFG.WINDOW_X_POSITION),
					 gint(CFG.WINDOW_Y_POSITION));
	gtk_window_set_default_size(GTK_WINDOW(MainWindow),gint(CFG.WINDOW_WIDTH),gint(CFG.WINDOW_HEIGHT));
	gtk_window_set_title(GTK_WINDOW (MainWindow), VERSION_NAME);
	gtk_widget_set_usize( GTK_WIDGET (MainWindow), 400, 200);
	gtk_widget_realize(MainWindow);
	MainWindowGC=gdk_gc_new(MainWindow->window);

        dnd_trash_init_menu();
	init_main_menu();
	init_list_menu();
	init_status_bar();
	init_buttons_bar();
	init_main_window();
	init_pixmaps_for_log();
/* initing table of shifts
 */
	for (int i=0;i<ROLL_LAST;i++)
		ScrollShift[i]=0;
	FaceForPasswords=new tFacePass;
	FaceForPasswords->load();
#include "pixmaps/main.xpm"
	GdkBitmap *bitmap;
	GdkPixmap *pixmap=make_pixmap_from_xpm(&bitmap,main_xpm);
	gdk_window_set_icon(MainWindow->window,(GdkWindow *)NULL,pixmap,bitmap);
	gtk_signal_connect(GTK_OBJECT(MainWindow), "delete_event",
	                   GTK_SIGNAL_FUNC(ask_exit2),
	                   NULL);
	gtk_signal_connect(GTK_OBJECT(MainWindow), "selection_get",
			   GTK_SIGNAL_FUNC(d4x_mw_selection_get),NULL);
	gtk_signal_connect(GTK_OBJECT(MainWindow), "selection_received",
			   GTK_SIGNAL_FUNC(my_get_xselection),NULL);
/*
	gtk_signal_connect(GTK_OBJECT(MainWindow), "selection_clear_event",
			   GTK_SIGNAL_FUNC(d4x_mw_selection_clear),NULL);
*/
	gtk_signal_connect (GTK_OBJECT (MainWindow),
			    "key_press_event",
			    GTK_SIGNAL_FUNC (main_menu_prepare),
			    NULL);
	lod_init_pixmaps();
	lod_set_height();
	if (CFG.DND_TRASH){
		dnd_trash_init();
		if (CFG.HIDE_MAIN_WINDOW) main_window_toggle();
	};
	main_log_mask=0;
};
