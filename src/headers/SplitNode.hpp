#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class SplitNode : public CCNode {
protected:
    float m_splitTime = 0.f; // This split time

    bool init();

public:
    static SplitNode* create();
};