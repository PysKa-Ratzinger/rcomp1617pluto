/*  Telepathy - An awesome Peer to Peer application.
 *  Copyright (C) 2017 Pluto (2DD)
 * 
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "header.h"

/* Well, by golly, this program uses widgets: */
typedef struct {
	GtkWidget *Start;
	GtkWidget *folderPicker;
	GtkWidget *nextButton1;
	GtkWidget *quitButton1;
	GtkWidget *nextButton2;
	GtkWidget *quitButton2;
	GtkWidget *folderButton;
	GtkWidget *helpButton;
	GtkWidget *HelpAboutDialog;
} AppWidgets;

#if 1
#define UNUSED gpointer _x_ G_GNUC_UNUSED
#endif


void set_Next_Visible(UNUSED, AppWidgets *app){
	gtk_widget_show (app->nextButton1);
}

void close_app(UNUSED, AppWidgets *app){
	gtk_main_quit();
	
}

void gtk_widget_showPicker(UNUSED, AppWidgets *app){
	gtk_widget_destroy(app->Start);
	gtk_widget_show(app->folderPicker);
}

void file_set(UNUSED, AppWidgets *app){
	
	/***
	 * WHAT TO DO WITH FOLDER?
	 * **/
	
}

/* Show the About box */
void help_about(UNUSED, AppWidgets *app){
	gtk_dialog_run(GTK_DIALOG(app->HelpAboutDialog));
}



/*MAIN*/
int main(int argc, char **argv) {
	GtkBuilder *builder;
	AppWidgets *app = g_slice_new(AppWidgets);
	GError *err = NULL;
	GtkWidget *window;
	gtk_init(&argc, &argv);
	builder=gtk_builder_new();
	gtk_builder_add_from_file(builder, "Telepathy.glade", &err);
	
	if (err) {
		g_error(err->message);
		g_error_free(err);
		g_slice_free(AppWidgets, app);
		return 1;
	}
	
	//Get handles to our app widgets.
	#define appGET(xx) \
	app->xx=GTK_WIDGET(gtk_builder_get_object(builder,#xx))
	appGET(Start);
	appGET(folderPicker);
	appGET(nextButton1);
	appGET(quitButton1);
	appGET(nextButton2);
	appGET(quitButton2);
	appGET(helpButton);
	appGET(folderButton);
	appGET(HelpAboutDialog);
	gtk_builder_connect_signals(builder, app);
	g_object_unref(G_OBJECT(builder));
	window = GTK_WIDGET(app->Start);
	gtk_widget_show(window);
	gtk_main();
	g_slice_free(AppWidgets, app);
	return 0;
}
