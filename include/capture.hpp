#include <wiringPi.h>
#include <raspicam_cv.h>
#include <opencv2/opencv.hpp>
#include <vector>

class capture
{
public:
    std::vector<int> histogramLane, histogramLane_end;
    int leftLanePos, rightLanePos, laneCenterPos, frameCenter, Result, laneEnd;
    std::stringstream ss;

    raspicam::RaspiCam_Cv camera;
    cv::Mat original, frame, Matrix, framePerspective, frameGray,
        frameThreshold, frameCanny, frameFinal, frameFinalDuplicate,
        ROILane, detection, ROILane_end, frameFinalDuplicate_end;

    void perspectiveTrans();
    void Threshold();
    void Histogram();
    void laneFinder();
    void laneCenter();
};