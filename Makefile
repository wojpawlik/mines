# Wojciech Pawlik

SRC := $(wildcard src/*.c)
DEP := $(SRC:src/%.c=lib/%.d)
OBJ := $(SRC:src/%.c=lib/%.o)
LDLIBS := -lm `pkg-config --libs gtk+-3.0`
CFLAGS := -Wall -Wextra -O2 -MMD `pkg-config --cflags gtk+-3.0`

# Executable
lib/mines: lib/assets.o $(OBJ)
	${CC} $^ -o $@ ${LDLIBS}

# Object files
lib/%.o:: src/%.c
	${CC} -c $< -o $@ ${CFLAGS}

lib/assets.c: assets/*
	mkdir -p lib/
	glib-compile-resources \
		--sourcedir assets/ \
		--generate-source    \
		--target lib/assets.c \
		assets/Index.gresource.xml

# Dependencies
# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
-include $(DEP)

.PHONY: clean run
clean:
	rm lib/*
run: lib/mines
	$^
