CC = g++
CFLAGS = -g -std=c++11 $(OPTIMIZE) -lglfw -lGL -lX11 -lpthread -lXrandr -lXi
SOURCES = Source.cpp
EXECS = main.bin
BREAKPOINTS = 
OPTIMIZE = ./glad/glad.o

all: clean $(SOURCES) $(EXECS)

run: all
	./main.bin

main.bin: Source.cpp
	$(CC) $< -o $@ $(BREAKPOINTS) $(CFLAGS)

clean:
	rm -f $(EXECS)

d: BREAKPOINTS = -D MYDEBUG_BREAK
d: all

opt: OPTIMIZE = -O2
opt: all
