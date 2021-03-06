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
#define GTK_DISABLE_DEPRECATED

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
#include <ctype.h>
#include "themes.h"
#include "eggtrayicon.h"
#include <sstream>

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
GtkTreeView *FSearchView,*FSearchView2;
GtkWidget *BoxForGraph;
//GtkItemFactory *main_menu_item_factory=NULL;
GtkUIManager *main_menu_ui_manager=NULL;
GtkWidget *MainLogList,*MAIN_PANED=(GtkWidget *)NULL,*MAIN_PANED2=(GtkWidget *)NULL;
GtkWidget *MAIN_PANED1=(GtkWidget *)NULL;
d4xQsTree *D4X_QVT;
int main_log_mask;
unsigned int ScrollShift[2];
int mainwin_title_state;
gfloat main_log_value;
GtkAdjustment *main_log_adj;

GtkWidget *D4X_TOOL_ONE;
GtkWidget *D4X_TOOL_TWO;
GtkWidget *D4X_TOOL_THREE;
GtkWidget *D4X_TOOL_CURRENT;
GtkWidget *D4X_TOOL_CONTAINER;
GtkWidget *D4X_TOOL_UP;
GtkWidget *D4X_TOOL_DOWN;
GtkWidget *D4X_TOOL_BUTTON_FIND;
GtkWidget *D4X_VTOOLBAR_CONTAINER;
GtkWidget *D4X_OFFLINE_BUTTON;
d4x::Theme::SlaveImage *D4X_OFFLINE_IMAGE;

d4xDisplayLogInfo D4X_LOG_DISPLAY;

//GtkItemFactory *list_menu_itemfact;

tConfirmedDialog *AskDelete=(tConfirmedDialog *)NULL;
tConfirmedDialog *AskDeleteCompleted=(tConfirmedDialog *)NULL;
tConfirmedDialog *AskDeleteFataled=(tConfirmedDialog *)NULL;
tConfirmedDialog *AskExit=(tConfirmedDialog *)NULL;

gint StatusBarContext,RBStatusBarContext;
int LogsTimer,GraphTimer,ListTimer;
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
	MM_QUEUE,MM_QUEUE_NQ,MM_QUEUE_NSQ,MM_QUEUE_REMOVE,MM_QUEUE_PROP,
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
	"queue",
	"newqueue",
	"newsubqueue",
	"removequeue",
	"queueproperties",
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

void d4x_load_accelerators();

char *old_clipboard_content(){
	if (CFG.CLIPBOARD_MONITOR)
		return(OLD_CLIPBOARD_CONTENT);
	return(NULL);
};


static void d4x_filters_tool_switch(){
	GtkWidget *w=d4x_filters_window_init();
	if (D4X_TOOL_CURRENT!=w){
		gtk_container_remove(GTK_CONTAINER(D4X_TOOL_CONTAINER),D4X_TOOL_CURRENT);
		gtk_container_add(GTK_CONTAINER (D4X_TOOL_CONTAINER),w);
		D4X_TOOL_CURRENT=w;
		gtk_widget_show_all(D4X_TOOL_CURRENT);
	};
};

static void d4x_scheduler_tool_switch(){
	GtkWidget *w=d4x_scheduler_init();
	if (D4X_TOOL_CURRENT!=w){
		gtk_container_remove(GTK_CONTAINER(D4X_TOOL_CONTAINER),D4X_TOOL_CURRENT);
		gtk_container_add(GTK_CONTAINER (D4X_TOOL_CONTAINER),w);
		D4X_TOOL_CURRENT=w;
		gtk_widget_show_all(D4X_TOOL_CURRENT);
	};
};

static void open_passwords_window(...) {
	GtkWidget *w=FaceForPasswords->init();
	if (D4X_TOOL_CURRENT!=w){
		gtk_container_remove(GTK_CONTAINER(D4X_TOOL_CONTAINER),D4X_TOOL_CURRENT);
		gtk_container_add(GTK_CONTAINER (D4X_TOOL_CONTAINER),w);
		D4X_TOOL_CURRENT=w;
		gtk_widget_show_all(D4X_TOOL_CURRENT);
	};
};

/*
void util_item_factory_popup(GtkItemFactory *ifactory,guint x, guint y,guint mouse_button,guint32 time) {
	static GQuark quark_user_menu_pos=0;
	struct Pos {
		gint x;
		gint y;
	}
	*pos;

	if(!quark_user_menu_pos)
		quark_user_menu_pos=g_quark_from_static_string("user_menu_pos");
	pos=(Pos *)g_object_get_qdata(G_OBJECT(ifactory),quark_user_menu_pos);
	if(!pos) {
		pos=(Pos *)g_malloc0(sizeof(struct Pos));
		g_object_set_qdata_full(G_OBJECT(ifactory->widget),quark_user_menu_pos,pos,g_free);
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

*/

void _rerun_failed_downloads(){
	_aa_.rerun_failed();
};

static bool _no_speed_callback_=false;

void main_menu_speed_callback(GtkAction *action){
	if (_no_speed_callback_) return;
	_no_speed_callback_=true;
	CFG.SPEED_LIMIT=gtk_radio_action_get_current_value (GTK_RADIO_ACTION (action))+1;
	set_speed_buttons();
	_no_speed_callback_=false;
};

void main_menu_speed_prepare(){
	if (_no_speed_callback_) return;
	_no_speed_callback_=true;
	switch (CFG.SPEED_LIMIT){
	case 1:{
		GtkAction *action1=gtk_ui_manager_get_action(main_menu_ui_manager,"/MainMenu/Options/Speed/speedlow");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action1),TRUE);
		break;
	};
	case 2:{
		GtkAction *action2=gtk_ui_manager_get_action(main_menu_ui_manager,"/MainMenu/Options/Speed/speedmedium");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action2),TRUE);
		break;
	};
	default:{
		GtkAction *action3=gtk_ui_manager_get_action(main_menu_ui_manager,"/MainMenu/Options/Speed/speedunlim");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action3),TRUE);
	};
	};
	_no_speed_callback_=false;
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
/* FIXME: GTK2.4
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
//                      adding tooltip
			GtkWidget *menu_item=gtk_item_factory_get_widget_by_action(main_menu_item_factory,
										   tmp.callback_action);
			if (menu_item){
				GtkTooltips *tooltip=gtk_tooltips_new();
				gtk_tooltips_force_window(tooltip);
				GtkStyle *current_style=gtk_style_copy(gtk_widget_get_style(tooltip->tip_window));
				current_style->bg[GTK_STATE_NORMAL] = LYELLOW;
				gtk_widget_set_style(tooltip->tip_window, current_style);
				gtk_tooltips_set_tip(tooltip,menu_item,str->body,(const gchar *)NULL);
				gtk_tooltips_enable(tooltip);
			};
//			 modify path for deleting it successfully
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
				       "<control>Q",
				       (GtkItemFactoryCallback)ask_exit,
				       0,
				       (gchar *)NULL};
	gtk_item_factory_create_item(main_menu_item_factory,
				     &sep_item,NULL,1);
	gtk_item_factory_create_item(main_menu_item_factory,
				     &exit_item,NULL,1);
*/
};

static gint main_menu_enable_all(){
	GtkWidget *menu_item;
	char *menu_path[]={
		"/MainMenu/Download/viewlog",
		"/MainMenu/Download/stop",
		"/MainMenu/Download/edit",
		"/MainMenu/Download/delete",
		"/MainMenu/Download/continue",
		"/MainMenu/Download/protect",
		"/MainMenu/Download/unselectall",
		"/MainMenu/Download/selectall",
		"/MainMenu/Download/invert"};
	if (D4X_QUEUE->qv.last_selected){
		for (int i=0;i<sizeof(menu_path)/sizeof(char*);i++){
			menu_item=gtk_ui_manager_get_widget(main_menu_ui_manager,menu_path[i]);
			if (menu_item) gtk_widget_set_sensitive(menu_item,TRUE);
		};
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


static void _mm_queue_menu_nq(){
	D4X_QVT->create_init();
};
static void _mm_queue_menu_nsq(){
	D4X_QVT->create_init(1);
};
static void _mm_queue_menu_remove(){
	D4X_QVT->delete_queue();
};
static void _mm_queue_menu_prop(){
	D4X_QVT->prefs_init();
};

static const char *main_menu_ui_info=
"<menubar name='MainMenu'>"
"  <menu action='File'>"
"     <menuitem action='save' />"
"     <menuitem action='load' />"
"     <menuitem action='findlinks' />"
"     <separator action='separator1' />"
"     <menuitem action='newitem' />"
"     <menuitem action='pasteitem' />"
"     <menuitem action='autoadd' />"
"     <separator action='separator2' />"
"     <menuitem action='quit' />"
"  </menu>"
"  <menu action='Download'>"
"     <menuitem action='viewlog' />"
"     <menuitem action='stop' />"
"     <menuitem action='edit' />"
"     <menuitem action='delete' />"
"     <menuitem action='continue' />"
"     <separator action='separator3' />"
"     <menuitem action='delcompleted' />"
"     <menuitem action='delfailed' />"
"     <menuitem action='rerunfailed' />"
"     <menuitem action='protect' />"
"     <separator action='separator4' />"
"     <menuitem action='unselectall' />"
"     <menuitem action='selectall' />"
"     <menuitem action='invert' />"
"  </menu>"
"  <menu action='Queue'>"
"     <menuitem action='createqueue' />"
"     <menuitem action='createsubqueue' />"
"     <menuitem action='removequeue' />"
"     <menuitem action='queueprop' />"
"  </menu>"
"  <menu action='Options'>"
"     <menuitem action='scheduler' />"
"     <menuitem action='urlmanager' />"
"     <menuitem action='general' />"
"     <menuitem action='filters' />"
"     <menu action='Speed'>"
"         <menuitem action='speedlow' />"
"         <menuitem action='speedmedium' />"
"         <menuitem action='speedunlim' />"
"     </menu>"
"     <menuitem action='buttons' />"
"  </menu>"
"  <menu action='Help'>"
"     <menuitem action='about' />"
"  </menu>"
"</menubar>";

void init_main_menu() {
	d4x_load_accelerators();
	static GtkActionEntry entries[] = {
		{ "File", NULL, N_("_File") },
		{ "Download", NULL, N_("_Download") },
		{ "Queue", NULL, N_("_Queue") },
		{ "Options", NULL, N_("_Options") },
		{ "Speed", NULL, N_("Speed") },
		{ "Help", NULL, N_("_Help") },
		{ "save", GTK_STOCK_SAVE, N_("_Save List"), main_menu_kb[MM_FILE_SAVE], NULL,  G_CALLBACK(init_save_list)},
		{ "load", GTK_STOCK_OPEN, N_("_Load List"), main_menu_kb[MM_FILE_LOAD], NULL,  G_CALLBACK(init_load_list)},
		{ "findlinks", NULL, N_("_Find links in file"), main_menu_kb[MM_FILE_TXT], NULL,  G_CALLBACK(init_load_txt_list)},
		{ "newitem", GTK_STOCK_ADD, N_("_New Download"), main_menu_kb[MM_FILE_NEW], NULL,  G_CALLBACK(init_add_window)},
		{ "pasteitem",GTK_STOCK_PASTE, N_("_Paste Download"), main_menu_kb[MM_FILE_PASTE], NULL,  G_CALLBACK(init_add_clipboard_window)},
		{ "autoadd", NULL, N_("_Automated adding"), main_menu_kb[MM_FILE_AUTO], NULL,  G_CALLBACK(d4x_automated_add)},
		{ "quit", GTK_STOCK_QUIT, N_("_Exit"), main_menu_kb[MM_FILE_EXIT], NULL,  G_CALLBACK(ask_exit)},
		{ "viewlog", NULL, N_("_View Log"), main_menu_kb[MM_DOWNLOAD_LOG], NULL,  G_CALLBACK(mmenu_open_logs)},
		{ "stop", GTK_STOCK_STOP, N_("_Stop downloads"), main_menu_kb[MM_DOWNLOAD_STOP], NULL,  G_CALLBACK(stop_downloads)},
		{ "edit", GTK_STOCK_PROPERTIES, N_("_Edit downloads"), main_menu_kb[MM_DOWNLOAD_EDIT], NULL,  G_CALLBACK(open_edit_for_selected)},
		{ "delete", GTK_STOCK_REMOVE, N_("_Delete downloads"), main_menu_kb[MM_DOWNLOAD_DEL], NULL,  G_CALLBACK(ask_delete_download)},
		{ "continue", NULL, N_("_Continue downloads"), main_menu_kb[MM_DOWNLOAD_RUN], NULL,  G_CALLBACK(continue_downloads)},
		{ "delcompleted", NULL, N_("Delete Completed"), main_menu_kb[MM_DOWNLOAD_DEL_C], NULL,  G_CALLBACK(ask_delete_completed_downloads)},
		{ "delfailed", NULL, N_("Delete _Failed"), main_menu_kb[MM_DOWNLOAD_DEL_F], NULL,  G_CALLBACK(ask_delete_fataled_downloads)},
		{ "rerunfailed", NULL, N_("_Rerun failed"), main_menu_kb[MM_DOWNLOAD_RERUN], NULL,  G_CALLBACK(_rerun_failed_downloads)},
		{ "protect", NULL, N_("(Un)Protect"), main_menu_kb[MM_DOWNLOAD_PROTECT], NULL,  G_CALLBACK(lm_inv_protect_flag)},
		{ "unselectall", NULL, N_("_Unselect All"), main_menu_kb[MM_DOWNLOAD_UNSELECT_ALL], NULL,  G_CALLBACK(mmenu_unselect_all)},
		{ "selectall", NULL, N_("Select _All"), main_menu_kb[MM_DOWNLOAD_SELECT_ALL], NULL,  G_CALLBACK(mmenu_select_all)},
		{ "invert", NULL, N_("_Invert selection"), main_menu_kb[MM_DOWNLOAD_INVERT], NULL,  G_CALLBACK(mmenu_invert_selection)},
		{ "createqueue", GTK_STOCK_ADD, N_("_Create new queue"), main_menu_kb[MM_QUEUE_NQ], NULL,  G_CALLBACK(_mm_queue_menu_nq)},
		{ "createsubqueue", NULL, N_("Create new _Subqueue"), main_menu_kb[MM_QUEUE_NSQ], NULL,  G_CALLBACK(_mm_queue_menu_nsq)},
		{ "removequeue", GTK_STOCK_REMOVE, N_("_Remove this queue"), main_menu_kb[MM_QUEUE_REMOVE], NULL,  G_CALLBACK(_mm_queue_menu_remove)},
		{ "queueprop", GTK_STOCK_PROPERTIES, N_("_Properties"), main_menu_kb[MM_QUEUE_PROP], NULL,  G_CALLBACK(_mm_queue_menu_prop)},
		{ "scheduler", NULL, N_("_Scheduler"), main_menu_kb[MM_OPTIONS_SCHEDULER], NULL,  G_CALLBACK(d4x_scheduler_tool_switch)},
		{ "urlmanager", NULL, N_("_URL-manager"), main_menu_kb[MM_OPTIONS_PASSWORDS], NULL,  G_CALLBACK(open_passwords_window)},
		{ "general", GTK_STOCK_PREFERENCES, N_("_General"), main_menu_kb[MM_OPTIONS_COMMON], NULL,  G_CALLBACK(d4x_prefs_init)},
		{ "filters", NULL, N_("_Filters"), main_menu_kb[MM_OPTIONS_FILTERS], NULL,  G_CALLBACK(d4x_filters_tool_switch)},
		{ "buttons", NULL, N_("_Buttons"), main_menu_kb[MM_OPTIONS_BUTTONS], NULL,  G_CALLBACK(buttons_configure)},
		{ "about", GTK_STOCK_HELP, N_("_About"), main_menu_kb[MM_HELP_ABOUT], NULL,  G_CALLBACK(init_about_window)}
	};
	static GtkRadioActionEntry radio_entries[] = {
		{ "speedlow", NULL, N_("_Low"), NULL, NULL, 0 },
		{ "speedmedium", NULL, N_("_Medium"), NULL, NULL, 1 },
		{ "speedunlim", NULL, N_("_Unlimited"), NULL, NULL, 2 }
	};

	GtkActionGroup *action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_set_translate_func(action_group,d4x_menu_translate_func,NULL,NULL);
	main_menu_ui_manager=gtk_ui_manager_new ();
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), NULL);
	gtk_action_group_add_radio_actions (action_group, radio_entries, G_N_ELEMENTS (radio_entries), CFG.SPEED_LIMIT-1, G_CALLBACK(main_menu_speed_callback), NULL);
	gtk_ui_manager_insert_action_group (main_menu_ui_manager, action_group, 0);
	GtkAccelGroup *accel_group = gtk_ui_manager_get_accel_group (main_menu_ui_manager);
	gtk_window_add_accel_group (GTK_WINDOW (MainWindow), accel_group);
	GError *error = NULL;
	if (!gtk_ui_manager_add_ui_from_string (main_menu_ui_manager, main_menu_ui_info, -1, &error))
	{
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	}
	MainMenu=gtk_ui_manager_get_widget (main_menu_ui_manager,"/MainMenu");
        /*
	main_menu_item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>",accel_group);
	gtk_item_factory_create_items(main_menu_item_factory, nmenu_items, menu_items, NULL);
	init_load_accelerators();
	MainMenu= gtk_item_factory_get_widget(main_menu_item_factory, "<main>");
	gtk_window_add_accel_group( GTK_WINDOW( MainWindow ), accel_group );
	*/
	g_signal_connect(G_OBJECT (MainMenu),
			   "button_press_event",
			   G_CALLBACK (main_menu_prepare),
			   NULL);
	/* to avoid losing accelerators key */
	g_signal_connect(G_OBJECT (MainMenu),
			   "deactivate",
			   G_CALLBACK (main_menu_enable_all),
			   NULL);
	for (int i=0;i<=MM_HELP_ABOUT;i++)
		if (main_menu_kb[i]){
			delete[] main_menu_kb[i];
			main_menu_kb[i]=NULL;
		};
};

void main_menu_completed_empty(){
	GtkWidget *menu_item=gtk_ui_manager_get_widget(main_menu_ui_manager,"/MainMenu/Download/delcompleted");
	if (menu_item)
		gtk_widget_set_sensitive(menu_item,FALSE);
	gtk_widget_set_sensitive(ListMenuArray[LM_DELC],FALSE);
	gtk_widget_set_sensitive(buttons_array[BUTTON_DEL_COMPLETED],FALSE);
	dnd_trash_set_del_completed(FALSE);
};

void main_menu_completed_nonempty(){
	GtkWidget *menu_item=gtk_ui_manager_get_widget(main_menu_ui_manager,"/MainMenu/Download/delcompleted");
	if (menu_item)
		gtk_widget_set_sensitive(menu_item,TRUE);
	gtk_widget_set_sensitive(ListMenuArray[LM_DELC],TRUE);
	gtk_widget_set_sensitive(buttons_array[BUTTON_DEL_COMPLETED],TRUE);
	dnd_trash_set_del_completed(TRUE);
};

void main_menu_failed_empty(){
	GtkWidget *menu_item=gtk_ui_manager_get_widget(main_menu_ui_manager,"/MainMenu/Download/delfailed");
	if (menu_item)
		gtk_widget_set_sensitive(menu_item,FALSE);
	menu_item=gtk_ui_manager_get_widget(main_menu_ui_manager,"/MainMenu/Download/rerunfailed");
	if (menu_item)
		gtk_widget_set_sensitive(menu_item,FALSE);
	gtk_widget_set_sensitive(ListMenuArray[LM_DELF],FALSE);
};

void main_menu_failed_nonempty(){
	GtkWidget *menu_item=gtk_ui_manager_get_widget(main_menu_ui_manager,"/MainMenu/Download/delfailed");
	if (menu_item)
		gtk_widget_set_sensitive(menu_item,TRUE);
	menu_item=gtk_ui_manager_get_widget(main_menu_ui_manager,"/MainMenu/Download/rerunfailed");
	if (menu_item)
		gtk_widget_set_sensitive(menu_item,TRUE);
	gtk_widget_set_sensitive(ListMenuArray[LM_DELF],TRUE);
};

gint main_menu_prepare(){
	GtkWidget *menu_item;
	char *menu_path[]={
		"/MainMenu/Download/viewlog",
		"/MainMenu/Download/stop",
		"/MainMenu/Download/edit",
		"/MainMenu/Download/delete",
		"/MainMenu/Download/continue",
		"/MainMenu/Download/protect"};
	if (D4X_QUEUE->qv.last_selected){
		for (int i=0;i<sizeof(menu_path)/sizeof(char*);i++){
			menu_item=gtk_ui_manager_get_widget(main_menu_ui_manager,menu_path[i]);
			if (menu_item) gtk_widget_set_sensitive(menu_item,TRUE);
		};
	}else{
		for (int i=0;i<sizeof(menu_path)/sizeof(char*);i++){
			menu_item=gtk_ui_manager_get_widget(main_menu_ui_manager,menu_path[i]);
			if (menu_item) gtk_widget_set_sensitive(menu_item,FALSE);
		};
	};
	char *menu_path_sel[]={
		"/MainMenu/Download/unselectall",
		"/MainMenu/Download/selectall",
		"/MainMenu/Download/invert"};
	if (D4X_QUEUE->qv.rows()==0){
		for (int i=0;i<sizeof(menu_path_sel)/sizeof(char*);i++){
			menu_item=gtk_ui_manager_get_widget(main_menu_ui_manager,menu_path_sel[i]);
			if (menu_item) gtk_widget_set_sensitive(menu_item,FALSE);
		};
	}else{
		for (int i=0;i<sizeof(menu_path_sel)/sizeof(char*);i++){
			menu_item=gtk_ui_manager_get_widget(main_menu_ui_manager,menu_path_sel[i]);
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
/* FIXME: GTK2.4	
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
*/
};

static void d4x_lap(tQueue *q,char *menu,int name){
	char *s=sum_strings(menu," ",main_menu_kbnames[name],NULL);
	d4xXmlObject *xml=d4x_xml_find_obj(q,s);
	delete[] s;
	if (xml){
		s=xml->value.get();
		if (s && strlen(s)){
			int alt=0,ctrl=0,shift=0;
			if (strstr(s,"Ctrl+")) ctrl=1;
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
		main_menu_kb[MM_FILE_EXIT]=copy_string("<control>Q");
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
//		g_timeout_remove(ListTimer);
//		g_timeout_remove(LogsTimer);
//		g_timeout_remove(GraphTimer);
		D4X_QUEUE->qv.get_adj();
		CFG.CLIST_SHIFT=D4X_QUEUE->qv.current_shift;
		d4x_save_accelerators();
	};
	D4X_LOG_DISPLAY.log=NULL;
	save_list();
	save_config();
	_aa_.done();
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
		if (AskDelete) delete(AskDelete);
		if (AskDeleteCompleted) delete(AskDeleteCompleted);
		if (AskDeleteFataled) delete(AskDeleteFataled);
		if (AskExit) delete(AskExit);
		load_save_list_cancel();
	};
	for (int i=0;i<LAST_HISTORY;i++)
		delete(ALL_HISTORIES[i]);
	if (CFG.WITHOUT_FACE==0){
		gtk_main_quit();
	}else{
		_aa_.run_after_quit();
		var_free(&CFG);
		exit(0);
	};
};

static void _open_edit_foreach_(GtkTreeModel *model,GtkTreePath *path,
				GtkTreeIter *iter,gpointer data){
	GValue val={0,};
	gtk_tree_model_get_value(model,iter,NOTHING_COL,&val);
	tDownload *tmp=(tDownload *)g_value_peek_pointer(&val);
	g_value_unset(&val);
	init_edit_window(tmp);
};

void open_edit_for_selected(...) {
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(D4X_QUEUE->qv.ListOfDownloads));
	gtk_tree_selection_selected_foreach(sel,_open_edit_foreach_,NULL);
};

void del_completed_downloads(...) {
	_aa_.del_completed();
	if (AskDeleteCompleted) AskDeleteCompleted->done();
};

void del_fataled_downloads(...) {
	_aa_.del_fataled();
	if (AskDeleteFataled) AskDeleteFataled->done();
};

void stop_downloads(...) {
	D4X_QUEUE->qv.stop_downloads();
	prepare_buttons();
	_aa_.try_to_run_wait(D4X_QUEUE);
};

static void _my_main_quit_ask_exit_(GtkWidget *widget,tConfirmedDialog *parent){
	CFG.CONFIRM_EXIT=!(GTK_TOGGLE_BUTTON(parent->check)->active);
	my_main_quit();
};

void ask_exit(...) {
	if (CFG.CONFIRM_EXIT) {
		if (!AskExit) AskExit=new tConfirmedDialog;
		if (AskExit->init(_("Do you really want to quit?"),_("Quit?")))
			g_signal_connect(G_OBJECT(AskExit->ok_button),"clicked",
					   G_CALLBACK(_my_main_quit_ask_exit_),
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
			g_signal_connect(G_OBJECT(AskDelete->ok_button),
					   "clicked",
					   G_CALLBACK(a_delete_downloads),
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
			g_signal_connect(G_OBJECT(AskDeleteCompleted->ok_button),
					   "clicked",
					   G_CALLBACK(_ask_delete_completed_check_),
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
			g_signal_connect(G_OBJECT(AskDeleteFataled->ok_button),
					   "clicked",
					   G_CALLBACK(_ask_delete_failed_check_),
					   AskDeleteFataled);
		AskDeleteFataled->set_modal(MainWindow);
	} else
		del_fataled_downloads();
};

void delete_downloads(gint flag) {
	D4X_QUEUE->qv.delete_downloads(flag);
	if (AskDelete) AskDelete->done();
	_aa_.try_to_run_wait(D4X_QUEUE);
};

void continue_downloads(...) {
	GdkModifierType mask;
	gint x,y;
	gdk_window_get_pointer(MainWindow->window,&x,&y,&mask);
	D4X_QUEUE->qv.continue_downloads(mask & GDK_SHIFT_MASK);
	prepare_buttons();
};

/* ******************************************************************** */



/* ******************************************************************** */
void init_status_bar() {
	ProgressBarValues = (GtkAdjustment *) gtk_adjustment_new (0, 1, 0 , 0, 0, 0);
	ProgressOfDownload = gtk_progress_bar_new();
	/* Set the format of the string that can be displayed in the
	 * trough of the progress bar:
	 * %p - percentage
	 * %v - value
	 * %l - lower range value
	 * %u - upper range value */
	gtk_widget_set_size_request(ProgressOfDownload,-1,-1);
	MainStatusBar=gtk_statusbar_new();
	gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(MainStatusBar),FALSE);
	ReadedBytesStatusBar=gtk_statusbar_new();
	StatusBarContext=gtk_statusbar_get_context_id(
	                     GTK_STATUSBAR(MainStatusBar),"Main window context");
	RBStatusBarContext=gtk_statusbar_get_context_id(
	                       GTK_STATUSBAR(ReadedBytesStatusBar),"Readed Bytes");
	gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(ReadedBytesStatusBar),FALSE);
	gtk_statusbar_push(GTK_STATUSBAR(MainStatusBar),StatusBarContext,_("Ready to go ????"));
	gtk_statusbar_push(GTK_STATUSBAR(ReadedBytesStatusBar),RBStatusBarContext,"0");
	gtk_widget_set_size_request(ReadedBytesStatusBar,150,-1);
};


void update_progress_bar() {
	if (!D4X_QUEUE) return;
	tDownload *temp=D4X_QUEUE->qv.last_selected;
	char data[MAX_LEN];
	if(temp && (temp->finfo.size>0 || temp->Size>0)){
		//FIXME: std::string and boost::lexical_cast!
		char b[100];
		d4x_percent_str(temp->Percent, b, sizeof(b));
		sprintf(data, "%s%(%lli/%lli)",b,fsize_t(temp->Size),temp->finfo.size);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ProgressOfDownload),(temp->Percent>100?100:temp->Percent)/100.0);
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(ProgressOfDownload),data);
	}else{
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ProgressOfDownload),1);
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(ProgressOfDownload),NULL);
	};
	gtk_widget_show(ProgressOfDownload);
	char data1[MAX_LEN];
	sprintf(data1,"%s(%iB/s)",
		make_number_nice(GVARS.READED_BYTES,D4X_QUEUE->NICE_DEC_DIGITALS).c_str(),
		GlobalMeter->last_speed());
	gtk_statusbar_pop(GTK_STATUSBAR(ReadedBytesStatusBar),RBStatusBarContext);
	gtk_statusbar_push(GTK_STATUSBAR(ReadedBytesStatusBar),RBStatusBarContext,data1);
};

/* ******************************************************************** */

static void d4x_tray_clicked(GtkWidget *button, GdkEventButton *event, void *data){
	if (event->type==GDK_2BUTTON_PRESS){
		main_window_toggle();
	}else if (event->button==3){
		dnd_trash_menu_popdown(event);
	};
};

namespace d4x{
	
	class Tray{
		EggTrayIcon *tray;
		GtkWidget *box;
		GtkTooltips *tooltip;
	public:
		Tray():tray(0),box(0),tooltip(0){
#include "pixmaps/dndtrash_bar.xpm"
			EggTrayIcon *tray=egg_tray_icon_new("D4X");
			if (tray){
				box = gtk_event_box_new();
				tooltip=gtk_tooltips_new();
				gtk_tooltips_set_tip(tooltip,box,"Downloader for X",(const gchar *)NULL);
				gtk_tooltips_enable(tooltip);
				d4x::Theme::Image *img=new Theme::Image(dndtrash_bar_xpm,"main tray>file");
				d4x::CUR_THEME->monitor(img);
				gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(img->img));
				gtk_container_add(GTK_CONTAINER(tray), box);
				g_signal_connect(G_OBJECT(box), "button-press-event", G_CALLBACK(d4x_tray_clicked),this);
				gtk_widget_show_all(GTK_WIDGET(tray));
			};
		};
		void set_tooltip(const gchar *txt){
			if (box && tooltip)
				gtk_tooltips_set_tip(tooltip,box,txt?txt:"Downloader for X",NULL);
		};
	};
	static Tray *TRAY;
};

/*****************************************************************************/

void d4x_normalize_coords(gint *x,gint *y,gint width,gint heigh){
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

static gint cb_page_size(GtkAdjustment *get) {
	if (get==NULL || D4X_TOOL_CURRENT!=D4X_TOOL_ONE){
		return(FALSE);
	};
	if (main_log_value==get->value && get->value<get->upper-get->page_size) {
		get->value=get->upper-get->page_size;
		main_log_value=get->value;
		gtk_adjustment_value_changed(get);
	} else{
		main_log_value=get->value;
	};
	return(FALSE);
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
			char *desc_utf=desc?g_convert(desc,-1,"UTF-8",LOCALE_CODEPAGE,NULL,NULL,NULL):NULL;
			d4xDownloadQueue *tmpq=D4X_QUEUE;
			if (is_dnd_basket && dnd_trash_target_queue)
				D4X_QUEUE=dnd_trash_target_queue;
			if (CFG.NEED_DIALOG_FOR_DND){
				init_add_dnd_window(str,desc_utf);
			}else{
				_aa_.add_downloading(str, (char*)NULL,(char*)NULL,desc_utf);
			};
			if (desc_utf) g_free(desc_utf);
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
//FIXME: GTK2
		float ratio=(float)CFG.WINDOW_CLIST_HEIGHT/(float)(MAIN_PANED_HEIGHT);//-GTK_PANED(MAIN_PANED)->gutter_size);
		CFG.WINDOW_CLIST_HEIGHT=int(ratio*(float)(allocation->height));//-GTK_PANED(MAIN_PANED)->gutter_size));
		lod_set_height();
	};
	MAIN_PANED_HEIGHT=allocation->height;
	lod_get_height();
};

void init_main_log(){
	GtkListStore *list_store = gtk_list_store_new(ML_COL_LAST+1,
						      G_TYPE_INT,
						      G_TYPE_STRING,
						      G_TYPE_STRING,
						      G_TYPE_STRING,
						      G_TYPE_STRING);
	MainLogList = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(MainLogList),FALSE);
	for (int i=0;i<ML_COL_LAST;i++){
		GtkCellRenderer *renderer;
		GtkTreeViewColumn *col;
		renderer = gtk_cell_renderer_text_new ();
		col=gtk_tree_view_column_new_with_attributes ("Tittle",
							      renderer,
							      "text",i,
							      "foreground",ML_COL_LAST,
							      NULL);
		gtk_tree_view_column_set_resizable(col,FALSE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(MainLogList),col);
	};
};

gint d4x_main_switch_2downloads(){
	if (D4X_TOOL_CURRENT!=D4X_TOOL_ONE){
		gtk_container_remove(GTK_CONTAINER(D4X_TOOL_CONTAINER),D4X_TOOL_CURRENT);
		gtk_container_add(GTK_CONTAINER (D4X_TOOL_CONTAINER),D4X_TOOL_ONE);
		gtk_widget_show_all(D4X_TOOL_ONE);
		D4X_TOOL_CURRENT=D4X_TOOL_ONE;
	};
	return(TRUE);
};

gint d4x_main_switch_2mainlog(){
	if (D4X_TOOL_CURRENT!=D4X_TOOL_TWO){
		gtk_container_remove(GTK_CONTAINER(D4X_TOOL_CONTAINER),D4X_TOOL_CURRENT);
		gtk_container_add(GTK_CONTAINER (D4X_TOOL_CONTAINER),D4X_TOOL_TWO);
//		gtk_box_pack_start(GTK_BOX (D4X_TOOL_CONTAINER),D4X_TOOL_TWO, TRUE, TRUE, 0);
		D4X_TOOL_CURRENT=D4X_TOOL_TWO;
		gtk_widget_show_all(D4X_TOOL_CURRENT);
	};
	return(TRUE);
};

gint d4x_main_switch_2ftpsearch(){
	if (D4X_TOOL_CURRENT!=D4X_TOOL_THREE){
		if (D4X_SEARCH_ENGINES.count()==0)
			gtk_widget_set_sensitive(D4X_TOOL_BUTTON_FIND,FALSE);
		gtk_container_remove(GTK_CONTAINER(D4X_TOOL_CONTAINER),D4X_TOOL_CURRENT);
		gtk_container_add(GTK_CONTAINER (D4X_TOOL_CONTAINER),D4X_TOOL_THREE);
		D4X_TOOL_CURRENT=D4X_TOOL_THREE;
		gtk_widget_show_all(D4X_TOOL_CURRENT);
	};
	return(TRUE);
};

static gint d4x_main_vtoolbar_changed(GtkAdjustment *adj){
	if (adj->lower<adj->value)
		gtk_widget_set_sensitive(D4X_TOOL_UP,TRUE);
	else
		gtk_widget_set_sensitive(D4X_TOOL_UP,FALSE);
	if (adj->upper>adj->page_size+adj->value)
		gtk_widget_set_sensitive(D4X_TOOL_DOWN,TRUE);
	else
		gtk_widget_set_sensitive(D4X_TOOL_DOWN,FALSE);
};

static gint d4x_main_vtoolbar_up(GtkWidget *button,GtkAdjustment *adj){
	adj->value-=20;
	if (adj->value<adj->lower)
		adj->value=adj->lower;
	gtk_adjustment_value_changed(adj);
	gtk_adjustment_changed(adj);
};

static gint d4x_main_vtoolbar_down(GtkWidget *button,GtkAdjustment *adj){
	adj->value+=20;
	if (adj->value>adj->upper-adj->page_size)
		adj->value=adj->upper-adj->page_size;
	gtk_adjustment_value_changed(adj);
	gtk_adjustment_changed(adj);
};

static gint d4x_main_fsearch_activate(GtkWidget *entry){
	char *name=(char *)gtk_entry_get_text(GTK_ENTRY(entry));
	if (name && *name && !isspace(*name)){
		_aa_.ftp_search_name(name);
		gtk_entry_set_text(GTK_ENTRY(entry),"");
	};
	return (TRUE);
};

static gint d4x_main_fsearch_click(GtkWidget *button,GtkWidget *entry){
	return (d4x_main_fsearch_activate(entry));
};


GtkWidget *init_vertical_toolbar(){
#include "pixmaps/dndtrash.xpm"
#include "pixmaps/clocks.xpm"
#include "pixmaps/queues.xpm"
#include "pixmaps/filters.xpm"
#include "pixmaps/ftpsearch.xpm"
#include "pixmaps/mainlog.xpm"
#include "pixmaps/urlmng.xpm"
#include "pixmaps/down.xpm"
#include "pixmaps/up.xpm"
	GtkWidget *vbox1=gtk_vbox_new(FALSE,1);
	GtkWidget *vbox=gtk_vbox_new(FALSE,1);
	d4x::Theme::Image *img[6];
	img[0]=new d4x::Theme::Image(queues_xpm,"toolbar queues>file");
	img[1]=new d4x::Theme::Image(mainlog_xpm,"toolbar mainlog>file");
	img[2]=new d4x::Theme::Image(urlmng_xpm,"toolbar urlmng>file");
	img[3]=new d4x::Theme::Image(ftpsearch_xpm,"toolbar ftpsearch>file");
	img[4]=new d4x::Theme::Image(filters_xpm,"toolbar filters>file");
	img[5]=new d4x::Theme::Image(clocks_xpm,"toolbar scheduler>file");
	GtkWidget *buttons[6];
	GtkStyle  *tmpstyle = gtk_widget_get_style(MainWindow);
	GdkColor tmpcolor=tmpstyle->bg[GTK_STATE_NORMAL];
	tmpcolor.red=(tmpcolor.red*2)/3;
	tmpcolor.green=(tmpcolor.green*2)/3;
	tmpcolor.blue=(tmpcolor.blue*2)/3;
	for (int i=0;i<6;i++){
		d4x::CUR_THEME->monitor(img[i]);
		buttons[i]=gtk_button_new();
		gtk_button_set_relief(GTK_BUTTON(buttons[i]),GTK_RELIEF_NONE);
		gtk_container_add(GTK_CONTAINER(buttons[i]),GTK_WIDGET(img[i]->img));
		gtk_box_pack_start (GTK_BOX (vbox), buttons[i], FALSE, FALSE, 0);
		gtk_widget_modify_bg (buttons[i],GTK_STATE_PRELIGHT,&tmpcolor);
		gtk_widget_modify_bg (buttons[i],GTK_STATE_ACTIVE,&tmpcolor);
	};
 	g_signal_connect(G_OBJECT(buttons[0]),"clicked",
			   G_CALLBACK(d4x_main_switch_2downloads),NULL);
 	g_signal_connect(G_OBJECT(buttons[1]),"clicked",
			   G_CALLBACK(d4x_main_switch_2mainlog),NULL);
	g_signal_connect(G_OBJECT(buttons[2]),"clicked",
			 G_CALLBACK(open_passwords_window),NULL);
	g_signal_connect(G_OBJECT(buttons[3]),"clicked",
			 G_CALLBACK(d4x_main_switch_2ftpsearch),NULL);
	g_signal_connect(G_OBJECT(buttons[4]),"clicked",
			 G_CALLBACK(d4x_filters_tool_switch),NULL);
	g_signal_connect(G_OBJECT(buttons[5]),"clicked",
			 G_CALLBACK(d4x_scheduler_tool_switch),NULL);
	GtkTooltips *tooltip=gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltip,buttons[0],_("Downloads"),(const gchar *)NULL);
	gtk_tooltips_set_tip(tooltip,buttons[1],_("Main log"),(const gchar *)NULL);
	gtk_tooltips_set_tip(tooltip,buttons[2],_("URL-manager"),(const gchar *)NULL);
	gtk_tooltips_set_tip(tooltip,buttons[3],_("FTP-search"),(const gchar *)NULL);
	gtk_tooltips_set_tip(tooltip,buttons[4],_("Filters"),(const gchar *)NULL);
	gtk_tooltips_set_tip(tooltip,buttons[5],_("Scheduler"),(const gchar *)NULL);
	gtk_tooltips_enable(tooltip);
	
	/* need this stupid widget to change background of my toolbar */
	
	GtkWidget *stupid_gtk = gtk_event_box_new();
	gtk_widget_modify_bg (stupid_gtk,GTK_STATE_NORMAL,&tmpcolor);
	gtk_container_add(GTK_CONTAINER(stupid_gtk),vbox);
	
	GtkWidget *sw=D4X_VTOOLBAR_CONTAINER=gtk_scrolled_window_new((GtkAdjustment *)NULL,(GtkAdjustment *)NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),GTK_SHADOW_NONE);
	GtkWidget *viewport=gtk_viewport_new((GtkAdjustment *)NULL,
					     (GtkAdjustment *)NULL);
	gtk_viewport_set_shadow_type(GTK_VIEWPORT(viewport),GTK_SHADOW_NONE);
	gtk_container_add(GTK_CONTAINER(viewport),stupid_gtk);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
	                                GTK_POLICY_NEVER,GTK_POLICY_NEVER);
	gtk_container_add(GTK_CONTAINER(sw),viewport);
	GtkWidget *button_up=D4X_TOOL_UP=gtk_button_new();
	GtkWidget *pixmap_up=gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data((const char**)up_xpm));
	GtkWidget *button_down=D4X_TOOL_DOWN=gtk_button_new();
	GtkWidget *pixmap_down=gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_xpm_data((const char**)down_xpm));
	gtk_container_add(GTK_CONTAINER(button_up),pixmap_up);
	gtk_container_add(GTK_CONTAINER(button_down),pixmap_down);
	gtk_widget_set_sensitive(button_down,FALSE);

	/* packing buttons to vertical toolbar */
	gtk_widget_modify_bg (button_up,GTK_STATE_PRELIGHT,&tmpcolor);
	gtk_box_pack_start (GTK_BOX (vbox1), button_up, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox1), sw, TRUE, TRUE, 0);
	gtk_widget_modify_bg (button_down,GTK_STATE_PRELIGHT,&tmpcolor);
	gtk_box_pack_start (GTK_BOX (vbox1), button_down, FALSE, FALSE, 0);
	/* some handlers */
	GtkAdjustment *adj=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(sw));
	g_signal_connect(G_OBJECT (adj), "changed",
			 G_CALLBACK (d4x_main_vtoolbar_changed), NULL);
	g_signal_connect(G_OBJECT(button_up),"clicked",
			 G_CALLBACK(d4x_main_vtoolbar_up),adj);
	g_signal_connect(G_OBJECT(button_down),"clicked",
			 G_CALLBACK(d4x_main_vtoolbar_down),adj);
	gtk_button_set_relief(GTK_BUTTON(button_up),GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(button_down),GTK_RELIEF_NONE);
		
	GtkWidget *vb=gtk_vbox_new(FALSE,1);
	GtkWidget *label=gtk_label_new(_("Main log"));
	
	gtk_box_pack_start (GTK_BOX (vb),my_gtk_set_header_style(label), FALSE, FALSE, 0);
	GtkWidget *scroll_window=gtk_scrolled_window_new((GtkAdjustment *)NULL,main_log_adj);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scroll_window),
					    GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(scroll_window),GTK_WIDGET(MainLogList));
	gtk_box_pack_start (GTK_BOX (vb),scroll_window, TRUE, TRUE, 0);
	D4X_TOOL_TWO=vb;


	vb=gtk_vbox_new(FALSE,1);
	label=gtk_label_new(_("FTP-search"));

	gtk_box_pack_start (GTK_BOX (vb),my_gtk_set_header_style(label), FALSE, FALSE, 0);
	GtkWidget *hbox=gtk_hbox_new(FALSE,5);
	GtkWidget *label1=gtk_label_new(_("Filename:"));
	gtk_misc_set_alignment(GTK_MISC(label1),1,0);
	GtkWidget *entry=gtk_entry_new();
	GtkWidget *button_find=D4X_TOOL_BUTTON_FIND=gtk_button_new_from_stock(GTK_STOCK_FIND);
	g_signal_connect(G_OBJECT(entry), "activate",
			 G_CALLBACK(d4x_main_fsearch_activate), entry);
	g_signal_connect(G_OBJECT(button_find),"clicked",
			 G_CALLBACK(d4x_main_fsearch_click),entry);
	gtk_box_pack_start (GTK_BOX (hbox), label1, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), button_find, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vb), hbox, FALSE, FALSE, 0);
	scroll_window=gtk_scrolled_window_new((GtkAdjustment *)NULL,(GtkAdjustment *)NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scroll_window),
					    GTK_SHADOW_IN);
	FSearchView=fs_list_init();
	FSearchView2=fs_list_init_sublist();
	GtkWidget *hpaned=gtk_vpaned_new();
	gtk_container_add(GTK_CONTAINER(scroll_window),GTK_WIDGET(FSearchView));
	gtk_paned_add1(GTK_PANED(hpaned),GTK_WIDGET(scroll_window));
	scroll_window=gtk_scrolled_window_new((GtkAdjustment *)NULL,(GtkAdjustment *)NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scroll_window),
					    GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(scroll_window),GTK_WIDGET(FSearchView2));
	gtk_paned_add2(GTK_PANED(hpaned),GTK_WIDGET(scroll_window));
	gtk_box_pack_start (GTK_BOX (vb),hpaned, TRUE, TRUE, 0);
	D4X_TOOL_THREE=vb;

	
	stupid_gtk = gtk_event_box_new();
	gtk_widget_modify_bg (stupid_gtk,GTK_STATE_NORMAL,&tmpcolor);
	gtk_container_add(GTK_CONTAINER(stupid_gtk),vbox1);
	return(stupid_gtk);
};

void d4x_main_buttons_of_log_update(tDownload *dwn){
	if (D4X_LOG_DISPLAY.papa==dwn){
		int i=1;
//		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4X_LOG_DISPLAY.buttons[0]),TRUE);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(D4X_LOG_DISPLAY.buttons[0]),TRUE);
		if (dwn->split){
			for(i=1;i<dwn->split->NumOfParts;i++){
				if (D4X_LOG_DISPLAY.buttons[i]->parent==NULL)
					gtk_toolbar_insert(GTK_TOOLBAR(D4X_LOG_DISPLAY.buttonsbar),GTK_TOOL_ITEM(D4X_LOG_DISPLAY.buttons[i]),-1);
				gtk_widget_show(D4X_LOG_DISPLAY.buttons[i]);
			};
		};
		for(;i<10;i++)
			if (D4X_LOG_DISPLAY.buttons[i]->parent!=NULL)
				gtk_container_remove(GTK_CONTAINER(D4X_LOG_DISPLAY.buttonsbar),
						     D4X_LOG_DISPLAY.buttons[i]);
	};
};

void d4x_main_switch_log(tDownload *dwn){
	if (dwn!=NULL){
		if (dwn->LOG==NULL){
			dwn->LOG=new tLog;
			dwn->LOG->init(CFG.MAX_LOG_LENGTH);
			dwn->LOG->ref_inc();
		};
	
		if (dwn->LOG!=D4X_LOG_DISPLAY.log && D4X_LOG_DISPLAY.papa!=dwn){
			gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(D4X_LOG_DISPLAY.view)));
			log_print_to_view(dwn->LOG,D4X_LOG_DISPLAY.view);
			D4X_LOG_DISPLAY.log=dwn->LOG;
			D4X_LOG_DISPLAY.papa=dwn;
			d4x_main_buttons_of_log_update(dwn);
//			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4X_LOG_DISPLAY.buttons[D4X_LOG_DISPLAY.curbutton]),FALSE);
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(D4X_LOG_DISPLAY.buttons[D4X_LOG_DISPLAY.curbutton]),FALSE);
			D4X_LOG_DISPLAY.curbutton=0;
		};
	};
};

void d4x_main_log_del_string(){
	GtkTreeIter iter;
	GtkListStore *store=(GtkListStore *)gtk_tree_view_get_model(D4X_LOG_DISPLAY.view);
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter))
		gtk_list_store_remove(store,&iter);
};

void d4x_main_log_add_string(tLogString *str){
	log_model_view_add_string(D4X_LOG_DISPLAY.view,str);
};

gint d4x_main_dwn_log_callback(GtkWidget *button,int a){
	if (a==D4X_LOG_DISPLAY.curbutton) return FALSE;
	tDownload *dwn=D4X_LOG_DISPLAY.papa;
	if (dwn==NULL || (dwn->split==NULL && a>0)){
//		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4X_LOG_DISPLAY.buttons[D4X_LOG_DISPLAY.curbutton]),TRUE);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(D4X_LOG_DISPLAY.buttons[D4X_LOG_DISPLAY.curbutton]),TRUE);
		return FALSE;
	};
	int switch_to=0;
	int counter=a;
	while(dwn!=NULL && counter>0){
		dwn=dwn->split->next_part;
		switch_to++;
		counter--;
	};
	if (switch_to==a && dwn && dwn->LOG){
		gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(D4X_LOG_DISPLAY.view)));
		log_print_to_view(dwn->LOG,D4X_LOG_DISPLAY.view);
		D4X_LOG_DISPLAY.log=dwn->LOG;
		D4X_LOG_DISPLAY.curbutton=switch_to;
	}else{
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(D4X_LOG_DISPLAY.buttons[D4X_LOG_DISPLAY.curbutton]),TRUE);
//		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4X_LOG_DISPLAY.buttons[D4X_LOG_DISPLAY.curbutton]),TRUE);
	};
	return FALSE;
};

GtkWidget *d4x_main_dwn_log_init(){
	main_log_adj = (GtkAdjustment *)gtk_adjustment_new (0.0, 0.0, 0.0, 0.1, 1.0, 1.0);
	main_log_value=0.0;
	g_signal_connect(G_OBJECT (main_log_adj), "changed",
			 G_CALLBACK (cb_page_size), NULL);
	GtkWidget *scroll_window=gtk_scrolled_window_new((GtkAdjustment *)NULL,main_log_adj);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scroll_window),
					    GTK_SHADOW_IN);

	D4X_LOG_DISPLAY.view=log_model_view_init();
	gtk_container_add(GTK_CONTAINER(scroll_window),GTK_WIDGET(D4X_LOG_DISPLAY.view));

	GtkWidget *buttonsbar=D4X_LOG_DISPLAY.buttonsbar=gtk_toolbar_new();
	gtk_toolbar_set_orientation(GTK_TOOLBAR(buttonsbar),GTK_ORIENTATION_VERTICAL);
	gtk_toolbar_set_style(GTK_TOOLBAR(buttonsbar),GTK_TOOLBAR_TEXT);
	GtkWidget *tmpbutton=NULL;
	GSList *group=NULL;
	for (int i=0;i<10;i++){
		char data[MAX_LEN];
		g_snprintf(data,MAX_LEN,"%i",i);
//		tmpbutton=D4X_LOG_DISPLAY.buttons[i]=my_gtk_vbookmark_new_with_label(group,data);
		tmpbutton=D4X_LOG_DISPLAY.buttons[i]=GTK_WIDGET(gtk_radio_tool_button_new(group));
		gtk_tool_button_set_label(GTK_TOOL_BUTTON(tmpbutton),data);
		g_signal_connect(G_OBJECT(tmpbutton),"clicked",
				 G_CALLBACK(d4x_main_dwn_log_callback),GINT_TO_POINTER(i));
//		group=gtk_radio_button_get_group(GTK_RADIO_BUTTON(tmpbutton));
		group=gtk_radio_tool_button_get_group(GTK_RADIO_TOOL_BUTTON(tmpbutton));

		gtk_widget_ref(tmpbutton);
		g_object_set_data(G_OBJECT(tmpbutton),"d4x_user_data",GINT_TO_POINTER(i));
	};
//	gtk_toolbar_append_widget(GTK_TOOLBAR (buttonsbar),D4X_LOG_DISPLAY.buttons[0],NULL,NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(buttonsbar),GTK_TOOL_ITEM(D4X_LOG_DISPLAY.buttons[0]),-1);
	D4X_LOG_DISPLAY.curbutton=0;
	GtkWidget *hbox=gtk_hbox_new(FALSE,1);
	gtk_box_pack_start (GTK_BOX (hbox), buttonsbar, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), scroll_window, TRUE, TRUE, 0);
	return(hbox);
}

void d4x_main_offline_mode(){
	D4X_OFFLINE_IMAGE->reinit(CFG.OFFLINE_MODE?d4x::OMB_OFFLINE:d4x::OMB_ONLINE);
	GtkTooltipsData  *tooltipdata=gtk_tooltips_data_get(D4X_OFFLINE_BUTTON);
	if (tooltipdata){
		gtk_tooltips_set_tip(tooltipdata->tooltips,D4X_OFFLINE_BUTTON,
				     CFG.OFFLINE_MODE?_("Switch to online mode"):_("Switch to offline mode"),
				     (const gchar *)NULL);
	};
	dnd_trash_menu_prepare();
	dnd_trash_redraw();
};

void d4x_main_offline_click(GtkWidget *button){
	_aa_.switch_offline_mode();
};

void init_main_window() {
	ContainerForCList=gtk_scrolled_window_new((GtkAdjustment *)NULL,(GtkAdjustment *)NULL);
	GtkWidget *hbox=gtk_hbox_new(FALSE,1);
	MainHBox=hbox;
	gtk_box_pack_start (GTK_BOX (hbox), MainStatusBar, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), ProgressOfDownload, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), ReadedBytesStatusBar, FALSE, FALSE, 0);
	gtk_widget_set_size_request(hbox,-1,21);

	init_main_log();

	GtkWidget *hpaned=gtk_vpaned_new();
	MAIN_PANED=hpaned;
	MAIN_PANED1=gtk_hpaned_new();

	GtkWidget *scroll_window2=gtk_scrolled_window_new((GtkAdjustment *)NULL,(GtkAdjustment *)NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window2),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	D4X_QVT=new d4xQsTree;
	D4X_QVT->init();
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scroll_window2),
					    GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(scroll_window2),GTK_WIDGET(D4X_QVT->view));
	gtk_paned_add1(GTK_PANED(MAIN_PANED1),scroll_window2);
	gtk_paned_add2(GTK_PANED(MAIN_PANED1),ContainerForCList);
	gtk_paned_add1(GTK_PANED(hpaned),MAIN_PANED1);
	gtk_paned_add2(GTK_PANED(hpaned),d4x_main_dwn_log_init());
	GtkWidget *hbox_main=gtk_hbox_new(FALSE,1);
	GtkWidget *container_main=D4X_TOOL_CONTAINER=gtk_hbox_new(FALSE,1);

	GtkWidget *vb=gtk_vbox_new(FALSE,1);
	GtkWidget *label=gtk_label_new(_("Downloads"));

	gtk_box_pack_start (GTK_BOX (vb), my_gtk_set_header_style(label), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vb), hpaned, TRUE, TRUE, 0);
	D4X_TOOL_CURRENT=D4X_TOOL_ONE=vb;

	gtk_box_pack_start (GTK_BOX (container_main), vb, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox_main), container_main, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox_main), init_vertical_toolbar(), FALSE, FALSE, 0);
	gtk_widget_ref(D4X_TOOL_ONE);
	gtk_widget_ref(D4X_TOOL_TWO);
	gtk_widget_ref(D4X_TOOL_THREE);
	
	GtkWidget *scroll_window1=gtk_scrolled_window_new((GtkAdjustment *)NULL,(GtkAdjustment *)NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window1),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scroll_window1),
					    GTK_SHADOW_IN);

	GtkWidget *offline=D4X_OFFLINE_BUTTON=gtk_button_new();

	GtkTooltips *tooltip=gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltip,offline,_("Switch to offline mode"),(const gchar *)NULL);
	gtk_tooltips_enable(tooltip);
				
	GtkRcStyle *rc_style = gtk_rc_style_new ();
	rc_style->xthickness = 0;
	rc_style->ythickness = 0;
	gtk_widget_modify_style (offline, rc_style);
	g_object_unref(G_OBJECT(rc_style));
	GTK_WIDGET_UNSET_FLAGS(offline,GTK_CAN_FOCUS);
	
	D4X_OFFLINE_IMAGE=new d4x::Theme::SlaveImage(d4x::OMB_ONLINE);
	d4x::CUR_THEME->monitor(D4X_OFFLINE_IMAGE);
	gtk_container_add(GTK_CONTAINER(offline),GTK_WIDGET(D4X_OFFLINE_IMAGE->img));

 	g_signal_connect(G_OBJECT(offline),"clicked",
			 G_CALLBACK(d4x_main_offline_click),NULL);
	gtk_button_set_relief(GTK_BUTTON(offline),GTK_RELIEF_NONE);
	gtk_container_set_border_width (GTK_CONTAINER(offline),0);
	gtk_box_pack_end (GTK_BOX (hbox), offline, FALSE, FALSE, 0);
	
	GtkWidget *TEMP=my_gtk_graph_new();
	GLOBAL_GRAPH=(MyGtkGraph *)TEMP;
	GLOBAL_GRAPH->LocalM=LocalMeter;
	GLOBAL_GRAPH->GlobalM=GlobalMeter;
	gtk_widget_set_size_request(TEMP,104,-1);
	gtk_box_pack_end (GTK_BOX (hbox), TEMP, FALSE, FALSE, 0);

	GtkWidget *vbox=gtk_vbox_new(FALSE,1);
	gtk_box_pack_start (GTK_BOX (vbox), MainMenu, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), ButtonsBar, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox_main, TRUE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(MainWindow),vbox);

	gtk_widget_show(MainStatusBar);
	gtk_widget_show(ProgressOfDownload);
	gtk_widget_show(TEMP);
	gtk_widget_show(ReadedBytesStatusBar);
	gtk_progress_bar_set_fraction((GtkProgressBar *)ProgressOfDownload,1);

	gtk_widget_show(ContainerForCList);
	gtk_widget_show(MainLogList);
	gtk_widget_show(scroll_window1);
	gtk_widget_show_all(scroll_window2);
	gtk_widget_show(MAIN_PANED1);
	gtk_widget_show(hpaned);
	gtk_widget_show(MainMenu);
	gtk_widget_show(hbox);
	gtk_widget_show(vbox);
	gtk_widget_show_all(MainWindow);
	if (!CFG.DONOTSET_WINPOS)
		gdk_window_move_resize(MainWindow->window,
				       gint(CFG.WINDOW_X_POSITION),
				       gint(CFG.WINDOW_Y_POSITION),
				       gint(CFG.WINDOW_WIDTH),
				       gint(CFG.WINDOW_HEIGHT));
	g_signal_connect(G_OBJECT (MAIN_PANED), "size_allocate",
			   G_CALLBACK (list_of_downloads_allocation), NULL);
//	g_signal_connect(G_OBJECT (MAIN_PANED2), "size_allocate",
//			   G_CALLBACK (fs_list_allocation), NULL);


	/****************************************************************
	  Initing signals' handlers for DnD support (added by Justin Bradford)
	 ****************************************************************/
	// connect the drag-drop signal
	g_signal_connect(G_OBJECT(ContainerForCList),
	                   "drag_data_received",
	                   G_CALLBACK(list_dnd_drop_internal),
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
static void tmp_scroll_title(std::string &title,int ind){
	if (CFG.SCROLL_MAINWIN_TITLE){
		ScrollShift[ind]+=1;
		if (ScrollShift[ind]>=title.length())
			ScrollShift[ind]=0;
		if (ScrollShift[ind]){
			title=title.substr(ScrollShift[ind])+title.substr(0,ScrollShift[ind]);
		};
	};	
};

void update_mainwin_title() {
	if (CFG.USE_MAINWIN_TITLE) {
		mainwin_title_state=1;
		tDownload *temp=D4X_QUEUE->first(DL_RUN);
		std::string title;
		if (temp){
			int i=0;
			std::string tipstr;
			tDownload *frst=temp;
			while(temp && i++<3){
				std::string size=temp->finfo.size>=0?
					make_number_nice(temp->finfo.size,D4X_QUEUE->NICE_DEC_DIGITALS):
					"???";
				char b[100];
				d4x_percent_str(temp->Percent,b,sizeof(b));
				tipstr+=std::string(b)+"% "+make_number_nice(fsize_t(temp->Size),D4X_QUEUE->NICE_DEC_DIGITALS)
					+"/"+size+hexed_string(temp->info.file);
				if (i==2){
					tipstr+=std::string("\n");
				}else if(!i){
					title=tipstr;
				};
				temp=(tDownload*)(temp->prev);
			};
			d4x::TRAY->set_tooltip(tipstr.c_str());
			dnd_trash_set_tooltip(tipstr.c_str(),frst->Percent);
		}else{
			dnd_trash_set_tooltip(NULL);
			d4x::TRAY->set_tooltip(0);
		};
		if (CFG.USE_MAINWIN_TITLE2 && (UpdateTitleCycle % 3)!=0) {
			title=std::string("[")+D4X_QUEUE->name.get()+"] "+
				boost::lexical_cast<std::string>(D4X_QUEUE->count(DL_RUN))+"-running "+
				boost::lexical_cast<std::string>(D4X_QUEUE->count(DL_COMPLETE))+"-completed "+
				boost::lexical_cast<std::string>(D4X_QUEUE->count())+"-total";
		} else if (title.empty()){
			title=VERSION_NAME;
		};
		tmp_scroll_title(title,ROLL_INFO);
		gtk_window_set_title(GTK_WINDOW (MainWindow), title.c_str());
	} else{
		if (mainwin_title_state){
			gtk_window_set_title(GTK_WINDOW (MainWindow), VERSION_NAME);
			dnd_trash_set_tooltip(NULL);
			d4x::TRAY->set_tooltip(0);
		};
		mainwin_title_state=0;
	};
};


int time_for_refresh(void *a) {
	_aa_.main_circle();
	update_progress_bar();
	update_mainwin_title();
	UpdateTitleCycle+=1;
	my_gtk_graph_recalc(GLOBAL_GRAPH);
	if (D4X_DND_GRAPH)
		my_gtk_graph_recalc(D4X_DND_GRAPH);
	return 1;
};

static int _nano_step_=0;
static int _nano_stepn_=0;

int time_for_logs_refresh(void *a) {
	if (_nano_step_)
		_aa_.main_circle_nano1();
	else
		_aa_.main_circle_nano2();
	_nano_step_=~_nano_step_;
	_aa_.redraw_logs();
	_aa_.check_for_remote_commands();
	if (_nano_stepn_++>=GLOBAL_SLEEP_DELAY*5){
		time_for_refresh(a);
		_nano_stepn_=0;
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

bool check_for_clipboard_skiping(const char *buf){
	d4x::URL url(buf);
	std::string temp;
	if (CFG.CLIPBOARD_SKIP_OR_CATCH){
		std::istringstream extensions(CFG.CATCH_IN_CLIPBOARD);
		while(extensions>>temp){
			std::string ext=filename_extension(url.file);
			if (strcasecmp(ext.c_str(),temp.c_str())==0)
				return true;
		};
		return false;
	};
	std::istringstream extensions(CFG.SKIP_IN_CLIPBOARD);
	while(extensions>>temp){
		std::string ext=filename_extension(url.file);
		if (strcasecmp(ext.c_str(),temp.c_str())==0)
			return false;
	};
	return true;
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
		gtk_selection_convert(MainWindow, GDK_SELECTION_CLIPBOARD,
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
	ListTimer = g_timeout_add (60000, time_for_save_list , NULL);
	GraphTimer = g_timeout_add (250, time_for_draw_graph , NULL);
	LogsTimer = g_timeout_add (400, time_for_logs_refresh , NULL);
	FirstConfigureEvent=1;
	g_signal_connect(G_OBJECT(MainWindow), "configure_event",
	                   G_CALLBACK(get_mainwin_sizes),
	                   MainWindow);
};

void main_window_iconify(){
	if (MainWindow)
		gtk_window_iconify(GTK_WINDOW(MainWindow));
};

void main_window_popup(){
	if (MainWindow)
		gtk_window_present(GTK_WINDOW(MainWindow));
};

void main_window_toggle(){
	if (MainWindow){
		if (gdk_window_is_visible(MainWindow->window)){
			gdk_window_hide(MainWindow->window);
		}else{
			gtk_window_present(GTK_WINDOW(MainWindow));
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
	if (gdk_selection_owner_get(GDK_SELECTION_CLIPBOARD)==MainWindow->window){
		return(D4X_CLIPBOARD);
	};
	return(NULL);
};

void d4x_mw_clipboard_set(const char *str){
	if (D4X_CLIPBOARD) delete[] D4X_CLIPBOARD;
	D4X_CLIPBOARD=copy_string(str);
	gtk_selection_owner_set (MainWindow,
				 GDK_SELECTION_CLIPBOARD,
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
};


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
	if (!CFG.DONOTSET_WINPOS){
		gtk_window_move(GTK_WINDOW(MainWindow),
				gint(CFG.WINDOW_X_POSITION),
				gint(CFG.WINDOW_Y_POSITION));
	};
	gtk_window_set_default_size(GTK_WINDOW(MainWindow),gint(CFG.WINDOW_WIDTH),gint(CFG.WINDOW_HEIGHT));
	gtk_window_set_title(GTK_WINDOW (MainWindow), VERSION_NAME);
	gtk_widget_set_size_request( GTK_WIDGET (MainWindow), 400, 200);
	gtk_widget_realize(MainWindow);
	MainWindowGC=gdk_gc_new(MainWindow->window);

	d4x::CUR_THEME=new d4x::Theme;
	
        dnd_trash_init_menu();
	init_main_menu();
	init_list_menu();
	init_status_bar();
	init_buttons_bar();
	init_main_window();
/* initing table of shifts
 */
	for (int i=0;i<ROLL_LAST;i++)
		ScrollShift[i]=0;
#include "pixmaps/main.xpm"
	GdkBitmap *bitmap;
	GdkPixmap *pixmap=make_pixmap_from_xpm(&bitmap,main_xpm);
	gdk_window_set_icon(MainWindow->window,(GdkWindow *)NULL,pixmap,bitmap);
	g_signal_connect(G_OBJECT(MainWindow), "delete_event",
	                   G_CALLBACK(ask_exit2),
	                   NULL);
	g_signal_connect(G_OBJECT(MainWindow), "selection_get",
			   G_CALLBACK(d4x_mw_selection_get),NULL);
	g_signal_connect(G_OBJECT(MainWindow), "selection_received",
			   G_CALLBACK(my_get_xselection),NULL);
/*
	g_signal_connect(G_OBJECT(MainWindow), "selection_clear_event",
			   G_CALLBACK(d4x_mw_selection_clear),NULL);
*/
	g_signal_connect(G_OBJECT (MainWindow),
			   "key_press_event",
			   G_CALLBACK (main_menu_prepare),
			   NULL);
	lod_set_height();
	if (CFG.DND_TRASH){
		dnd_trash_init();
		if (CFG.HIDE_MAIN_WINDOW) main_window_toggle();
	};
	main_log_mask=0;
	buttons_cfg_init();
	d4x::TRAY=new d4x::Tray;
};
