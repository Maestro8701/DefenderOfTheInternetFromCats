#include "alarm_system.h"
#include "sound_player.h"
#include <iostream>

bool AlarmSystem::alarmActive = false;
int64 AlarmSystem::lastSoundTime = 0;

void AlarmSystem::handleAlarm(bool catDetected, UDPCommunicator& udp) {
    if (catDetected) {
        if (!alarmActive) {
            std::cout << "CAT CONFIRMED! ALARM!" << std::endl;
            alarmActive = true;
            lastSoundTime = utils::getCurrentTimeMs();
            SoundPlayer::playScarySound(0);
            udp.sendMessage("ALARM_ON");
        }
        else {
            int64 now = utils::getCurrentTimeMs();
            if (now - lastSoundTime > soundInterval) {
                SoundPlayer::playScarySound(1); // Увеличиваем громкость
                lastSoundTime = now;
            }
        }
    }
    else {
        if (alarmActive) {
            SoundPlayer::stopSound();
            alarmActive = false;
            udp.sendMessage("ALARM_OFF");
        }
    }
}
