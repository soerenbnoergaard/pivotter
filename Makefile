include Makefile.FLTK

.PHONY: all clean cleanall

OBJECTS = main.o source.o
TARGET = pivotter

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LINKFLTK_ALL) -o $@

clean:
	@rm -rf *.o

cleanall: clean
	rm $(TARGET)

