#include "header.h"


/*Sets NextButton1 visible (in FolderPicker*/
void set_Next_Visible(UNUSED, AppWidgets *app){

	gtk_widget_show(app->nextButton1);
	
	char *folderPath;

    folderPath = gtk_file_chooser_get_filename ((GtkFileChooser*) app->folderButton);
    /*What to do with folder path?*/
    printf("%s", folderPath);
    
    g_free (folderPath);
    
}

/*Closes App*/
void close_app(UNUSED, AppWidgets *app){
	
	//Code Before closing comes here...
	
	gtk_main_quit();
	
}

/*Show FolderPicker Window + close Start Window*/
void gtk_widget_showPicker(UNUSED, AppWidgets *app){
	gtk_widget_hide(app->Start);
	gtk_widget_show(app->folderPicker);
	gtk_widget_hide(app->nextButton1);
}

/*Default operations for FolderPicker window*/
void do_picker_defaults(UNUSED, AppWidgets *app){
	printf("\naaa");fflush(stdout);
	printf("\nbbb");fflush(stdout);
	
	printf("\nccc");fflush(stdout);
}

/*Action when NEXT from FilePicker is clicked*/
void folder_set(UNUSED, AppWidgets *app){
	
    
	
}

/* Show the About box */
void help_about(UNUSED, AppWidgets *app){
	gtk_dialog_run(GTK_DIALOG(app->HelpAboutDialog));
}
