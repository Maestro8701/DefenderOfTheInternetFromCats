#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp> 
#include <iostream>
#include <vector>
#include <fstream>
#include <random>

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

int alarmDurationCounter = 0; 
const int MAX_VOLUME_STEPS = 10; 
const float REAL_CAT_WIDTH = 12.0f;
const float FOCAL_LENGTH = 600.0f;
const float TRIGGER_DIST = 500.0f;
const int UDP_PORT = 12345;
const float CONFIDENCE_THRESHOLD = 0.85f; 
const int FRAME_CONFIRM_COUNT = 5;       

void playScarySound(int volumeLevel) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 5); 

    string soundName = "res\\sound" + to_string(dis(gen)) + ".mp3";
    char fullPath[MAX_PATH];
    GetFullPathNameA(soundName.c_str(), MAX_PATH, fullPath, NULL);

    if (GetFileAttributesA(fullPath) == INVALID_FILE_ATTRIBUTES) {
        cout << "!!! Файл отсутствует: " << fullPath << endl;
        return;
    }

    float vol = 0.4f + (volumeLevel * 0.15f);
    if (vol > 1.0f) vol = 1.0f;

    cout << "Playing: " << soundName << " (Vol: " << vol << ")" << endl;

    string cmd = "powershell -c \"Add-Type -AssemblyName PresentationCore; "
        "$p = New-Object System.Windows.Media.MediaPlayer; "
        "$p.Open('" + string(fullPath) + "'); "
        "$p.Volume = " + to_string(vol) + "; "
        "$p.Play(); Start-Sleep -s 5\"";

    WinExec(cmd.c_str(), SW_HIDE);
}


int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return -1;
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    u_long mode = 1;
    ioctlsocket(udpSocket, FIONBIO, &mode);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(UDP_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bind(udpSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

    sockaddr_in phoneAddr;
    phoneAddr.sin_family = AF_INET;
    phoneAddr.sin_port = htons(UDP_PORT);
    inet_pton(AF_INET, "192.168.1.100", &phoneAddr.sin_addr);

    Net net = readNetFromDarknet("models/yolov4-tiny.cfg", "models/yolov4-tiny.weights");
    net.setPreferableBackend(DNN_BACKEND_OPENCV);
    net.setPreferableTarget(DNN_TARGET_CPU);


    VideoCapture cap("test_cat.mp4");
    //VideoCapture cap(0);
    if (!cap.isOpened()) return -1;

    Mat frame;
    bool alarmActive = false;
    int catStrikeCounter = 0; 

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        Mat blob = blobFromImage(frame, 1 / 255.0, Size(416, 416), Scalar(0, 0, 0), true, false);
        net.setInput(blob);

        vector<Mat> outs;
        net.forward(outs, net.getUnconnectedOutLayersNames());

        bool catInThisFrame = false; // ОСТАВИЛИ ОДНУ
        float currentDistance = 9999.0f;

        for (const auto& out : outs) {
            float* data = (float*)out.data;
            for (int i = 0; i < out.rows; ++i, data += out.cols) {
                Mat scores = out.row(i).colRange(5, out.cols);
                Point classIdPoint;
                double confidence;
                minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);

                // classId 15 — это кот в YOLO
                if (confidence > CONFIDENCE_THRESHOLD && classIdPoint.x == 15) {
                    int width = (int)(data[2] * frame.cols);
                    currentDistance = (REAL_CAT_WIDTH * FOCAL_LENGTH) / width;

                    int centerX = (int)(data[0] * frame.cols);
                    int centerY = (int)(data[1] * frame.rows);
                    int height = (int)(data[3] * frame.rows);

                    Rect catRect(centerX - width / 2, centerY - height / 2, width, height);
                    rectangle(frame, catRect, Scalar(0, 255, 0), 2);

                    if (currentDistance <= TRIGGER_DIST) {
                        catInThisFrame = true;
                    }
                }
            }
        }

        int64 lastSoundTime = 0;
        int64 soundInterval = 6000; 

        if (catInThisFrame) {
            catStrikeCounter++;
            if (catStrikeCounter >= FRAME_CONFIRM_COUNT) {
                int64 currentTime = getTickCount() * 1000 / getTickFrequency(); // текущее время в мс

                if (!alarmActive) {
                    cout << "CAT CONFIRMED! ALARM!" << endl;
                    alarmActive = true;
                    alarmDurationCounter = 0;
                    playScarySound(alarmDurationCounter);
                    lastSoundTime = currentTime;

                    string msg = "ALARM_ON";
                    sendto(udpSocket, msg.c_str(), (int)msg.length(), 0, (sockaddr*)&phoneAddr, sizeof(phoneAddr));
                }
                else {
                    if (currentTime - lastSoundTime > soundInterval) {
                        alarmDurationCounter++;
                        playScarySound(alarmDurationCounter);
                        lastSoundTime = currentTime;
                    }
                }
            }
        }
        else {
            if (alarmActive) {
                mciSendStringA("stop mp3_sound", NULL, 0, NULL);
                alarmActive = false;
            
            catStrikeCounter = 0;
            alarmDurationCounter = 0;
        }

        char buffer[512];
        int addrLen = sizeof(phoneAddr);
        int resp = recvfrom(udpSocket, buffer, 511, 0, (sockaddr*)&phoneAddr, &addrLen);
        if (resp > 0) {
            buffer[resp] = '\0';
            if (string(buffer) == "ALARM_OFF") {
                cout << "Remote Reset!" << endl;
                alarmActive = false;
                catStrikeCounter = 0;
                mciSendStringA("stop mp3_sound", NULL, 0, NULL);
            }
        }

        imshow("YOLO Cat Detector", frame);
        if (waitKey(1) == 27) break;
    }

    closesocket(udpSocket);
    WSACleanup();
    return 0;
}
