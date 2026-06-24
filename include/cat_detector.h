#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

class CatDetector {
public:
    CatDetector(const std::string& configPath, const std::string& weightsPath);
    bool detectCat(const cv::Mat& frame);
    float getCatDistance();

private:
    cv::dnn::Net net;
    const float REAL_CAT_WIDTH = 12.0f;
    const float FOCAL_LENGTH = 600.0f;
    const float TRIGGER_DIST = 500.0f;
    const float CONFIDENCE_THRESHOLD = 0.85f;
    const int FRAME_CONFIRM_COUNT = 5;
    int catFrameCounter = 0;
    float currentDistance = 9999.0f;
};

