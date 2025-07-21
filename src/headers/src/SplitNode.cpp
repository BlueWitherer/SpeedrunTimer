#include "../SplitNode.hpp"

#include <Geode/Geode.hpp>

#include <Geode/cocos/CCScheduler.h>

using namespace geode::prelude;

bool SplitNode::init(float time) {
    if (CCNode::init()) {
        m_splitTime = time;

        return true;
    } else {
        return false;
    };
};

SplitNode* SplitNode::create(float time) {
    SplitNode* ret = new SplitNode();

    if (ret && ret->init(time)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};