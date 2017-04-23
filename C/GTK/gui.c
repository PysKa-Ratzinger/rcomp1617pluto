
#include <gtk/gtk.h>

int main(int argc, char **argv){
	
	GtkBuilder *gtkbuilder;
	GtkWidget *window;
	gtk_init(&argc, &argv);
	
	gtkbuilder = gtk_builder_new();
	gtk_builder_add_from_file(gtkbuilder, "Telepathy.glade", NULL);
	window = GTK_WIDGET(gtk_builder_get_object(gtkbuilder, "Start"));
	
	g_object_unref(G_OBJECT(gtkbuilder));
	gtk_widget_show(window);
	gtk_main();
	
	return 0;
}
