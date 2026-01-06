#include "../SplitSegment.hpp"

#include <fmt/core.h>

#include <Geode/Geode.hpp>

#include <Geode/utils/terminate.hpp>

#include <Geode/cocos/CCScheduler.h>

using namespace geode::prelude;

class SplitSegment::Impl final {
public:
    Mod* m_srtMod = getMod(); // it's modding time :3

    ccColor3B m_colPause = m_srtMod->getSettingValue<ccColor3B>("color-pause"); // The color of the speedrun timer when paused
    ccColor3B m_colStart = m_srtMod->getSettingValue<ccColor3B>("color-start"); // The color of the speedrun timer before starting

    float m_time = 0.f; // This split time
    float m_delta = 0.f; // The delta time for the speedrun timer
};

SplitSegment::SplitSegment() {
    m_impl = std::make_unique<Impl>();
};

SplitSegment::~SplitSegment() {};

bool SplitSegment::init(float time, float delta) {
    m_impl->m_time = time;
    m_impl->m_delta = delta;

    if (!CCNode::init()) return false;
    setID("split"_spr);
    setContentSize({ 125.f, 12.5f });
    setAnchorPoint({ 0, 1 });

    auto ms = static_cast<int>((m_impl->m_time - static_cast<int>(m_impl->m_time)) * 100);
    auto splitStr = fmt::format("{}.{:02d}", static_cast<int>(m_impl->m_time), ms);

    auto splitLabel = CCLabelBMFont::create(
        splitStr.c_str(),
        "gjFont17.fnt"
    );
    splitLabel->setID("split-label");
    splitLabel->setColor(m_impl->m_colPause);
    splitLabel->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
    splitLabel->setPosition({ getContentSize().width - 5, getContentSize().height / 2 });
    splitLabel->setAnchorPoint({ 1, 0.5 });
    splitLabel->setScale(0.25f);
    splitLabel->setZOrder(2);

    addChild(splitLabel);

    if (m_impl->m_delta == m_impl->m_time) {
        log::warn("Skipping first delta label");
    } else {
        auto deltaStr = fmt::format("+{:.2f}", m_impl->m_delta);

        auto deltaLabel = CCLabelBMFont::create(
            deltaStr.c_str(),
            "gjFont17.fnt"
        );
        deltaLabel->setID("delta-label");
        deltaLabel->setColor(m_impl->m_colStart);
        deltaLabel->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
        deltaLabel->setPosition({ splitLabel->getPositionX() - splitLabel->getScaledContentWidth() - 5.f, getContentSize().height / 2 });
        deltaLabel->setAnchorPoint({ 1, 0.5 });
        deltaLabel->setScale(0.2f);
        deltaLabel->setZOrder(2);

        addChild(deltaLabel);
    };

    auto bgOpacity = static_cast<int>(m_impl->m_srtMod->getSettingValue<int64_t>("bg-opacity"));

    auto bg = CCLayerColor::create({ 0, 0, 0, 255 });
    bg->setID("background");
    bg->setOpacity(bgOpacity);
    bg->setScaledContentSize(getScaledContentSize());
    bg->setZOrder(1);

    addChild(bg);

    return true;
};

SplitSegment* SplitSegment::create(float time, float delta) {
    auto ret = new SplitSegment();
    if (ret->init(time, delta)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};