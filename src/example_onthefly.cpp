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

    if (a.open("00B09D0100AF05C1", "640x480_MONO8", 60, "SIMPLE", "BGGR") < 0)
        return -1;

    a.setVideoMode("1280x960_MONO8");

    a.printGUID();
    a.printVideoMode();
    a.printFrameRate();

    int numDropped = 0;
    int count = 200;
    while (1) {
        camRead.start();
        aimage = a.read();
        camRead.end();
        imshow("test", aimage);
        waitKey(5);
        numDropped += a.getNumDroppedFrames();
        //cout << camRead.elapsed()*1000 << "ms, " << numDropped << endl;

        if (count == 0) {}

        count--;
    }
    return 0;
}
