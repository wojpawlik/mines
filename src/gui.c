/* Wojciech Pawlik */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "engine.h"
#include "gui.h"

static GtkWidget *game_board, *flags, *stopwatch;
static Engine engine = {0};
static bool is_game_over;
static GTimer *timer;

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
	"assets/mine.png",
	"assets/no_mine.png",
};

gchar *stringify(int16_t n) {
	static gchar s[6];
	sprintf(s, "%d", n);
	return s;
}

static void board_update (GtkWidget *event_box, AssetId result) {
	if (result == -1) return;
	gtk_image_set_from_file (
		GTK_IMAGE (gtk_bin_get_child (GTK_BIN (event_box))),
		ASSETS[result]
	);
}

gboolean time_update (void*) {
	gtk_label_set_text(
		GTK_LABEL (stopwatch),
		stringify (round (g_timer_elapsed (timer, NULL)))
	);
	return !is_game_over;
}

static void game_over (AssetId unmarked_mine) {
	unsigned x, y;
	is_game_over = true;
	for (y = 0; y < engine.height; y += 1) {
		for (x = 0; x < engine.width; x += 1) {
			board_update(
				gtk_grid_get_child_at (GTK_GRID (game_board), x, y),
				engine_reveal (engine_ptr (&engine, x, y), unmarked_mine)
			);
		}
	}
}

static void board_open_neighbors (unsigned x, unsigned y);

static void board_open (unsigned x, unsigned y) {
	GtkWidget *event_box = gtk_grid_get_child_at(GTK_GRID (game_board), x, y);
	AssetId result = engine_open(&engine, engine_ptr(&engine, x, y));
	if (result == 0) board_open_neighbors(x, y);
	if (result == 11) game_over(12);
	board_update(event_box, result);
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

static void board_click (GtkWidget *event_box, gpointer event, EngineSquare *square) {
	guint button;
	unsigned x, y;
	AssetId result;

	if (is_game_over) return;
	gdk_event_get_button (event, &button);
	engine_coords(&engine, square, &x, &y);
	switch (button) {
	/* left click */
	case 1:
		if (engine.opened == 0) {
			g_timer_start (timer);
			g_timeout_add (250, time_update, NULL);
		}
		board_open(x, y);
		break;
	/* middle click */
	case 2:
		if (*square == Open && engine_count(&engine, square, Flag) == engine_count(&engine, square, Mine)) {
			board_open_neighbors(x, y);
		}
		break;
	/* right click */
	case 3:
		result = engine_flag(&engine, square);
		gtk_label_set_text(GTK_LABEL (flags), stringify(engine.flags));
		board_update(event_box, result);
		return;
	}
	if (engine.opened == engine.width * engine.height - engine.mines) {
		game_over(10);
	}
}

#define SIZE 10

static void board_init (GtkWidget*, gpointer) {
	unsigned x, y;

	is_game_over = false;
	engine_alloc(&engine, SIZE, SIZE, SIZE);
	gtk_label_set_text(GTK_LABEL (stopwatch), "0");
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
	PangoFontDescription* monospace = pango_font_description_from_string ("bold monospace");
	GtkWidget *window = gtk_application_window_new (app);
	GtkWidget *grid = gtk_grid_new ();
	stopwatch = gtk_label_new ("");
	game_board = gtk_grid_new ();
	flags = gtk_label_new ("");
	timer = g_timer_new ();

	/* Structure */
	gtk_container_add (GTK_CONTAINER (window), grid);
	gtk_grid_attach (GTK_GRID (grid), flags, 0, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), stopwatch, 2, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), game_board, 0, 2, 3, 1);

	/* Properties */
	gtk_window_set_resizable (GTK_WINDOW (window), false);
	gtk_window_set_title (GTK_WINDOW (window), "Mines");
	gtk_widget_set_halign (stopwatch, GTK_ALIGN_END);
	gtk_widget_set_halign (flags, GTK_ALIGN_START);
	gtk_widget_set_halign (grid, GTK_ALIGN_CENTER);
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	gtk_widget_override_font (stopwatch, monospace);
	gtk_widget_override_font (flags, monospace);
	gtk_widget_set_margin_bottom (grid, 19);
	gtk_widget_set_margin_start (grid, 19);
	gtk_widget_set_margin_end (grid, 19);

	srand (time (NULL));
	board_init (NULL, NULL);
	gtk_widget_show_all (window);
}
