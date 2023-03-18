cmake_minimum_required(VERSION 3.16)

project(autopilot_ncnn)

set(
    ncnn_DIR "/home/pi/ncnn/build/install/lib/cmake/ncnn" 
    CACHE PATH "/home/pi/ncnn/build/install/lib/cmake/ncnn"
)
set(raspicam_DIR "/usr/local/include/raspicam")

find_package(OpenCV REQUIRED)
find_package(ncnn REQUIRED)

include_directories(
    include
    ${OpenCV_INCLUDE_DIRS}
    ${raspicam_DIR}
)


add_executable(opencv
    src/main.cpp
    src/yolo.cpp
    src/capture.cpp
)

target_link_libraries(opencv
    ${OpenCV_LIBS}
    ncnn
    wiringPi
    raspicam_cv
)
