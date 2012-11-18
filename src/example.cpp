#include <iostream>
#include "cv.h"
#include "camera.h"

using namespace std;
using namespace cv;
using namespace cam1394;

int main() {
        camera a;
        Mat aimage;

        if (a.open("NONE", "640x480_RGB8", 60, NULL, NULL) < 0)
                return -1;

        int numDropped = 0;
        while (1) {
                aimage = a.read();
                numDropped += a.getNumDroppedFrames();
                cout << numDropped << endl;
        }
        return 0;
}
