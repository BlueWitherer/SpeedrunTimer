#pragma once

#include <Geode/Geode.hpp>

#include <Geode/utils/terminate.hpp>

using namespace geode::prelude;

class SplitSegment : public CCNode {
protected:
    Mod* m_srtMod = getMod(); // it's modding time :3

    ccColor3B m_colPause = m_srtMod->getSettingValue<ccColor3B>("color-pause"); // The color of the speedrun timer when paused
    ccColor3B m_colStart = m_srtMod->getSettingValue<ccColor3B>("color-start"); // The color of the speedrun timer before starting

    float m_time = 0.f; // This split time
    float m_delta = 0.f; // The delta time for the speedrun timer

    bool init(float time, float delta = 0.f);

public:
    static SplitSegment* create(float time, float delta = 0.f);
};