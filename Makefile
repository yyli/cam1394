CXX = g++
CXXFLAGS = -g -Wall -Isrc
CXXLD = -ldc1394

CXXOPENCVFLAGS = `pkg-config opencv --cflags`
CXXOPENCVLD = `pkg-config opencv --libs`

CXXFLAGS += $(CXXOPENCVFLAGS)
CXXLD += $(CXXOPENCVLD)

BUILDDIR=build

.PHONY: all clean doxygen

all: example_basic example_auto example_onthefly getCams

# EXECUTABLES FILES HERE:

getCams: src/getCams.cpp $(BUILDDIR)/camera.o
	@echo "CC [$@]"
	@mkdir -p build
	@$(CXX) $? -o $(BUILDDIR)/$@ $(CXXFLAGS) $(CXXLD)

example_basic: src/examples/basic.cpp $(BUILDDIR)/camera.o
	@echo "CC [$@]"
	@mkdir -p build
	@$(CXX) $? -o $(BUILDDIR)/$@ $(CXXFLAGS) $(CXXLD)

example_auto: src/examples/auto.cpp $(BUILDDIR)/camera.o
	@echo "CC [$@]"
	@mkdir -p build
	@$(CXX) $? -o $(BUILDDIR)/$@ $(CXXFLAGS) $(CXXLD)

example_onthefly: src/examples/onthefly.cpp $(BUILDDIR)/camera.o
	@echo "CC [$@]"
	@mkdir -p build
	@$(CXX) $? -o $(BUILDDIR)/$@ $(CXXFLAGS) $(CXXLD)

# OBJECT FILES HERE:

$(BUILDDIR)/camera.o: src/camera.cpp
	@echo "CC [$@]"
	@mkdir -p build
	@$(CXX) $? -c -o $@ $(CXXFLAGS)

doxygen: 
	doxygen doxygen/Doxyfile

clean:
	rm -rf build
