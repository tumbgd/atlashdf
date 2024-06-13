CFLAGS=-Ofast -march=native
all:
	g++ -c ${CFLAGS} -o prepare_ogl.o prepare_ogl.cpp -Wall `pkg-config --cflags --libs hdf5` -I ../include
	g++ -c ${CFLAGS} -o mask_renderer.o mask_renderer.cpp -Wall
	g++ -std=c++17 -o egl main.cpp prepare_ogl.o mask_renderer.o -lGLEW -lGL -lEGL -lGLU -lproj `pkg-config --cflags --libs hdf5` -I ../include ${CFLAGS}
