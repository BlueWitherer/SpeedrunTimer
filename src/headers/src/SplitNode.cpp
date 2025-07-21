#include "../SplitNode.hpp"

#include <Geode/Geode.hpp>

#include <Geode/cocos/CCScheduler.h>

using namespace geode::prelude;

bool SplitNode::init() {
    if (CCNode::init()) {
        return true;
    } else {
        return false;
    };
};

SplitNode* SplitNode::create() {
    SplitNode* ret = new SplitNode();

    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};