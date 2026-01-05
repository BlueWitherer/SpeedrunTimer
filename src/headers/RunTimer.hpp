#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class RunTimer : public CCNode {
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

protected:
    RunTimer();
    virtual ~RunTimer();

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

    // Check if timer is paused
    bool isTimerPaused();

    static RunTimer* create();
};