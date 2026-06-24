#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp> 

#include "cat_detector.h"
#include "alarm_system.h"
#include "udp_communicator.h"
#include "utils.h"

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h> 

#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace cv;
using namespace std;
using namespace cv::dnn;

const int UDP_PORT = 12345;

int main() {
    try {
        // Инициализация UDP
        UDPCommunicator udp(UDP_PORT);

        // Загрузка YOLO-модели
        CatDetector detector("models/yolov4-tiny.cfg", "models/yolov4-tiny.weights");

        // Открытие видео
        VideoCapture cap("test_cat1.mp4");
        if (!cap.isOpened()) {
            std::cerr << "Failed to open video!" << std::endl;
            return -1;
        }

        // Основной цикл
        while (true) {
            Mat frame;
            if (!cap.read(frame)) break;

            // Обнаружение кота
            bool catDetected = detector.detectCat(frame);

            // Логика тревоги
            AlarmSystem::handleAlarm(catDetected, udp);

            // Обработка UDP-сообщений
            udp.checkForCommands();

            // Отображение кадра
            imshow("YOLO Cat Detector", frame);
            if (waitKey(1) == 27) break;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
