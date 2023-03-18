#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include <algorithm>
#include <net.h>
#include <string>
#include <chrono>

class TargetBox
{
private:
    float getWidth() { return (x2 - x1); };
    float getHeight() { return (y2 - y1); };

public:
    int x1;
    int y1;
    int x2;
    int y2;

    int cate;
    float score;
    float area() { return getWidth() * getHeight(); };
};

class Yolo
{
public:
    // Yolo();
    // std::vector<TargetBox> boxes;
    int init(const bool use_vulkan_compute = false);
    int loadModel();
    int loadData();

    void frame(const cv::Mat srcImg);
    int dist_stop;
    int dist_car;

private:
    ncnn::Net net;

    const char *paramPath = "../model/yolov2.param";
    const char *binPath = "../model/yolov2.bin";

    std::vector<float> anchor;

    int inputWidth = 352, inputHeight = 352;
    int numThreads = 4;

    int numAnchor = 3;
    int numCategory = 80;

    int nmsThresh = 0.25;

    cv::String dataPath = "../test.mp4";
    std::stringstream ssDis_Stop;
    std::stringstream ssDis_Car;

    int detect(const cv::Mat srcImg, std::vector<TargetBox> &dstBoxes, const float thresh = 0.3);

    void draw(cv::Mat cvImg, const std::vector<TargetBox> &boxes);

    int predHandle(const ncnn::Mat *out, std::vector<TargetBox> &dstBoxes,
                   const float scaleW, const float scaleH, const float thresh);
    int getCategory(const float *values, int index, int &category, float &score);
    int nmsHandle(std::vector<TargetBox> &tmpBoxes, std::vector<TargetBox> &dstBoxes);

    float intersection_area(const TargetBox &a, const TargetBox &b);
};