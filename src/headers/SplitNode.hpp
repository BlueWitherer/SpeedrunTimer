#pragma once

#include <Geode/Geode.hpp>

#include <Geode/utils/terminate.hpp>

using namespace geode::prelude;

class SplitNode : public CCNode {
protected:
    Mod* m_srtMod = getMod(); // it's modding time :3

    float m_splitTime = 0.f; // This split time

    bool init(float time);

public:
    static SplitNode* create(float time);
};