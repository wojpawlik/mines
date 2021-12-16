# Wojciech Pawlik

CFLAGS = -Wall -Wextra -lm `pkg-config --cflags --libs gtk+-3.0`

# Executable
lib/mines: lib/gui.o lib/main.o lib/engine.o lib/assets.o
	${CC} $^ -o $@ ${CFLAGS}

# Object files
lib/%.o:: src/%.c src/%.h
	${CC} -c $< -o $@ ${CFLAGS}

lib/assets.c: assets/*
	glib-compile-resources \
		--sourcedir assets/ \
		--generate-source    \
		--target lib/assets.c \
		assets/Index.gresource.xml

# Dependencies
lib/main.o:: src/gui.h
lib/gui.o:: src/engine.h

.PHONY: clean run
clean:
	rm lib/*
run: lib/mines
	$^
