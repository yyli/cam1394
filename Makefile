CXX = g++
CXXFLAGS = -O3 -Wall -Isrc
CXXLD = -ldc1394

CXXOPENCVFLAGS = `pkg-config opencv --cflags`
CXXOPENCVLD = `pkg-config opencv --libs`

CXXFLAGS += $(CXXOPENCVFLAGS)
CXXLD += $(CXXOPENCVLD)

BUILDDIR=build

.PHONY: all clean doxygen

all: camera example

# EXECUTABLES FILES HERE:

example: src/example.cpp $(BUILDDIR)/camera.o
	mkdir -p build
	$(CXX) $? -o $(BUILDDIR)/$@ $(CXXFLAGS) $(CXXLD)

# OBJECT FILES HERE:

camera: src/camera.cpp
	mkdir -p build
	$(CXX) $? -c -o $(BUILDDIR)/$@.o $(CXXFLAGS)

doxygen: 
	doxygen doxygen/Doxyfile

clean:
	rm -rf build
