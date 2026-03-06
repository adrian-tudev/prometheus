CC = clang++
FLAGS = -std=c++20 -g
CPPFLAGS = -I$(SRC_DIR)

SRC_DIR = src
TEST_DIR = tests
BUILDDIR = bin

OUT = prometheus
PERFT_OUT = perft
PERFT_SRC = $(TEST_DIR)/perft.cpp

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))
CORE_OBJS = $(filter-out $(BUILDDIR)/main.o,$(OBJS))
PERFT_OBJ = $(BUILDDIR)/perft.o

.PHONY: all run perft clean

all: $(BUILDDIR)/$(OUT)

run: $(BUILDDIR)/$(OUT)
	./$(BUILDDIR)/$(OUT)

$(BUILDDIR)/$(OUT): $(OBJS) | $(BUILDDIR)
	$(CC) $(OBJS) -o $@

$(BUILDDIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILDDIR)
	$(CC) $(CPPFLAGS) $(FLAGS) -c $< -o $@

perft: $(BUILDDIR)/$(PERFT_OUT)
	./$(BUILDDIR)/$(PERFT_OUT)

$(BUILDDIR)/$(PERFT_OUT): $(PERFT_OBJ) $(CORE_OBJS) | $(BUILDDIR)
	$(CC) $(PERFT_OBJ) $(CORE_OBJS) -o $@

$(PERFT_OBJ): $(PERFT_SRC) | $(BUILDDIR)
	$(CC) $(CPPFLAGS) $(FLAGS) -c $< -o $@

clean:
	rm -f $(BUILDDIR)/$(OUT) $(BUILDDIR)/$(PERFT_OUT) $(OBJS) $(PERFT_OBJ)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)
