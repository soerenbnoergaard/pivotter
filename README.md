# pivotter

Real-time pivt plotting tool. Branch in C++.

# Build procedure

Download FLTK 

    wget https://github.com/fltk/fltk/archive/release-1.3.5.tar.gz
    tar xzvf release-1.3.5.tar.gz
    cd fltk-release-1.3.5

For Windows (msys2, 64 bit):

    ./autogen.sh --mingw64
    make

For Linux:

    ./autogen.sh
    make


# TODO

- Add FLTK as submodule (and update Makefile.FLTK)
