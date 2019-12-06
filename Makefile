include Makefile.FLTK

.PHONY: all clean

TARGET = pivotter
INCLUDES = -I.

all: gui.o
	@g++ -std=c++11 $(INCLUDES) -c pivotter.cxx
	@g++ -std=c++11 pivotter.o gui.o -o $(TARGET)


