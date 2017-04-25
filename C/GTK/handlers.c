#include "header.h"


/*Sets NextButton1 visible (in FolderPicker*/
void set_Next_Visible(UNUSED, AppWidgets *app){
	gtk_widget_show_all(app->nextButton1);
}

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

/*Action when NEXT from FilePicker is clicked*/
void folder_set(UNUSED, AppWidgets *app){
	
	/***
	 * WHAT TO DO WITH FOLDER?
	 * **/
	
}

/* Show the About box */
void help_about(UNUSED, AppWidgets *app){
	gtk_dialog_run(GTK_DIALOG(app->HelpAboutDialog));
}


