#include "sound_player.h"
#include <windows.h>
#include <mmsystem.h>
#include <random>
#include <iostream>
#include <filesystem>
#include <algorithm>  // Для std::min и std::to_string

void SoundPlayer::playScarySound(int volumeLevel) {
    std::string soundName = "res\\sound" + std::to_string(getRandomNumber(1, 5)) + ".mp3";
    char fullPath[MAX_PATH];

    // Получаем полный путь и проверяем наличие файла
    DWORD attrs = GetFileAttributesA(fullPath);
    if (!GetFullPathNameA(soundName.c_str(), MAX_PATH, fullPath, nullptr) ||
        attrs == INVALID_FILE_ATTRIBUTES) {
        std::cerr << "!!! Sound file missing: " << soundName << std::endl;
        return;
    }

    // Расчет громкости с лимитом на максимум 1.0
    float vol = 0.4f + (volumeLevel * 0.15f);
    std::cout << "Playing: " << fullPath << " (Vol: " << vol << ")" << std::endl;

    // Формирование команды для PowerShell
    std::string cmd = R"(powershell -c "$p=New-Object System.Windows.Media.MediaPlayer;
                          $p.Open(')" + std::string(fullPath) + R"(');
                          $p.Volume=" + std::to_string(vol) + R";
                          $p.Play(); Start-Sleep -s 5)";

    WinExec(cmd.c_str(), SW_HIDE);
}

void SoundPlayer::stopSound() {
    mciSendStringA("stop mp3_sound", nullptr, 0, nullptr);
}

int SoundPlayer::getRandomNumber(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

std::string SoundPlayer::getRandomSoundFile() {
    return "res\\sound" + std::to_string(getRandomNumber(1, 5)) + ".mp3";
}
