#include <iostream>
#include "camera.h"
#include "Timer.hpp"

using namespace std;
using namespace cam1394;

int main() {
    camera a;
	Timer camRead;
	cam1394Image img;

	a.printConnectedCams();

    if (a.open("NONE", "640x480_MONO8", 60, "SIMPLE", "BGGR") < 0)
            return -1;

	a.printGUID();

	int i = 0;
    while (i++ < 50) {
    	if (a.read(&img) < 0)
    		return 1;

    	cout << img.size << " " << img.width << "x" << img.height << endl;
    }

    img.destroy();
    return 0;
}
