include Makefile.FLTK

.PHONY: all clean cleanall

all: main

clean:
	@rm -rf *.o

cleanall: clean
	rm main

