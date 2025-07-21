#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class SpeedrunNode : public CCNode {
protected:
    CCScheduler* m_scheduler = nullptr; // The scheduler for the speedrun timer

    float m_speedTime = 0.f; // Current time
    bool m_speedtimerOn = false; // If the speedrun is active

    CCLabelBMFont* m_speedtimer = nullptr; // The text label for the time
    CCLabelBMFont* m_speedtimerMs = nullptr; // The text label for the time in milliseconds

    ScrollLayer* m_splitList = nullptr;

    void update(float dt);

    bool init();

public:
    // Activate or deactivate the timer
    bool toggle(bool on = false);

    static SpeedrunNode* create();
};