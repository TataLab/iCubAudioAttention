//
// Created by jonas on 10/20/17.
//

/**
  @file videocapture_basic.cpp
  @brief A very basic sample for using VideoCapture and VideoWriter
  @author PkLab.net
  @date Aug 24, 2016
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Log.h>

using namespace cv;
using namespace std;
using namespace yarp::sig;
using namespace yarp::os;
using namespace yarp::dev;

int main(int, char**)
{
    Network yarp;
    Mat frame;
    //--- INITIALIZE VIDEOCAPTURE
    VideoCapture cap;
    // open the default camera using default API
    cap.open(0);
    // OR advance usage: select any API backend
    int deviceID = 0;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API
    // open selected camera using selected API
    cap.open(deviceID + apiID);
    // check if we succeeded
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    //--- GRAB AND WRITE LOOP
    cout << "Start grabbing" << endl
         << "Press any key to terminate" << endl;
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelBgr> > cameraStreamPort;
    cameraStreamPort.open("/webCamRGB:o");

    for (;;)
    {
        // wait for a new frame from camera and store it into 'frame'
        cap.read(frame);
        // check if we succeeded
        if (frame.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }
        // show live and wait for a key with timeout long enough to show images
        //imshow("Live", frame);



        yarp::sig::ImageOf<yarp::sig::PixelBgr> *processingRgbImage = &cameraStreamPort.prepare();
        processingRgbImage->resize(frame.cols, frame.rows);
        IplImage output = (IplImage) frame;
        processingRgbImage->zero();
        IplImage *yarpImage = (IplImage*) processingRgbImage->getIplImage();

        cvCopy(&output, yarpImage);
        cameraStreamPort.write();

    }

    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}