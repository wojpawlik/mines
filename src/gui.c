/* Wojciech Pawlik */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "engine.h"
#include "gui.h"

static GtkWidget *game_board = NULL;
static GtkWidget *flags = NULL;
static Engine engine = {0};

static const char *ASSETS[] = {
	"assets/open.png",
	"assets/one.png",
	"assets/two.png",
	"assets/three.png",
	"assets/four.png",
	"assets/five.png",
	"assets/six.png",
	"assets/seven.png",
	"assets/eight.png",
	"assets/closed.png",
	"assets/flag.png",
	"assets/clicked_mine.png",
};

gchar *stringify(int n) {
	static gchar s[5];
	sprintf(s, "%d", n);
	return s;
}

static void board_open_neighbors (unsigned x, unsigned y);

static void board_open (unsigned x, unsigned y) {
	GtkWidget *event_box = gtk_grid_get_child_at(GTK_GRID (game_board), x, y);
	AssetId result = engine_open(&engine, engine_ptr(&engine, x, y));
	if (result == 0) board_open_neighbors(x, y);
	if (result != -1) {
		gtk_image_set_from_file (
			GTK_IMAGE (gtk_bin_get_child (GTK_BIN (event_box))),
			ASSETS[result]
		);
	}
}

static void board_open_neighbors (unsigned x, unsigned y) {
	board_open(x - 1, y - 1);
	board_open(x - 1, y + 1);
	board_open(x - 1, y);

	board_open(x, y - 1);
	board_open(x, y + 1);

	board_open(x + 1, y);
	board_open(x + 1, y - 1);
	board_open(x + 1, y + 1);
}

static void board_click (GtkWidget *event_box, gpointer event, gpointer square) {
	guint button;
	unsigned x, y;
	AssetId result;

	gdk_event_get_button (event, &button);
	engine_coords(&engine, square, &x, &y);
	switch (button) {
	/* left click */
	case 1:
		board_open(x, y);
		return;
	/* middle click */
	case 2:
		if (engine_count(&engine, square, Flag) == engine_count(&engine, square, Mine)) {
			board_open_neighbors(x, y);
			return;
		}
		break;
	/* right click */
	case 3:
		result = engine_flag(&engine, square);
		gtk_label_set_text(GTK_LABEL (flags), stringify(engine.flags));
		if (result != -1) {
			gtk_image_set_from_file (
				GTK_IMAGE (gtk_bin_get_child (GTK_BIN (event_box))),
				ASSETS[result]
			);
		}
		break;
	}
}

#define SIZE 10

static void board_init (GtkWidget *widget, gpointer data) {
	unsigned x, y;

	engine_alloc(&engine, SIZE, SIZE, SIZE);
	gtk_label_set_text(GTK_LABEL (flags), stringify(engine.flags));


	/* Clear board */
	gtk_container_foreach (GTK_CONTAINER (game_board), (void*) gtk_widget_destroy, NULL);

	for (y = 0; y < SIZE; y += 1) {
		for (x = 0; x < SIZE; x += 1) {
			/* Couldn't use Gtk.Button (and thus gtk4) because it adds padding */
			GtkWidget *event_box = gtk_event_box_new ();
			gtk_container_add(GTK_CONTAINER (event_box), gtk_image_new_from_file("assets/closed.png"));
			gtk_grid_attach (GTK_GRID (game_board), event_box, x, y, 1, 1);
			g_signal_connect (
				event_box,
				"button-release-event",
				G_CALLBACK (board_click),
				engine_ptr(&engine, x, y)
			);
		}
	}


}

void gui_activate (GtkApplication *app) {
	assert(game_board == NULL);
	assert(flags == NULL);

	GtkWidget *window = gtk_application_window_new (app);
	GtkWidget *grid = gtk_grid_new ();
	game_board = gtk_grid_new ();
	flags = gtk_label_new ("");

	/* Structure */
	gtk_container_add (GTK_CONTAINER (window), grid);
	gtk_grid_attach (GTK_GRID (grid), flags, 0, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), game_board, 0, 2, 3, 1);

	/* Properties */
	gtk_window_set_resizable (GTK_WINDOW (window), false);
	gtk_window_set_title (GTK_WINDOW (window), "Mines");
	gtk_widget_set_margin_bottom (game_board, 19);
	gtk_widget_set_margin_start (game_board, 19);
	gtk_widget_set_margin_end (game_board, 19);

	board_init (NULL, NULL);
	gtk_widget_show_all (window);
}
