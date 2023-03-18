#include <yolo.hpp>
#include <capture.hpp>

const char *class_names[] = {
    "background", "person", "bicycle",
    "car", "motorbike", "aeroplane", "bus", "train", "truck",
    "boat", "traffic light", "fire hydrant", "stop sign",
    "parking meter", "bench", "bird", "cat", "dog", "horse",
    "sheep", "cow", "elephant", "bear", "zebra", "giraffe",
    "backpack", "umbrella", "handbag", "tie", "suitcase",
    "frisbee", "skis", "snowboard", "sports ball", "kite",
    "baseball bat", "baseball glove", "skateboard", "surfboard",
    "tennis racket", "bottle", "wine glass", "cup", "fork",
    "knife", "spoon", "bowl", "banana", "apple", "sandwich",
    "orange", "broccoli", "carrot", "hot dog", "pizza", "donut",
    "cake", "chair", "sofa", "pottedplant", "bed", "diningtable",
    "toilet", "tvmonitor", "laptop", "mouse", "remote", "keyboard",
    "car", "microwave", "oven", "toaster", "sink",
    "refrigerator", "book", "clock", "vase", "scissors",
    "teddy bear", "hair drier", "toothbrush"};

int Yolo::init(const bool use_vulkan_compute)
{
    net.opt.use_winograd_convolution = true;
    net.opt.use_sgemm_convolution = true;
    net.opt.use_int8_inference = true;
    net.opt.use_vulkan_compute = use_vulkan_compute;
    net.opt.use_fp16_packed = true;
    net.opt.use_fp16_storage = true;
    net.opt.use_fp16_arithmetic = true;
    net.opt.use_int8_storage = true;
    net.opt.use_int8_arithmetic = true;
    net.opt.use_packing_layout = true;
    net.opt.use_shader_pack8 = false;
    net.opt.use_image_storage = false;

    return 0;
}

int Yolo::loadModel()
{
    net.load_param(paramPath);
    net.load_model(binPath);

    // std::cout << "Model Loaded Successfully"
    //           << "\n";
    return 0;
}

int Yolo::loadData()
{
    cv::VideoCapture video(0);

    if (!video.isOpened())
    {
        std::cout << "Error opening video file"
                  << "\n";
        // return;
    }

    cv::Mat frame;

    float f;
    float FPS[16];
    int i, Fcnt = 0;
    std::chrono::steady_clock::time_point Tbegin, Tend;

    while (true)
    {
        Tbegin = std::chrono::steady_clock::now();
        video >> frame;

        if (frame.empty())
        {
            std::cerr << "ERROR: Unable to grab from the source"
                      << "\n";
            break;
        }

        std::vector<TargetBox> boxes;
        // cv::namedWindow("Video", cv::WINDOW_NORMAL);
        detect(frame, boxes);
        draw(frame, boxes);
        Tend = std::chrono::steady_clock::now();

        // calculate frame rate
        f = std::chrono::duration_cast<std::chrono::milliseconds>(Tend - Tbegin).count();
        if (f > 0.0)
            FPS[((Fcnt++) & 0x0F)] = 1000.0 / f;
        for (f = 0.0, i = 0; i < 16; i++)
        {
            f += FPS[i];
        }
        putText(frame, cv::format("FPS %0.2f", f / 16), cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255));

        cv::imshow("Video", frame);
        char esc = cv::waitKey(5);
        if (esc == 27)
            break;
    }

    return 0;
}

int Yolo::detect(const cv::Mat srcImg, std::vector<TargetBox> &dstBoxes, const float thresh)
{
    dstBoxes.clear();

    float scaleW = (float)srcImg.cols / (float)inputWidth;
    float scaleH = (float)srcImg.rows / (float)inputHeight;

    ncnn::Mat inputImg = ncnn::Mat::from_pixels_resize(srcImg.data, ncnn::Mat::PIXEL_BGR,
                                                       srcImg.cols, srcImg.rows, inputWidth, inputHeight);

    const float mean_vals[3] = {0.f, 0.f, 0.f};
    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    inputImg.substract_mean_normalize(mean_vals, norm_vals);

    ncnn::Extractor ex = net.create_extractor();
    ex.set_num_threads(numThreads);

    ex.input("input.1", inputImg);

    ncnn::Mat out[2];
    ex.extract("794", out[0]);
    ex.extract("796", out[1]);

    std::vector<TargetBox> tmpBoxes;
    predHandle(out, tmpBoxes, scaleW, scaleH, thresh);

    nmsHandle(tmpBoxes, dstBoxes);

    return 0;
}

int Yolo::predHandle(const ncnn::Mat *out, std::vector<TargetBox> &dstBoxes,
                     const float scaleW, const float scaleH, const float thresh)
{
    std::vector<float> bias{12.64, 19.39, 37.88, 51.48, 55.71, 138.31,
                            126.91, 78.23, 131.57, 214.55, 279.92, 258.87};
    anchor.assign(bias.begin(), bias.end());
    for (int i = 0; i < 2; i++)
    {
        int stride;
        int outW, outH, outC;

        outH = out[i].c;
        outW = out[i].h;
        outC = out[i].w;

        assert(inputHeight / outH == inputWidth / outW);
        stride = inputHeight / outH;

        for (int h = 0; h < outH; h++)
        {
            const float *values = out[i].channel(h);
            for (int w = 0; w < outW; w++)
            {
                for (int b = 0; b < numAnchor; b++)
                {
                    TargetBox tmpBox;
                    int category = -1;
                    float score = -1;

                    getCategory(values, b, category, score);

                    if (score > thresh)
                    {
                        float bcx, bcy, bw, bh;

                        bcx = ((values[b * 4 + 0] * 2. - 0.5) + w) * stride;
                        bcy = ((values[b * 4 + 1] * 2. - 0.5) + h) * stride;
                        bw = pow((values[b * 4 + 2] * 2.), 2) * anchor[(i * numAnchor * 2) + b * 2 + 0];
                        bh = pow((values[b * 4 + 3] * 2.), 2) * anchor[(i * numAnchor * 2) + b * 2 + 1];

                        tmpBox.x1 = (bcx - 0.5 * bw) * scaleW;
                        tmpBox.y1 = (bcy - 0.5 * bh) * scaleH;
                        tmpBox.x2 = (bcx + 0.5 * bw) * scaleW;
                        tmpBox.y2 = (bcy + 0.5 * bh) * scaleH;
                        tmpBox.score = score;
                        tmpBox.cate = category;

                        dstBoxes.push_back(tmpBox);
                    }
                }
                values += outC;
            }
        }
    }

    return 0;
}

int Yolo::getCategory(const float *values, int index, int &category, float &score)
{
    float tmp = 0;
    float objScore = values[4 * numAnchor + index];

    for (int i = 0; i < numCategory; i++)
    {
        float clsScore = values[4 * numAnchor + numAnchor + i];
        clsScore *= objScore;

        if (clsScore > tmp)
        {
            score = clsScore;
            category = i;
            tmp = clsScore;
        }
    }

    return 0;
}

float Yolo::intersection_area(const TargetBox &a, const TargetBox &b)
{
    if (a.x1 > b.x2 || a.x2 < b.x1 || a.y1 > b.y2 || a.y2 < b.y1)
        return 0.f; // no intersection

    float inter_width = std::min(a.x2, b.x2) - std::max(a.x1, b.x1);
    float inter_height = std::min(a.y2, b.y2) - std::max(a.y1, b.y1);

    return inter_width * inter_height;
}

bool scoreSort(TargetBox a, TargetBox b)
{
    return (a.score > b.score);
}

int Yolo::nmsHandle(std::vector<TargetBox> &tmpBoxes,
                    std::vector<TargetBox> &dstBoxes)
{
    std::vector<int> picked;

    sort(tmpBoxes.begin(), tmpBoxes.end(), scoreSort);

    for (size_t i = 0; i < tmpBoxes.size(); i++)
    {
        int keep = 1;
        for (size_t j = 0; j < picked.size(); j++)
        {
            // 交集
            float inter_area = intersection_area(tmpBoxes[i], tmpBoxes[picked[j]]);
            // 并集
            float union_area = tmpBoxes[i].area() + tmpBoxes[picked[j]].area() - inter_area;
            float IoU = inter_area / union_area;

            if (IoU > nmsThresh && tmpBoxes[i].cate == tmpBoxes[picked[j]].cate)
            {
                keep = 0;
                break;
            }
        }

        if (keep)
        {
            picked.push_back(i);
        }
    }

    for (size_t i = 0; i < picked.size(); i++)
    {
        dstBoxes.push_back(tmpBoxes[picked[i]]);
    }

    return 0;
}

void Yolo::draw(cv::Mat cvImg, const std::vector<TargetBox> &boxes)
{
    for (size_t i = 0; i < boxes.size(); i++)
    {
        char text[256];
        sprintf(text, "%s %.1f%%", class_names[boxes[i].cate + 1], boxes[i].score * 100);

        if ((boxes[i].cate + 1) == 12) // stop sign class
        {
            int baseLine = 0;
            cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

            int x = boxes[i].x1;
            int y = boxes[i].y1 - label_size.height - baseLine;
            if (y < 0)
                y = 0;
            if (x + label_size.width > cvImg.cols)
                x = cvImg.cols - label_size.width;

            cv::rectangle(cvImg, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                          cv::Scalar(255, 255, 255), -1);

            cv::putText(cvImg, text, cv::Point(x, y + label_size.height),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

            cv::rectangle(cvImg, cv::Point(boxes[i].x1, boxes[i].y1),
                          cv::Point(boxes[i].x2, boxes[i].y2), cv::Scalar(255, 0, 0), 2);

            // ssDis_Stop.str(" ");
            // ssDis_Stop.clear();
            // ssDis_Stop << "Distance = " << (boxes[i].x2) - (boxes[i].x1) << " pixels";
            dist_stop = ((-0.75) * ((boxes[i].x2) - (boxes[i].x1))) + (78.75);
            ssDis_Stop.str(" ");
            ssDis_Stop.clear();
            ssDis_Stop << "Distance Stop = " << dist_stop << " cm";
        }
        else if ((boxes[i].cate + 1) == 68)
        {
            int baseLine = 0;
            cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

            int x = boxes[i].x1;
            int y = boxes[i].y1 - label_size.height - baseLine;
            if (y < 0)
                y = 0;
            if (x + label_size.width > cvImg.cols)
                x = cvImg.cols - label_size.width;

            cv::rectangle(cvImg, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                          cv::Scalar(255, 255, 255), -1);

            cv::putText(cvImg, text, cv::Point(x, y + label_size.height),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

            cv::rectangle(cvImg, cv::Point(boxes[i].x1, boxes[i].y1),
                          cv::Point(boxes[i].x2, boxes[i].y2), cv::Scalar(0, 255, 0));
            // ssDis_Car.str(" ");
            // ssDis_Car.clear();
            // ssDis_Car << "Distance = " << (boxes[i].x2) - (boxes[i].x1) << " pixels";

            dist_car = ((-0.75) * ((boxes[i].x2) - (boxes[i].x1))) + (74.5);
            ssDis_Car.str(" ");
            ssDis_Car.clear();
            ssDis_Car << "Distance Car = " << dist_car << " cm";
        }
    }
}

void Yolo::frame(const cv::Mat srcImg)
{
    init(false);
    loadModel();

    std::vector<TargetBox> boxes;
    detect(srcImg, boxes);
    draw(srcImg, boxes);

    cv::namedWindow("Detection", cv::WINDOW_KEEPRATIO);
    cv::moveWindow("Detection", 640, 580);
    cv::resizeWindow("Detection", 640, 480);
    putText(srcImg, ssDis_Stop.str(), cv::Point2f(1, 130), 0, 1, cv::Scalar(0, 0, 255), 2);
    putText(srcImg, ssDis_Car.str(), cv::Point2f(1, 180), 0, 1, cv::Scalar(0, 0, 255), 2);
    cv::imshow("Detection", srcImg);
}