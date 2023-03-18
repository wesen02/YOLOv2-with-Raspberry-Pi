#include <yolo.hpp>
#include <capture.hpp>
#include <emergency_stop.hpp>
#include <manoeuvre.hpp>

void cameraSetup(int argc, char **argv, raspicam::RaspiCam_Cv &camera)
{
    camera.set(cv::CAP_PROP_FRAME_WIDTH, ("-w", argc, argv, 400));
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, ("-h", argc, argv, 240));
    camera.set(cv::CAP_PROP_BRIGHTNESS, ("-br", argc, argv, 50));
    camera.set(cv::CAP_PROP_CONTRAST, ("-co", argc, argv, 50));
    camera.set(cv::CAP_PROP_SATURATION, ("-sa", argc, argv, 50));
    camera.set(cv::CAP_PROP_GAIN, ("-g", argc, argv, 50));
    camera.set(cv::CAP_PROP_FPS, ("-fps", argc, argv, 0));
}

int main(int argc, char **argv)
{
    Yolo yolo;
    e_stop e_stop;
    moving moving;
    capture capture;

    std::cout << "Connecting to camera"
              << "\n";
    cameraSetup(argc, argv, capture.camera);
    if (!capture.camera.open())
    {
        std::cerr << "Error Opening Camera"
                  << "\n";
        return 0;
    }
    std::cout << "Camera ID = " << capture.camera.getId() << "\n";

    // INIT
    moving.wiringPi();
    e_stop.initSensor();

    // FPS
    clock_t start;
    clock_t end;
    double ms, fpsLive, seconds;

    while (true)
    {
        start = clock();

        double distance = e_stop.detectDistance();

        std::cout << "Distance: " << distance << " cm"
                  << "\n";

        capture.camera.grab();
        capture.camera.retrieve(capture.original);

        capture.frame = capture.original.clone();
        capture.detection = capture.original.clone();

        // cv::resize(capture.frame, capture.frame, cv::Size(400, 240), cv::INTER_LINEAR);
        // cv::resize(capture.detection, capture.detection, cv::Size(400, 240), cv::INTER_LINEAR);

        capture.perspectiveTrans();
        capture.Threshold();
        capture.Histogram();
        capture.laneFinder();
        capture.laneCenter();

        if (distance < 14)
        {
            moving.stop();
            goto stopSign;
        }
        else
        {
            // if (capture.laneEnd > 700000)
            // {
            //     moving.noLane();
            //     // moving.stop();
            // }
            if (yolo.dist_stop > 5 && yolo.dist_stop < 20)
            {
                moving.stop();
                yolo.dist_stop = 0;
                goto stopSign;
            }
            std::cout << "No stop"
                      << "\n";
            moving.forward(capture.Result);
        }
    stopSign:
        capture.ss.str(" ");
        capture.ss.clear();
        capture.ss << "Result = " << capture.Result;
        putText(capture.frame, capture.ss.str(), cv::Point2f(1, 50), 0, 1, cv::Scalar(0, 0, 255), 2);

        cv::namedWindow("Original", cv::WINDOW_KEEPRATIO);
        cv::moveWindow("Original", 0, 100);
        cv::resizeWindow("Original", 640, 480);
        cv::imshow("Original", capture.original);

        cv::namedWindow("Point View", cv::WINDOW_KEEPRATIO);
        cv::moveWindow("Point View", 640, 100);
        cv::resizeWindow("Point View", 640, 480);
        cv::imshow("Point View", capture.frame);

        cv::namedWindow("Perspective View", cv::WINDOW_KEEPRATIO);
        cv::moveWindow("Perspective View", 1280, 100);
        cv::resizeWindow("Perspective View", 640, 480);
        cv::imshow("Perspective View", capture.framePerspective);

        cv::namedWindow("Lane Finder", cv::WINDOW_KEEPRATIO);
        cv::moveWindow("Lane Finder", 0, 580);
        cv::resizeWindow("Lane Finder", 640, 480);
        cv::imshow("Lane Finder", capture.frameFinal);

        yolo.frame(capture.detection);
        end = clock();
        seconds = (double(end) - double(start)) / double(CLOCKS_PER_SEC);
        int(fpsLive) = 1.0 / double(seconds);
        std::cout << "FPS: " << fpsLive << "\n";

        char key = cv::waitKey(1);
        if (key == 'q')
        {
            moving.stop();
            break;
        }
    }

    return 0;
}