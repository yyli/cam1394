CXX = g++
CXXFLAGS = -g -Wall -Isrc
CXXLD = -ldc1394

CXXOPENCVFLAGS = `pkg-config opencv --cflags`
CXXOPENCVLD = `pkg-config opencv --libs`

CXXFLAGS += $(CXXOPENCVFLAGS)
CXXLD += $(CXXOPENCVLD)

BUILDDIR=build

.PHONY: all clean doxygen

all: example example_auto example_onthefly getCams

# EXECUTABLES FILES HERE:

getCams: src/getCams.cpp $(BUILDDIR)/camera.o
	@echo "CC [$@]"
	@mkdir -p build
	@$(CXX) $? -o $(BUILDDIR)/$@ $(CXXFLAGS) $(CXXLD)

example: src/example.cpp $(BUILDDIR)/camera.o
	@echo "CC [$@]"
	@mkdir -p build
	@$(CXX) $? -o $(BUILDDIR)/$@ $(CXXFLAGS) $(CXXLD)

example_auto: src/example_auto.cpp $(BUILDDIR)/camera.o
	@echo "CC [$@]"
	@mkdir -p build
	@$(CXX) $? -o $(BUILDDIR)/$@ $(CXXFLAGS) $(CXXLD)

example_onthefly: src/example_onthefly.cpp $(BUILDDIR)/camera.o
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
