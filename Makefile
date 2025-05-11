all: test-fit

GEOM=../libgeom
MARCHING=../marching

INCLUDES=-I$(GEOM) -I$(MARCHING) -I/usr/include/eigen3
LIBS=-L$(GEOM)/release -lgeom -L$(MARCHING)/build -lmarching -lasan

CXXFLAGS=-std=c++20 -Wall -pedantic -O3 -DNDEBUG $(INCLUDES)
#CXXFLAGS=-std=c++20 -Wall -pedantic -O0 -g -DDEBUG $(INCLUDES) -fsanitize=address

libquadric.a: quadric-fit.o fitter.o solver.o classifier.o
	$(AR) rcs $@ $^

test-fit: test-fit.o libquadric.a
	$(CXX) -o $@ $< -L. -lquadric $(LIBS)
