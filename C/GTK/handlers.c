#include "header.h"


/*Closes App*/
void close_app(UNUSED, AppWidgets *app){
	
	g_free(app);
	//Code Before closing comes here... (cleanup)
	gtk_main_quit();
	
}

/*Show FolderPicker Window + close Start Window*/
void gtk_widget_showPicker(UNUSED, AppWidgets *app){
	gtk_widget_destroy(app->Start);
	gtk_widget_show(app->folderPicker);
}

/*Show FolderPicker Window + close Start Window*/
void gtk_widget_showPicker_again(UNUSED, AppWidgets *app){
	gtk_widget_hide(app->selectNick);
	gtk_widget_show(app->folderPicker);
}

/*Action when NEXT from FilePicker is clicked*/
void folder_set(UNUSED, AppWidgets *app){
	
	/*GET FOLDER:*/
    gchar *folder =  gtk_file_chooser_get_uri ((GtkFileChooser*)app->folderButton);
	if(folder != NULL){
		printf("-------\nCHOSEN FOLDER: 	%s\n--------", folder);
		
		/**
		 * WHAT TO DO WITH FOLDER?
		 * 
		 * */
		
		//avança...
		gtk_widget_show(app->selectNick);
		gtk_widget_hide(app->folderPicker);
	}//else do nothing
    g_free (folder);
	
}

void finish(UNUSED, AppWidgets *app){
	
	const gchar* nickname = gtk_entry_get_text((GtkEntry *) app->nickBox);
	printf("\n-------\nUSERNAME: %s\n-------\n", nickname);
	
	
	
	
	g_free((gpointer* )nickname);
	gtk_main_quit();
	printf("\n---------\nApp closed\n---------\n");
}

/* Show the About box */
void help_about(UNUSED, AppWidgets *app){
	gtk_widget_hide(app->Start);
	gtk_widget_show(app->HelpAboutDialog);
}

/*Close About box*/
void closeHelpAbout(UNUSED, AppWidgets* app){
	gtk_widget_hide(app->HelpAboutDialog);
	gtk_widget_show(app->Start);
}

/*Show team info from About Dialog*/
void show_team(UNUSED, AppWidgets* app){
	gtk_widget_show(app->theTeam);
}

/*Show team info from license Dialog*/
void show_team2(UNUSED, AppWidgets* app){
	gtk_widget_hide(app->licenseWindow);
	gtk_widget_show(app->theTeam);
}

/*Shows License from About Dialog*/
void show_license(UNUSED, AppWidgets* app){
	gtk_widget_show(app->licenseWindow);
}

/*Shows License from license Dialog*/
void show_license2(UNUSED, AppWidgets* app){
	gtk_widget_show(app->licenseWindow);
	gtk_widget_hide(app->theTeam);
}

void close_license(UNUSED, AppWidgets* app){
	gtk_widget_hide(app->licenseWindow);
}

/*Closes team info*/
void closeTheTeam(UNUSED, AppWidgets* app){
	gtk_widget_hide(app->theTeam);
}




