#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class SplitSegment : public CCNode {
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

protected:
    SplitSegment();
    ~SplitSegment();

    bool init(float time, float delta);

public:
    static SplitSegment* create(float time, float delta = 0.f);
};