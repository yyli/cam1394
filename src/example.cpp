#include <iostream>
#include "cv.h"
#include "camera.h"
#include "Timer.hpp"

using namespace std;
using namespace cv;
using namespace cam1394;

int main() {
        camera a;
        Mat aimage;
	Timer camRead;

        if (a.open("NONE", "640x480_MONO8", 60, NULL, NULL) < 0)
                return -1;

        int numDropped = 0;
        while (1) {
		camRead.start();
                aimage = a.read();
		camRead.end();
                numDropped += a.getNumDroppedFrames();
                cout << camRead.elapsed()*1000 << "ms, " << numDropped << endl;
        }
        return 0;
}
