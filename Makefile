.PHONY: all clean

TARGET = pivotter
SOURCES = pivot.cpp
INCLUDES = -I.

all: 
	@g++ $(INCLUDES) $(SOURCES) -o $(TARGET)
