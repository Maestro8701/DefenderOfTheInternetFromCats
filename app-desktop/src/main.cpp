#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include "cat_detector.h"
#include "alarm_system.h"
#ifdef _WIN32
#include "udp_communicator_win.h"
#else
#include "udp_communicator_unix.h"
#endif

#include "MainWindow.h"
#include <QtWidgets/QApplication>

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp> 

#include <toml.hpp>

using namespace cv;
using namespace std;
using namespace cv::dnn;
int main(int argc, char* argv[]) {
    std::atomic<bool> keepRunning(true);
    string windowName = "YOLO Cat Detector";

    try {
        std::thread qtThread([argc, argv, &keepRunning]() {
            int mockArgc = argc;
            QApplication app(mockArgc, argv);
            MainWindow window;
            window.show();

            app.exec();

            keepRunning = false;
            });

        UDPCommunicator udp;
        CatDetector detector("models/yolov4-tiny.cfg",
            "models/yolov4-tiny.weights");
        VideoCapture cap(0);//"multimedia/video/test_cat.mp4"   Или если с камерыу   0

        if (!cap.isOpened()) {
            std::cout << "Video stream not detected" << std::endl;
            return -1;
        }



        std::thread udpThread([&udp, &keepRunning]() {
            try {
                while (keepRunning) {
                    udp.checkForCommands();
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            catch (const std::exception& e) {
                std::cerr << "[UDP Thread Error] " << e.what() << std::endl;
            }
            });

        namedWindow(windowName);
        while (keepRunning) {
            Mat frame;
            if (!cap.read(frame)) break;

            bool catDetected = detector.detectCat(frame);

            AlarmSystem::handleAlarm(catDetected, udp);
            imshow("YOLO Cat Detector", frame);

            if (getWindowProperty(windowName, WND_PROP_VISIBLE) < 1) {
                keepRunning = false;
                break;
            }

            if (waitKey(1) == 27) break;

        }

        destroyAllWindows();

        if (udpThread.joinable()) {
            udpThread.join();
        }

        if (qtThread.joinable()) {
            QMetaObject::invokeMethod(QApplication::instance(), "quit", Qt::QueuedConnection);
            qtThread.join();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        keepRunning = false;
        return -1;

    }



    return 0;

}