#include "../SplitNode.hpp"

#include <Geode/Geode.hpp>

#include <Geode/utils/terminate.hpp>

#include <Geode/cocos/CCScheduler.h>

using namespace geode::prelude;

bool SplitNode::init(float time) {
    if (CCNode::init()) {
        m_srtMod = getMod();
        m_splitTime = time;

        if (!m_srtMod) {
            log::error("Couldn't load self mod");
            return false;
        };

        setID("split"_spr);
        setContentSize({ 125.f, 2.5f });
        setAnchorPoint({ 0, 1 });

        auto splitLabel = CCLabelBMFont::create(
            std::to_string(as<int>(m_splitTime)).c_str(),
            "gjFont17.fnt"
        );
        splitLabel->setColor(m_srtMod->getSettingValue<ccColor3B>("color"));
        splitLabel->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
        splitLabel->setPosition({ getContentSize().width - 5, getContentSize().height / 2 });
        splitLabel->setAnchorPoint({ 1, 0 });
        splitLabel->setScale(0.875f);

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