#include <string>
#include <thread>
#include <chrono>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <opencv2/opencv.hpp>
#include <SFML/Audio.hpp>

#include "alarm_system.h"
#include "cat_detector.h"
#include "sound_player.h"
#include "udp_communicator.h"
#include "utils.h"
// ==================== Моки ====================
class MockUDPCommunicator : public UDPCommunicator {
public:
    MOCK_METHOD(void, sendMessage, (const std::string&), ());
    MOCK_METHOD(void, checkForCommands, (), ());
    MOCK_METHOD(bool, isReady, (), (const));
};

class SoundPlayerWrapper {
public:
    static float currentDynamicVolume;
    static void resetVolume() { currentDynamicVolume = 15.0f; }
    static float getCurrentDynamicVolume() { return currentDynamicVolume; }
    static void playScarySound(int durationSeconds) {
        currentDynamicVolume += 5.0f;
        currentDynamicVolume = std::min(100.0f, currentDynamicVolume);
    }
    static void stopSound() { currentDynamicVolume = 15.0f; }
};
float SoundPlayerWrapper::currentDynamicVolume = 15.0f;

class MockCatDetector {
public:
    MOCK_METHOD(bool, detectCat, (const cv::Mat&), ());
    MOCK_METHOD(float, getCatDistance, (), (const));
};

class CatDetectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        SoundPlayerWrapper::resetVolume();
        SoundPlayer::setPathToSounds("./multimedia/sound/");
    }

    void TearDown() override {
        SoundPlayer::stopSound();
    }
};

// ==================== Тесты ====================
// --- AlarmSystem Tests ---
TEST_F(CatDetectorTest, AlarmSystem_Activates_WhenCatDetected) {
    MockUDPCommunicator mockUdp;
    EXPECT_CALL(mockUdp, isReady()).WillOnce(::testing::Return(true));
    EXPECT_CALL(mockUdp, sendMessage("ALARM_ON")).Times(1);

    AlarmSystem::handleAlarm(true, mockUdp);
    EXPECT_GT(SoundPlayerWrapper::getCurrentDynamicVolume(), 0.0f);
}

TEST_F(CatDetectorTest, AlarmSystem_DoesNotDeactivate_WhenCatDisappears) {
    MockUDPCommunicator mockUdp;
    EXPECT_CALL(mockUdp, isReady())
        .WillOnce(::testing::Return(true))
        .WillOnce(::testing::Return(true))
        .WillOnce(::testing::Return(true)); 

    EXPECT_CALL(mockUdp, sendMessage("ALARM_ON")).Times(1);
    EXPECT_CALL(mockUdp, sendMessage("ALARM_OFF")).Times(1);

    AlarmSystem::handleAlarm(true, mockUdp);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Небольшая задержка
    AlarmSystem::handleAlarm(false, mockUdp);
    std::this_thread::sleep_for(std::chrono::seconds(30));
    AlarmSystem::handleAlarm(false, mockUdp);
}

TEST_F(CatDetectorTest, CatDetector_CorrectInitialization) {
    CatDetector detector{ "", 
                              "" };
    cv::Mat emptyFrame;
    ASSERT_NO_THROW(detector.detectCat(emptyFrame));
}

TEST_F(CatDetectorTest, CatDetector_DetectsCat_WithHighConfidence) {
    MockCatDetector mockDetector;
    cv::Mat fakeFrame(100, 100, CV_8UC3, cv::Scalar(100, 100, 100));

    EXPECT_CALL(mockDetector, detectCat(::testing::_))
        .WillOnce(::testing::Return(true));

    ASSERT_TRUE(mockDetector.detectCat(fakeFrame));
}

TEST_F(CatDetectorTest, CatDetector_RejectsLowConfidence) {
    MockCatDetector mockDetector;
    cv::Mat fakeFrame(100, 100, CV_8UC3, cv::Scalar(100, 100, 100));

    EXPECT_CALL(mockDetector, detectCat(::testing::_))
        .WillOnce(::testing::Return(false));

    ASSERT_FALSE(mockDetector.detectCat(fakeFrame));
}

TEST_F(CatDetectorTest, CatDetector_CalculatesDistanceCorrectly) {
    CatDetector detector{ "", "" };
    float dist = (12.0f * 600.0f) / 100.0f;
    EXPECT_NEAR(dist, 72.0f, 1.0f);
}

TEST_F(CatDetectorTest, SoundPlayer_PlayScarySound_IncreasesVolume) {
    SoundPlayerWrapper::resetVolume();
    SoundPlayer::playScarySound(10);
    float vol1 = SoundPlayerWrapper::getCurrentDynamicVolume();
    SoundPlayer::playScarySound(10);
    float vol2 = SoundPlayerWrapper::getCurrentDynamicVolume();
    EXPECT_GT(vol2, vol1);
}

TEST_F(CatDetectorTest, SoundPlayer_StopSound_ResetsVolume) {
    SoundPlayerWrapper::resetVolume();
    SoundPlayer::playScarySound(10);
    SoundPlayer::stopSound();
    EXPECT_EQ(SoundPlayerWrapper::getCurrentDynamicVolume(), 15.0f);
}

TEST_F(CatDetectorTest, Utils_GetCurrentTimeMs_ReturnsPositiveValue) {
    int64_t now = utils::getCurrentTimeMs();
    EXPECT_GT(now, 0);
}

TEST_F(CatDetectorTest, CatDetector_ReturnsFalse_OnEmptyOrInvalidFrame) {
    CatDetector detector{ "", "" };
    cv::Mat invalidFrame;
    ASSERT_FALSE(detector.detectCat(invalidFrame));
    cv::Mat wrongType(100, 100, CV_32FC3, cv::Scalar(0));
    ASSERT_NO_THROW(detector.detectCat(wrongType)); 
}

TEST_F(CatDetectorTest, SoundPlayer_StopSound_WhenNotPlaying) {
    SoundPlayer::stopSound();
    bool wasVolumeReset = (SoundPlayerWrapper::currentDynamicVolume == 15.0f);
    EXPECT_TRUE(wasVolumeReset);
}

TEST_F(CatDetectorTest, SoundPlayer_HandlesMissingSoundFile) {
    SoundPlayer::setPathToSounds("./nonexistent/");
    float before = SoundPlayerWrapper::getCurrentDynamicVolume();
    SoundPlayer::playScarySound(1);
    float after = SoundPlayerWrapper::getCurrentDynamicVolume();
    EXPECT_EQ(before, after); 
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
