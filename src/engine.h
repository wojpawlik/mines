/* Wojciech Pawlik */

typedef enum {
	Flag = 1,
	Open = 2,
	Mine = 4,
} EngineSquare;

typedef struct {
	EngineSquare *board;
	unsigned width;
	unsigned height;
	unsigned opened;
	int mines;
	int flags;
} Engine;

void engine_alloc (Engine *engine, unsigned width, unsigned height, int flags);
void engine_free (Engine *engine);

EngineSquare* engine_ptr (Engine *engine, unsigned x, unsigned y);
void engine_coords (Engine *engine, EngineSquare *square, unsigned *x, unsigned *y);

unsigned engine_count (Engine *engine, EngineSquare *square, EngineSquare type);
bool engine_open (Engine *engine, EngineSquare *square);
void engine_flag (Engine *engine, EngineSquare *square);
