#pragma once

#include <Geode/Geode.hpp>

class SplitSegment final : public cocos2d::CCNode {
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