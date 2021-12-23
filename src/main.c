/* Wojciech Pawlik */

#include <gtk/gtk.h>

#include "gui.h"

int main (int argc, char **argv) {
	GtkApplication *app = gtk_application_new (
		"pl.edu.uj.student.wojpawlik.mines",
		G_APPLICATION_NON_UNIQUE
	);
	int status;

	g_application_set_option_context_summary (
		G_APPLICATION (app),
		"GTK3 Minesweeper game by Wojciech Pawlik"
	);
	g_application_set_option_context_description (G_APPLICATION (app), GAME_DESCRIPTION);
	g_signal_connect (app, "activate", G_CALLBACK (gui_activate), NULL);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;
}
