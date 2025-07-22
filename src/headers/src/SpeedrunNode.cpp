#include "../SpeedrunNode.hpp"

#include "../SplitNode.hpp"

#include <sstream>

#include <Geode/Geode.hpp>

#include <geode.custom-keybinds/include/Keybinds.hpp>

using namespace geode::prelude;
using namespace keybinds;

bool SpeedrunNode::init() {
    m_scheduler = CCDirector::get()->getScheduler();

    if (CCNode::init()) {
        setID("timer"_spr);
        setContentSize({ 125.f, 10.f });

        auto layout = AxisLayout::create(Axis::Row)
            ->setDefaultScaleLimits(0.5f, 0.875f)
            ->setAxisAlignment(AxisAlignment::End)
            ->setCrossAxisAlignment(AxisAlignment::Start)
            ->setCrossAxisLineAlignment(AxisAlignment::Start)
            ->setCrossAxisReverse(true)
            ->setGrowCrossAxis(false)
            ->setAutoGrowAxis(125.f)
            ->setAxisReverse(true)
            ->setAutoScale(false)
            ->setGap(0.f);

        m_timeMenu = CCMenu::create();
        m_timeMenu->setID("timer-menu");
        m_timeMenu->setAnchorPoint({ 0, 0 });
        m_timeMenu->setPosition({ 0.f, 0.f });
        m_timeMenu->setZOrder(2);
        m_timeMenu->setLayout(layout);

        addChild(m_timeMenu);

        m_speedtimer = CCLabelBMFont::create("0", "gjFont16.fnt");
        m_speedtimer->setID("timer-seconds");
        m_speedtimer->setColor(m_srtMod->getSettingValue<ccColor3B>("color"));
        m_speedtimer->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
        m_speedtimer->setAnchorPoint({ 1, 0 });
        m_speedtimer->setScale(0.875f);

        m_speedtimerMs = CCLabelBMFont::create(".0", "gjFont16.fnt");
        m_speedtimerMs->setID("timer-milliseconds");
        m_speedtimerMs->setColor(m_srtMod->getSettingValue<ccColor3B>("color"));
        m_speedtimerMs->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
        m_speedtimerMs->setAnchorPoint({ 0, 0 });
        m_speedtimerMs->setScale(0.5f);

        m_timeMenu->addChild(m_speedtimerMs);
        m_timeMenu->addChild(m_speedtimer);

        m_timeMenu->updateLayout(true);

        // Create layout for scroll layer
        auto scrollLayerLayout = ColumnLayout::create()
            ->setAxisAlignment(AxisAlignment::End)
            ->setCrossAxisAlignment(AxisAlignment::End)
            ->setCrossAxisLineAlignment(AxisAlignment::End)
            ->setGrowCrossAxis(false)
            ->setAxisReverse(true)
            ->setAutoGrowAxis(0.f)
            ->setGap(1.25f);

        if (auto scroll = ScrollLayer::create({ getContentSize().width, 250.f })) {
            scroll->setID("split-list");
            scroll->ignoreAnchorPointForPosition(false);
            scroll->setAnchorPoint({ 0, 1 });
            scroll->setPosition({ 0.f, 0.f });

            scroll->m_contentLayer->setAnchorPoint({ 0, 1 });
            scroll->m_contentLayer->setLayout(scrollLayerLayout);

            m_splitList = scroll;
            addChild(m_splitList);

            m_splitList->m_contentLayer->updateLayout(true);
            m_splitList->scrollToTop();
        } else {
            log::error("Failed to create scroll layer for speedrun splits");
        };

        m_scheduler->scheduleUpdateForTarget(this, 0, false);

        auto bg = CCLayerColor::create({ 0, 0,0,255 });
        bg->setID("background");
        bg->setAnchorPoint(m_timeMenu->getAnchorPoint());
        bg->setPosition(m_timeMenu->getPosition());
        bg->setScaledContentSize(m_timeMenu->getScaledContentSize());
        bg->setZOrder(1);

        addChild(bg);

        m_timeMenu->setScale(0.875f);

        // toggle timer
        this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
            if (event->isDown()) pauseTimer(!m_speedtimerPaused); // toggle the timer on or off
            log::info("Speedrun timer set to {}", m_speedtimerPaused ? "paused" : "resumed");

            return ListenerResult::Propagate;
                                                          }, "pause-timer"_spr);

        // create a split
        this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
            if (event->isDown()) createSplit(); // create a split at the current time
            log::info("Speedrun split created at {} seconds", m_speedTime);

            return ListenerResult::Propagate;
                                                          }, "split-timer"_spr);

        // reset everything
        this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
            if (event->isDown()) resetAll(); // reset the entire speedrun
            log::info("Speedrun fully reset");

            return ListenerResult::Propagate;
                                                          }, "reset-timer"_spr);

        return true;
    } else {
        return false;
    };
};

void SpeedrunNode::update(float dt) {
    auto current = as<int>(m_speedTime);
    if (m_speedtimerOn) m_speedTime += m_speedtimerPaused ? 0.f : dt;

    auto newTime = as<int>(m_speedTime);
    std::string secStr = std::to_string(newTime);

    if (m_srtMod->getSettingValue<bool>("format-minutes")) {
        int minutes = as<int>(m_speedTime / 60.f);
        int seconds = newTime % 60;

        if (minutes > 0) {
            std::ostringstream oss;
            oss << minutes << ":" << (seconds > 9 ? "" : "0") << seconds;
            secStr = oss.str();
        } else {
            secStr = std::to_string(seconds);
        };
    } else {
        secStr = std::to_string(newTime);
    };

    if (m_speedtimer) m_speedtimer->setCString(secStr.c_str()); // seconds

    if (m_speedtimerMs) { // ms
        std::string msStr = "." + std::to_string(as<int>(m_speedTime * 100) % 100);
        m_speedtimerMs->setCString(msStr.c_str());
    };

    if (m_timeMenu) m_timeMenu->updateLayout(true);
};

void SpeedrunNode::toggleTimer(bool toggle) {
    if (m_speedtimerOn == toggle) {
        log::info("Speedrun timer is already {}", toggle ? "on" : "off");
    } else {
        m_speedtimerOn = toggle;

        if (m_speedtimerOn) {
            log::info("Speedrun timer started");
        } else {
            log::info("Speedrun timer stopped at {} seconds", m_speedTime);
        };
    };
};

void SpeedrunNode::pauseTimer(bool pause) {
    if (m_speedtimerPaused == pause) {
        log::info("Speedrun timer is already {}", pause ? "paused" : "unpaused");
    } else {
        m_speedtimerPaused = pause;

        if (m_speedtimerPaused) {
            log::info("Speedrun timer paused at {} seconds", m_speedTime);
        } else {
            log::info("Speedrun timer unpaused");
        };
    };
};

void SpeedrunNode::createSplit() {
    if (m_speedtimerOn) {
        if (auto splitNode = SplitNode::create(m_speedTime)) {
            if (m_splitList) m_splitList->m_contentLayer->addChild(splitNode);
            if (m_splitList) m_splitList->m_contentLayer->updateLayout(true);

            log::info("Speedrun split created at {} seconds", m_speedTime);
        } else {
            log::error("Failed to create speedrun split node");
        };
    } else {
        log::error("Speedrun timer is not active");
    };
};

void SpeedrunNode::resetAll() {
    m_speedTime = 0.f;

    m_speedtimerPaused = true;

    if (m_speedtimer) m_speedtimer->setCString("0");
    if (m_speedtimerMs) m_speedtimerMs->setCString(".0");

    if (m_splitList) {
        m_splitList->m_contentLayer->removeAllChildren();
        m_splitList->m_contentLayer->updateLayout(true);
    } else {
        log::error("Split list not found");
    };

    log::info("Speedrun timer reset");
};

SpeedrunNode* SpeedrunNode::create() {
    SpeedrunNode* ret = new SpeedrunNode();

    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};