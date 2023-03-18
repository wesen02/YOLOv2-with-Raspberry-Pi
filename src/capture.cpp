#include <capture.hpp>

void capture::perspectiveTrans()
{
    cv::Point2f Source[] = {cv::Point2f(65, 180), cv::Point2f(290, 160), cv::Point2f(40, 230), cv::Point2f(320, 210)};
    cv::Point2f Destination[] = {cv::Point2f(100, 0), cv::Point2f(300, 0), cv::Point2f(100, 220), cv::Point2f(300, 220)};

    line(frame, Source[0], Source[1], cv::Scalar(0, 0, 255), 2);
    line(frame, Source[1], Source[3], cv::Scalar(0, 0, 255), 2);
    line(frame, Source[3], Source[2], cv::Scalar(0, 0, 255), 2);
    line(frame, Source[2], Source[0], cv::Scalar(0, 0, 255), 2);

    Matrix = getPerspectiveTransform(Source, Destination);
    warpPerspective(frame, framePerspective, Matrix, cv::Size(400, 240));
}

void capture::Threshold()
{
    cvtColor(framePerspective, frameGray, cv::COLOR_RGB2GRAY);
    inRange(frameGray, 150, 255, frameThreshold); // This can be change 230 to reduce the noise beside the road increase to remove the noise
    Canny(frameGray, frameCanny, 900, 900, 3, false);
    add(frameThreshold, frameCanny, frameFinal);
    cvtColor(frameFinal, frameFinal, cv::COLOR_GRAY2RGB);
    cvtColor(frameFinal, frameFinalDuplicate, cv::COLOR_RGB2BGR); // Used in histogram function only
    cvtColor(frameFinal, frameFinalDuplicate_end, cv::COLOR_RGB2BGR);
}

void capture::Histogram()
{
    histogramLane.resize(400);
    histogramLane.clear();

    for (int i = 0; i < 400; i++) // frame.size().width = 400
    {
        ROILane = frameFinalDuplicate(cv::Rect(i, 140, 1, 100));
        cv::divide(255, ROILane, ROILane);
        histogramLane.push_back((int)(cv::sum(ROILane)[0]));
    }

    histogramLane_end.resize(400);
    histogramLane_end.clear();
    for (int i = 0; i < 400; i++) // frame.size().width = 400
    {
        ROILane_end = frameFinalDuplicate(cv::Rect(i, 140, 1, 100));
        cv::divide(255, ROILane_end, ROILane_end);
        histogramLane_end.push_back((int)(cv::sum(ROILane_end)[0]));
    }
    laneEnd = cv::sum(histogramLane_end)[0];
    std::cout << "Lane End = " << laneEnd << "\n";
}

void capture::laneFinder()
{
    std::vector<int>::iterator leftPtr;
    leftPtr = max_element(histogramLane.begin(), histogramLane.begin() + 150);
    leftLanePos = distance(histogramLane.begin(), leftPtr);

    std::vector<int>::iterator rightPtr;
    rightPtr = max_element(histogramLane.begin() + 250, histogramLane.end());
    rightLanePos = distance(histogramLane.begin(), rightPtr);

    line(frameFinal, cv::Point2f(leftLanePos, 0), cv::Point2f(leftLanePos, 240), cv::Scalar(0, 255, 0), 2);
    line(frameFinal, cv::Point2f(rightLanePos, 0), cv::Point2f(rightLanePos, 240), cv::Scalar(0, 255, 0), 2);
}

void capture::laneCenter()
{
    laneCenterPos = (rightLanePos - leftLanePos) / 2 + leftLanePos;

    frameCenter = 210; // Change the value to collabs the middle line with the default frame center.

    line(frameFinal, cv::Point2f(laneCenterPos, 0), cv::Point2f(laneCenterPos, 240), cv::Scalar(0, 255, 0), 3);
    line(frameFinal, cv::Point2f(frameCenter, 0), cv::Point2f(frameCenter, 240), cv::Scalar(255, 0, 0), 3);

    Result = laneCenterPos - frameCenter;
}