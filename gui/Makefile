CC = clang++
FLAGS = -std=c++20 -g
CPPFLAGS = -I../src

SDL_CFLAGS := $(shell pkg-config --cflags sdl3 sdl3-image 2>/dev/null)
SDL_LIBS := $(shell pkg-config --libs sdl3 sdl3-image 2>/dev/null)

BUILDDIR = ../bin
OUT = prometheus-gui

GUI_SRCS = main.cpp gui.cpp
GUI_OBJS = $(patsubst %.cpp,$(BUILDDIR)/gui_%.o,$(GUI_SRCS))

CORE_SRCS = \
	../src/bitboard.cpp \
	../src/movegen.cpp \
	../src/position.cpp \
	../src/utils.cpp

CORE_OBJS = $(patsubst ../src/%.cpp,$(BUILDDIR)/%.o,$(CORE_SRCS))

.PHONY: all run clean

all:
	@if [ -z "$(SDL_LIBS)" ]; then \
		echo "error: SDL3 and SDL3_image not found via pkg-config"; \
		echo "hint: install packages providing 'sdl3' and 'sdl3-image'"; \
		exit 1; \
	fi
	@$(MAKE) $(BUILDDIR)/$(OUT)

run: all
	$(BUILDDIR)/$(OUT)

$(BUILDDIR)/$(OUT): $(GUI_OBJS) $(CORE_OBJS) | $(BUILDDIR)
	$(CC) $(GUI_OBJS) $(CORE_OBJS) -o $@ $(SDL_LIBS)

$(BUILDDIR)/gui_%.o: %.cpp | $(BUILDDIR)
	$(CC) $(CPPFLAGS) $(FLAGS) $(SDL_CFLAGS) -c $< -o $@

# Build core engine objects in the shared bin/ folder.
$(BUILDDIR)/%.o: ../src/%.cpp | $(BUILDDIR)
	$(CC) $(CPPFLAGS) $(FLAGS) -c $< -o $@

clean:
	rm -f $(BUILDDIR)/$(OUT) $(GUI_OBJS)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)
