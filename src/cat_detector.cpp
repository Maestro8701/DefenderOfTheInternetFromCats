#include "cat_detector.h"
#include <iostream>

CatDetector::CatDetector(const std::string& configPath, const std::string& weightsPath) {
    net = cv::dnn::readNetFromDarknet(configPath, weightsPath);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

bool CatDetector::detectCat(const cv::Mat& frame) {
    cv::Mat blob = cv::dnn::blobFromImage(frame, 1.0 / 255.0, cv::Size(416, 416), cv::Scalar(0, 0, 0), true, false);
    net.setInput(blob);
    std::vector<cv::Mat> outs;
    net.forward(outs, net.getUnconnectedOutLayersNames());

    bool catInFrame = false;
    for (const auto& out : outs) {
        const float* data = (float*)out.data;
        for (int i = 0; i < out.rows; ++i, data += out.cols) {
            cv::Mat scores = out.row(i).colRange(5, out.cols);
            cv::Point classIdPoint;
            double confidence;
            cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);

            if (confidence > CONFIDENCE_THRESHOLD && classIdPoint.x == 15) { // classId 15 = cat
                int width = static_cast<int>(data[2] * frame.cols);
                currentDistance = (REAL_CAT_WIDTH * FOCAL_LENGTH) / width;

                int centerX = static_cast<int>(data[0] * frame.cols);
                int centerY = static_cast<int>(data[1] * frame.rows);
                int height = static_cast<int>(data[3] * frame.rows);

                cv::Rect catRect(centerX - width / 2, centerY - height / 2, width, height);
                cv::rectangle(frame, catRect, cv::Scalar(0, 255, 0), 2);

                if (currentDistance <= TRIGGER_DIST) {
                    catInFrame = true;
                    catFrameCounter++;
                    if (catFrameCounter >= FRAME_CONFIRM_COUNT) {
                        return true;
                    }
                }
            }
        }
    }
    catFrameCounter = 0;
    return false;
}
