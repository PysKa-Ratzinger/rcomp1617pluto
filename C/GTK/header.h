
#include <gtk/gtk.h>
#include <glib/gprintf.h>  // g_sprintf
#include <stdlib.h>        // system (web link)
#include <openssl/md5.h>   // MD5 hash
#include <gdk/gdkkeysyms.h>// GDK_Escape


/* Well, by golly, this program uses widgets: */
typedef struct {
	GtkWidget *Start;
	GtkWidget *folderPicker;
	GtkWidget *nextButton1;
	GtkWidget *quitButton1;
	GtkWidget *nextButton2;
	GtkWidget *quitButton2;
	GtkWidget *quitButton3;
	GtkWidget *finishButton;
	GtkWidget *backButton;
	GtkWidget *nickBox;
	GtkWidget *selectNick;
	GtkWidget *folderButton;
	GtkWidget *helpButton;
	GtkWidget *HelpAboutDialog;
	
	GtkWidget *theTeam;
	GtkWidget *closeTheTeam;
	GtkWidget *about_license;
	GtkWidget *license_show;
	GtkWidget *about_close;
	GtkWidget *about_creditos;
	GtkWidget *about_creditos2;
	GtkWidget *licenseWindow;
	GtkWidget *close_license;
	
	
} AppWidgets;

#if 1
#define UNUSED gpointer _x_ G_GNUC_UNUSED
#endif


void set_Next_Visible(UNUSED, AppWidgets *app);
void close_app(UNUSED, AppWidgets *app);
void gtk_widget_showPicker(UNUSED, AppWidgets *app);
void file_set(UNUSED, AppWidgets *app);
void help_about(UNUSED, AppWidgets *app);
