#include "header.h"


/*Closes App*/
void close_app(UNUSED, AppWidgets *app){
	
	//Code Before closing comes here...
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
		printf("-------\nFOLDER: 	%s\n--------", folder);
		
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
	gtk_main_quit();
	printf("\n---------\nApp closed\n---------\n");
}

/* Show the About box */
void help_about(UNUSED, AppWidgets *app){
	gtk_dialog_run(GTK_DIALOG(app->HelpAboutDialog));
	gtk_widget_show(app->HelpAboutDialog);
}
