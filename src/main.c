/* Wojciech Pawlik */

#include <gtk/gtk.h>

#include "gui.h"

int main (int argc, char **argv) {
	GtkApplication *app = gtk_application_new (
		"pl.edu.uj.student.wojpawlik.mines",
		G_APPLICATION_FLAGS_NONE
	);
	int status;

	g_signal_connect (app, "activate", G_CALLBACK (gui_activate), NULL);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;
}
