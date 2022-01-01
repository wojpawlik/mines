/* Wojciech Pawlik */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "engine.h"
#include "gui.h"

const char *GAME_DESCRIPTION =
	"Open all unmined squares.\n\n"
	"Left click a closed square to open it and reveal mine count of adjacent squares.\n"
	"Right click a closed square to (un-)flag it as mined.\n"
	"Middle click an open square to open all (unflagged) adjacent squares.\n"
	"Press F2 to restart the game.";

static GtkWidget *description, *difficulty, *game_board;
static GtkWidget *custom, *height, *width, *mines;
static GtkWidget *flags, *status, *stopwatch;
static Engine engine = {0};
static bool is_game_over;
static GTimer *timer;

static const char *ASSETS[] = {
	"/pl/edu/uj/student/wojpawlik/mines/assets/open.gif",
	"/pl/edu/uj/student/wojpawlik/mines/assets/one.gif",
	"/pl/edu/uj/student/wojpawlik/mines/assets/two.gif",
	"/pl/edu/uj/student/wojpawlik/mines/assets/three.gif",
	"/pl/edu/uj/student/wojpawlik/mines/assets/four.gif",
	"/pl/edu/uj/student/wojpawlik/mines/assets/five.gif",
	"/pl/edu/uj/student/wojpawlik/mines/assets/six.gif",
	"/pl/edu/uj/student/wojpawlik/mines/assets/seven.gif",
	"/pl/edu/uj/student/wojpawlik/mines/assets/eight.gif",
};

static const char *ASSET_CLICKED_MINE = "/pl/edu/uj/student/wojpawlik/mines/assets/clicked_mine.gif";
static const char *ASSET_CLOSED = "/pl/edu/uj/student/wojpawlik/mines/assets/closed.gif";
static const char *ASSET_FLAG = "/pl/edu/uj/student/wojpawlik/mines/assets/flag.gif";
static const char *ASSET_MINE = "/pl/edu/uj/student/wojpawlik/mines/assets/mine.gif";
static const char *ASSET_NO_MINE = "/pl/edu/uj/student/wojpawlik/mines/assets/no_mine.gif";
static const char *ASSET_PRESSED = "/pl/edu/uj/student/wojpawlik/mines/assets/pressed.gif";
static const char *ASSET_UNFLAG = "/pl/edu/uj/student/wojpawlik/mines/assets/unflag.gif";

static gchar *stringify(int16_t n) {
	static gchar s[7];
	sprintf(s, "%d", n);
	return s;
}

static void board_update (GtkWidget *event_box, const char *asset) {
	gtk_image_set_from_resource (
		GTK_IMAGE (gtk_bin_get_child (GTK_BIN (event_box))),
		asset
	);
}

static gboolean time_update () {
	if (is_game_over || !engine.opened) return false;
	gtk_label_set_text(
		GTK_LABEL (stopwatch),
		stringify (round (g_timer_elapsed (timer, NULL)))
	);
	return true;
}

static void game_over (const char *unmarked_mine) {
	unsigned x, y;
	if (is_game_over) return;
	is_game_over = true;
	for (y = 0; y < engine.height; y += 1) {
		for (x = 0; x < engine.width; x += 1) {
			EngineSquare *square = engine_ptr (&engine, x, y);
			GtkWidget *event_box = gtk_grid_get_child_at (GTK_GRID (game_board), x, y);
			if (*square == Mine) board_update (event_box, unmarked_mine);
			if (*square == Flag) board_update (event_box, ASSET_NO_MINE);
		}
	}
}

static void board_open_neighbors (unsigned x, unsigned y);

static void board_open (unsigned x, unsigned y) {
	GtkWidget *event_box = gtk_grid_get_child_at (GTK_GRID (game_board), x, y);
	EngineSquare *square = engine_ptr (&engine, x, y);
	if (!engine_open (&engine, square)) return;
	if (*square & Mine) {
		gtk_label_set_text (GTK_LABEL (status), "You lost. F2 to restart.");
		board_update (event_box, ASSET_CLICKED_MINE);
		game_over(ASSET_MINE);
		return;
	}
	unsigned count = engine_count (&engine, square, Mine);
	if (count == 0) board_open_neighbors (x, y);
	board_update (event_box, ASSETS[count]);
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

static void board_press_neighbors (int x, int y, const char *asset) {
	int i, j;
	for (i = -1; i <= 1; i++) for (j = -1; j <= 1; j++) {
		EngineSquare *square = engine_ptr (&engine, x + i, y + j);
		GtkWidget *event_box = gtk_grid_get_child_at (GTK_GRID (game_board), x + i, y + j);
		if (!(*square & (Flag | Open))) board_update (event_box, asset);
	}
}

static void board_press (GtkWidget *event_box, gpointer event, EngineSquare *square) {
	guint button;
	if (is_game_over) return;
	gdk_event_get_button (event, &button);
	if (button == 1 /* Left click */ && !(*square & Open)) {
		board_update (event_box, ASSET_PRESSED);
	} else if (button == 2 /* middle click */) {
		unsigned x, y;
		engine_coords (&engine, square, &x, &y);
		board_press_neighbors (x, y, ASSET_PRESSED);
	}
}

static void board_click (GtkWidget *event_box, gpointer event, EngineSquare *square) {
	guint button;
	unsigned x, y;

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
		} else {
			board_press_neighbors (x, y, ASSET_CLOSED);
		}
		break;
	/* right click */
	case 3:
		engine_flag (&engine, square);
		gtk_label_set_text (GTK_LABEL (flags), stringify (engine.flags));
		if (*square & Flag) board_update (event_box, ASSET_FLAG);
		else if (!(*square & Open)) board_update (event_box, ASSET_UNFLAG);
		return;
	}
	if (engine.opened == engine.width * engine.height - engine.mines) {
		gtk_label_set_text (GTK_LABEL (status), "You won! F2 to restart.");
		game_over (ASSET_FLAG);
	}
}

static void board_init (void) {
	unsigned x, y;
	unsigned mines_ = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (mines));
	unsigned width_ = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (width));
	unsigned height_ = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (height));

	is_game_over = false;
	engine_alloc (&engine, width_, height_, mines_);
	gtk_label_set_text (GTK_LABEL (stopwatch), "0");
	gtk_label_set_text (GTK_LABEL (flags), stringify(engine.flags));
	gtk_label_set_text (GTK_LABEL (status), "");

	/* Clear board */
	gtk_container_foreach (GTK_CONTAINER (game_board), (void*) gtk_widget_destroy, NULL);

	for (y = 0; y < height_; y++) {
		for (x = 0; x < width_; x++) {
			/* Couldn't use Gtk.Button (and thus gtk4) because it adds padding */
			GtkWidget *event_box = gtk_event_box_new ();
			EngineSquare *square = engine_ptr (&engine, x, y);
			gtk_container_add (
				GTK_CONTAINER (event_box),
				gtk_image_new_from_resource (ASSET_CLOSED)
			);
			gtk_grid_attach (GTK_GRID (game_board), event_box, x, y, 1, 1);
			g_signal_connect (
				event_box,
				"button-release-event",
				G_CALLBACK (board_click),
				square
			);
			g_signal_connect (
				event_box,
				"button-press-event",
				G_CALLBACK (board_press),
				square
			);
		}
	}

	gtk_widget_show_all (game_board);
	gtk_widget_hide (difficulty);
}

static void mines_range_update (void) {
	static const double MINE_RATIO_BEGINNER = .12345;
	static const double MINE_RATIO_EXPERT = .20625;
	double area = gtk_spin_button_get_value (GTK_SPIN_BUTTON (width)) * gtk_spin_button_get_value (GTK_SPIN_BUTTON (height));
	gtk_spin_button_set_range (
		GTK_SPIN_BUTTON (mines),
		area * MINE_RATIO_BEGINNER,
		area * MINE_RATIO_EXPERT
	);
}

static void difficulty_set_preset (GtkButton* _, char *difficulty) {
	(void) _; /* unused */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (width), difficulty[0]);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (height), difficulty[1]);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (mines), difficulty[2]);
	gtk_widget_set_sensitive (width, false);
	gtk_widget_set_sensitive (height, false);
	gtk_widget_set_sensitive (mines, false);
}

static void difficulty_set_custom (void) {
	gtk_widget_set_sensitive (width, true);
	gtk_widget_set_sensitive (height, true);
	gtk_widget_set_sensitive (mines, true);
}

static void gui_difficulty_init (void) {
	static char DIFFICULTIES[] = {
		 9,  9, 10, /* Beginner */
		16, 16, 40, /* Intermediate */
		30, 16, 99, /* Expert */
	};
	GtkWidget *beginner = gtk_radio_button_new_with_mnemonic (NULL, "_Beginner");
	GtkWidget *intermediate = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (beginner), "_Intermediate");
	GtkWidget *expert = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (beginner), "_Expert");
	custom = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (beginner), "_Custom");
	GtkWidget *height_label = gtk_label_new_with_mnemonic("_Height");
	GtkWidget *width_label = gtk_label_new_with_mnemonic("_Width");
	GtkWidget *mines_label = gtk_label_new_with_mnemonic("_Mines");
	GtkWidget *new_game = gtk_button_new_with_mnemonic ("_New game");
	mines = gtk_spin_button_new_with_range (10, 17, 1);
	height = gtk_spin_button_new_with_range (9, 16, 1);
	width = gtk_spin_button_new_with_range (9, 30, 1);
	difficulty = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget *credits = gtk_label_new (NULL);
	GtkWidget *grid = gtk_grid_new ();

	/* Structure */
	gtk_container_add (GTK_CONTAINER (difficulty), grid);
	gtk_label_set_mnemonic_widget (GTK_LABEL (width_label), width);
	gtk_label_set_mnemonic_widget (GTK_LABEL (height_label), height);
	gtk_label_set_mnemonic_widget (GTK_LABEL (mines_label), mines);
	gtk_grid_attach (GTK_GRID (grid), width_label,	1, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), height_label,	1, 2, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), mines_label,	1, 3, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), width,	2, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), height,	2, 2, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), mines,	2, 3, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), new_game,	2, 4, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), beginner,	3, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), intermediate,	3, 2, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), expert,	3, 3, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), custom,	3, 4, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), credits,	1, 5, 3, 1);

	/* Signals */
	g_signal_connect (G_OBJECT (beginner),	"clicked",	G_CALLBACK (difficulty_set_preset),	DIFFICULTIES);
	g_signal_connect (G_OBJECT (intermediate),	"clicked",	G_CALLBACK (difficulty_set_preset),	DIFFICULTIES + 3);
	g_signal_connect (G_OBJECT (expert),	"clicked",	G_CALLBACK (difficulty_set_preset),	DIFFICULTIES + 6);
	g_signal_connect (G_OBJECT (custom),	"clicked",	G_CALLBACK (difficulty_set_custom),	NULL);
	g_signal_connect (G_OBJECT (difficulty),	"delete-event",	G_CALLBACK (gtk_widget_hide_on_delete),	NULL);
	g_signal_connect (G_OBJECT (height),	"value-changed",	G_CALLBACK (mines_range_update),	NULL);
	g_signal_connect (G_OBJECT (width),	"value-changed",	G_CALLBACK (mines_range_update),	NULL);
	g_signal_connect (G_OBJECT (new_game),	"clicked",	G_CALLBACK (board_init),	NULL);

	/* Properties */
	gtk_window_set_title (GTK_WINDOW (difficulty), "Difficulty");
	gtk_window_set_resizable (GTK_WINDOW (difficulty), false);
	gtk_window_set_modal (GTK_WINDOW (difficulty), true);
	gtk_widget_set_sensitive (width, false);
	gtk_widget_set_sensitive (height, false);
	gtk_widget_set_sensitive (mines, false);
	gtk_widget_set_margin_bottom (grid, 6);
	gtk_widget_set_margin_start (grid, 6);
	gtk_widget_set_margin_end (grid, 6);
	gtk_label_set_markup (
		GTK_LABEL (credits),
		"Game by <a href=\"https://github.com/wojpawlik\">Wojciech Pawlik</a>, "
		"artwork by <a href=\"https://www.spriters-resource.com/submitter/striker212/\">striker212</a>"
	);
}

static void gui_window_show_cb (void *_, GtkWidget *window) {
	(void) _; /* unused */
	gtk_widget_show_all (window);
}

void gui_key_press (GtkWidget* _, GdkEventKey *event) {
	(void) _; /* unused */
	if (event->state) return;
	switch (event->keyval) {
	case GDK_KEY_F1:
		gtk_dialog_run (GTK_DIALOG (description));
		gtk_widget_hide (description);
		break;
	case GDK_KEY_F2:
		board_init ();
		break;
	}
}

void gui_activate (GtkApplication *app) {
	PangoFontDescription* monospace = pango_font_description_from_string ("bold monospace bold 19");
	GtkWidget *new_game = gtk_button_new_from_icon_name ("view-refresh-symbolic", 1);
	GtkWidget *window = gtk_application_window_new (app);
	GtkWidget *title_bar = gtk_header_bar_new ();
	GtkWidget *grid = gtk_grid_new ();
	stopwatch = gtk_label_new ("");
	game_board = gtk_grid_new ();
	status = gtk_label_new ("");
	flags = gtk_label_new ("");
	timer = g_timer_new ();
	gui_difficulty_init ();
	description = gtk_message_dialog_new (
		GTK_WINDOW (window),
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		GAME_DESCRIPTION
	);

	/* Structure */
	gtk_container_add (GTK_CONTAINER (window), grid);
	gtk_grid_attach (GTK_GRID (grid), flags, 0, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), status, 1, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), stopwatch, 2, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), game_board, 0, 2, 3, 1);
	gtk_window_set_titlebar (GTK_WINDOW (window), title_bar);
	gtk_header_bar_pack_start (GTK_HEADER_BAR (title_bar), new_game);
	gtk_window_set_transient_for (GTK_WINDOW (difficulty), GTK_WINDOW (window));

	/* Signals */
	g_signal_connect (new_game, "clicked", G_CALLBACK (gui_window_show_cb), difficulty);
	g_signal_connect (window, "key-release-event", G_CALLBACK (gui_key_press), NULL);

	/* Properties */
	gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (title_bar), true);
	gtk_window_set_resizable (GTK_WINDOW (window), false);
	gtk_window_set_title (GTK_WINDOW (window), "Mines");
	gtk_widget_set_halign (stopwatch, GTK_ALIGN_END);
	gtk_widget_set_halign (flags, GTK_ALIGN_START);
	gtk_widget_set_halign (grid, GTK_ALIGN_CENTER);
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	gtk_widget_override_font (stopwatch, monospace);
	gtk_widget_override_font (flags, monospace);
	gtk_widget_set_margin_bottom (grid, 6);
	gtk_widget_set_margin_start (grid, 6);
	gtk_widget_set_margin_end (grid, 6);

	srand (time (NULL));
	board_init ();
	gtk_widget_show_all (window);
}
