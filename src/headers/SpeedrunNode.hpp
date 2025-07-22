#pragma once

#include <Geode/Geode.hpp>

#include <Geode/utils/terminate.hpp>

using namespace geode::prelude;

class SpeedrunNode : public CCNode {
protected:
    Mod* m_srtMod = getMod(); // it's modding time :3

    CCScheduler* m_scheduler = nullptr; // The scheduler for the speedrun timer

    float m_speedTime = 0.f; // Current time

    bool m_speedtimerOn = false; // If the speedrun is active
    bool m_speedtimerPaused = true; // If the speedrun is paused

    CCMenu* m_timeMenu = nullptr; // The menu that contains the timers

    CCLabelBMFont* m_speedtimer = nullptr; // The text label for the time
    CCLabelBMFont* m_speedtimerMs = nullptr; // The text label for the time in milliseconds

    ScrollLayer* m_splitList = nullptr; // The scrolling list of timer splits

    void update(float dt);

    bool init();

public:
    // Toggle the timer, must be done manually at first
    void toggleTimer(bool toggle = false);

    // Pause the timer
    void pauseTimer(bool pause = true);

    // Create a split
    void createSplit();

    // Reset everything
    void resetAll();

    static SpeedrunNode* create();
};