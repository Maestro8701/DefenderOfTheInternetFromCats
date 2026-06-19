#pragma once
#include <opencv2/opencv.hpp>
#include "udp_communicator.h"
#include "utils.h"//Добавил

class AlarmSystem {
public:
    static void handleAlarm(bool catDetected, UDPCommunicator& udp);
private:
    static bool alarmActive;
    static int64 lastSoundTime;
    static const int64 soundInterval = 6000; // 6 секунд
};
