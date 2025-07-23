#include "../SplitSegment.hpp"

#include <fmt/core.h>

#include <Geode/Geode.hpp>

#include <Geode/cocos/CCScheduler.h>

using namespace geode::prelude;

bool SplitSegment::init(float time, float delta) {
    m_splitTime = time;
    m_delta = delta;

    if (CCNode::init()) {
        setID("split"_spr);
        setContentSize({ 125.f, 12.5f });
        setAnchorPoint({ 0, 1 });

        auto ms = as<int>((m_splitTime - as<int>(m_splitTime)) * 100);
        auto splitStr = fmt::format("{}.{:02d}", as<int>(m_splitTime), ms);

        auto splitLabel = CCLabelBMFont::create(
            splitStr.c_str(),
            "gjFont17.fnt"
        );
        splitLabel->setID("split-label");
        splitLabel->setColor(m_colPause);
        splitLabel->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
        splitLabel->setPosition({ getContentSize().width - 5, getContentSize().height / 2 });
        splitLabel->setAnchorPoint({ 1, 0.5 });
        splitLabel->setScale(0.25f);
        splitLabel->setZOrder(2);

        addChild(splitLabel);

        if (m_delta == m_splitTime) {
            log::warn("Skipping first delta label");
        } else {
            auto deltaStr = fmt::format("+{:.2f}", m_delta);

            auto deltaLabel = CCLabelBMFont::create(
                deltaStr.c_str(),
                "gjFont17.fnt"
            );
            deltaLabel->setID("delta-label");
            deltaLabel->setColor(m_colStart);
            deltaLabel->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
            deltaLabel->setPosition({ splitLabel->getPositionX() - splitLabel->getScaledContentWidth() - 5.f, getContentSize().height / 2 });
            deltaLabel->setAnchorPoint({ 1, 0.5 });
            deltaLabel->setScale(0.2f);
            deltaLabel->setZOrder(2);

            addChild(deltaLabel);
        };

        auto bgOpacity = as<int>(m_srtMod->getSettingValue<int64_t>("bg-opacity"));

        auto bg = CCLayerColor::create({ 0, 0, 0, 255 });
        bg->setID("background");
        bg->setOpacity(bgOpacity);
        bg->setScaledContentSize(getScaledContentSize());
        bg->setZOrder(1);

        addChild(bg);

        return true;
    } else {
        return false;
    };
};

SplitSegment* SplitSegment::create(float time, float delta) {
    SplitSegment* ret = new SplitSegment();

    if (ret && ret->init(time, delta)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};