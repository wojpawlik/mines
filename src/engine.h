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
	int opened;
	int flags;
} Engine;

typedef int AssetId;

void engine_alloc (Engine *engine, unsigned width, unsigned height, int flags);
void engine_free (Engine *engine);

EngineSquare* engine_ptr (Engine *engine, unsigned x, unsigned y);
void engine_coords (Engine *engine, EngineSquare *square, unsigned *x, unsigned *y);

unsigned engine_count (Engine *engine, EngineSquare *square, EngineSquare type);
AssetId engine_open (Engine *engine, EngineSquare *square);
AssetId engine_flag (Engine *engine, EngineSquare *square);
