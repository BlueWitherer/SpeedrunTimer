#include "../SplitNode.hpp"

#include <fmt/core.h>

#include <Geode/Geode.hpp>

#include <Geode/cocos/CCScheduler.h>

using namespace geode::prelude;

bool SplitNode::init(float time) {
    m_splitTime = time;

    if (CCNode::init()) {
        setID("split"_spr);
        setContentSize({ 125.f, 12.5f });
        setAnchorPoint({ 0, 1 });

        auto ms = as<int>((m_splitTime - as<int>(m_splitTime)) * 100);
        auto splitStr = fmt::format("{}.{:02d}s", as<int>(m_splitTime), ms);

        auto splitLabel = CCLabelBMFont::create(
            splitStr.c_str(),
            "gjFont17.fnt"
        );
        splitLabel->setColor(m_srtMod->getSettingValue<ccColor3B>("color"));
        splitLabel->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
        splitLabel->setPosition({ getContentSize().width - 5, getContentSize().height / 2 });
        splitLabel->setAnchorPoint({ 1, 0.5 });
        splitLabel->setScale(0.375f);

        addChild(splitLabel);

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