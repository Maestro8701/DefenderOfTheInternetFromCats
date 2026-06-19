#pragma once
#include <string>

class SoundPlayer {
public:
    static void playScarySound(int volumeLevel);
    static void stopSound();

    static int getRandomNumber(int min, int max);
private:
    static std::string getRandomSoundFile();
};
