/* Wojciech Pawlik */

#include <stdbool.h>
#include <stdlib.h>

#include "engine.h"

static const EngineSquare NOWHERE = Open;

void engine_free (Engine *engine) {
	free(engine->board);
	engine->board = NULL;
}

void engine_alloc (Engine *engine, unsigned width, unsigned height, int mines) {
	engine_free(engine);
	engine->opened = 0;
	engine->flags = mines;
	engine->mines = mines;
	engine->width = width;
	engine->height = height;
	engine->board = calloc(width * height, sizeof(EngineSquare));
}

EngineSquare* engine_ptr (Engine *engine, unsigned x, unsigned y) {
	/* Avoids multiple bounds- and NULL-checks */
	if (x >= engine->width || y >= engine->height) {
		return (EngineSquare*) &NOWHERE;
	}
	return engine->board + y * engine->width + x;
}

void engine_coords (Engine *engine, EngineSquare *square, unsigned *x, unsigned *y) {
	int n = square - engine->board;
	*x = n % engine->width;
	*y = n / engine->width;
}

static void engine_plant (Engine *engine) {
	int i, n;
	for (i = 0; i < engine->flags; i++) {
		do {
			n = rand() % (engine->width * engine->height);
		} while (engine->board[n]);
		engine->board[n] = Mine;
	}
}

unsigned engine_count (Engine *engine, EngineSquare *square, EngineSquare type) {
	unsigned x, y;
	engine_coords(engine, square, &x, &y);

	return (
		  !!(*engine_ptr(engine, x - 1, y - 1) & type)
		+ !!(*engine_ptr(engine, x - 1, y + 1) & type)
		+ !!(*engine_ptr(engine, x - 1, y) & type)

		+ !!(*engine_ptr(engine, x, y - 1) & type)
		+ !!(*engine_ptr(engine, x, y + 1) & type)

		+ !!(*engine_ptr(engine, x + 1, y) & type)
		+ !!(*engine_ptr(engine, x + 1, y - 1) & type)
		+ !!(*engine_ptr(engine, x + 1, y + 1) & type)
	);
}

bool engine_open (Engine *engine, EngineSquare *square) {
	if (*square & (Flag | Open)) return false;
	*square |= Open;
	if (!engine->opened) engine_plant(engine);
	engine->opened++;
	return true;
}

void engine_flag (Engine *engine, EngineSquare *square) {
	if (*square & Open) return;
	const int flagged = (*square ^= Flag) & Flag;
	engine->flags += flagged ? -1 : 1;
}
