
CC = g++
SRC = src/*.cpp
OUT = prometheus
FLAGS = -std=c++17 -g

all:
	$(CC) $(SRC) $(FLAGS) -o bin/$(OUT)
