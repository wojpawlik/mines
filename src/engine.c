/* Wojciech Pawlik */

#include <stdlib.h>

#include "engine.h"

static const EngineSquare ZERO = 0;

void engine_free (Engine *engine) {
	free(engine->board);
	engine->board = NULL;
}

void engine_alloc (Engine *engine, unsigned width, unsigned height, int flags) {
	engine_free(engine);
	engine->opened = 0;
	engine->flags = flags;
	engine->width = width;
	engine->height = height;
	engine->board = calloc(width * height, sizeof(EngineSquare));
}

EngineSquare* engine_ptr (Engine *engine, unsigned x, unsigned y) {
	if (x >= engine->width || y >= engine->height) return (EngineSquare*) &ZERO;
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

AssetId engine_open (Engine *engine, EngineSquare *square) {
	if (square == &ZERO) return -1;
	if (*square & (Flag | Open)) return -1;
	*square |= Open;
	if (!engine->opened) engine_plant(engine);
	engine->opened++;
	if (*square & Mine) return 11;
	return engine_count(engine, square, Mine);
}

AssetId engine_flag (Engine *engine, EngineSquare *square) {
	if (*square & Open) return -1;
	const int flagged = (*square ^= Flag) & Flag;
	engine->flags += flagged ? -1 : 1;
	return 9 + flagged;
}
