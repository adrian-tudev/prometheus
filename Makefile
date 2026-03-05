CC = clang++
SRC = src/*.cpp
OUT = prometheus
BUILDDIR = bin/
FLAGS = -std=c++20 -g

all: $(BUILDDIR)
	$(CC) $(SRC) $(FLAGS) -o bin/$(OUT)

$(BUILDDIR):
	mkdir $(BUILDDIR)
