all: test-fit

GEOM=../libgeom
MARCHING=../marching

INCLUDES=-I$(GEOM) -I$(MARCHING) -I/usr/include/eigen3
LIBS=-L$(GEOM)/release -lgeom -L$(MARCHING)/build -lmarching -lasan

CXXFLAGS=-std=c++20 -Wall -pedantic -O3 -DNDEBUG $(INCLUDES)
#CXXFLAGS=-std=c++20 -Wall -pedantic -O0 -g -DDEBUG $(INCLUDES) -fsanitize=address

test-fit: test-fit.o quadric-fit.o fitter.o solver.o classifier.o
	$(CXX) -o $@ $^ $(LIBS)
