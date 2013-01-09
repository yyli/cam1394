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

		a.printConnectedCams();

        //if (a.open("00B09D0100AF05C1") < 0)
        if (a.open("00B09D0100AF05C") < 0)
                return -1;

		a.printGUID();
		a.printVideoMode();
		a.printFrameRate();
		
        int numDropped = 0;
        while (1) {
			camRead.start();
			aimage = a.read();
			camRead.end();
			numDropped += a.getNumDroppedFrames();
			imshow("test", aimage);
			waitKey(5);
			cout << camRead.elapsed()*1000 << "ms, " << numDropped << endl;
        }
        return 0;
}
