#include "../RunTimer.hpp"

#include "../SplitSegment.hpp"

#include <sstream>

#include <Geode/Geode.hpp>

using namespace geode::prelude;

#ifndef GEODE_IS_IOS
#include <geode.custom-keybinds/include/Keybinds.hpp>
using namespace keybinds;
#endif

bool RunTimer::init() {
    m_scheduler = CCDirector::get()->getScheduler();

    if (CCNode::init()) {
        setID("timer"_spr);
        setContentSize({ 125.f, 0.f });

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
        m_timeMenu->setAnchorPoint({ 1, 1 });
        m_timeMenu->setPosition({ getScaledContentWidth(), 0.f });
        m_timeMenu->setZOrder(2);
        m_timeMenu->setLayout(layout);

        addChild(m_timeMenu);

        m_speedtimer = CCLabelBMFont::create("0", "gjFont16.fnt");
        m_speedtimer->setID("timer-seconds");
        m_speedtimer->setColor(m_colStart);
        m_speedtimer->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
        m_speedtimer->setAnchorPoint({ 1, 0 });
        m_speedtimer->setScale(0.875f);

        m_speedtimerMs = CCLabelBMFont::create(".0", "gjFont16.fnt");
        m_speedtimerMs->setID("timer-milliseconds");
        m_speedtimerMs->setColor(m_colStart);
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
            ->setGap(0.625f);

        if (auto scroll = ScrollLayer::create({ getContentSize().width, 275.f })) {
            scroll->setID("split-list");
            scroll->ignoreAnchorPointForPosition(false);
            scroll->setAnchorPoint({ 0, 1 });
            scroll->setPosition({ 15.f, -0.25f - m_timeMenu->getScaledContentHeight() });
            scroll->setTouchEnabled(false);

            scroll->m_contentLayer->setAnchorPoint({ 0, 1 });
            scroll->m_contentLayer->setLayout(scrollLayerLayout);
            scroll->m_contentLayer->setTouchEnabled(false);

            m_splitList = scroll;
            addChild(m_splitList);

            m_splitList->m_contentLayer->updateLayout(true);
            m_splitList->scrollToTop();
        } else {
            log::error("Failed to create scroll layer for speedrun splits");
        };

        m_scheduler->scheduleUpdateForTarget(this, 0, false);

        auto bgOpacity = as<int>(m_srtMod->getSettingValue<int64_t>("bg-opacity"));

        auto bg = CCLayerColor::create({ 0, 0, 0, 255 });
        bg->setID("background");
        bg->setOpacity(bgOpacity);
        bg->setPosition({ 15.f, 2.5f - m_timeMenu->getScaledContentHeight() });
        bg->setScaledContentSize(m_timeMenu->getScaledContentSize());
        bg->setZOrder(1);

        addChild(bg);

        m_timeMenu->setScale(0.875f);

#ifndef GEODE_IS_IOS
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
#endif

        setScale(as<float>(m_srtMod->getSettingValue<double>("scale")));

        return true;
    } else {
        return false;
    };
};

void RunTimer::update(float dt) {
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
        int ms = as<int>(m_speedTime * 100) % 100;

        std::ostringstream oss;
        oss << "." << (ms > 9 ? "" : "0") << ms;

        m_speedtimerMs->setCString(oss.str().c_str());
    };
};

void RunTimer::toggleTimer(bool toggle) {
    if (m_speedtimerOn == toggle) {
        log::info("Speedrun timer is already {}", toggle ? "on" : "off");
    } else {
        m_speedtimerOn = toggle;

        if (m_speedtimerOn) {
            auto color = m_speedTime < 0.f ? m_col : m_colStart;

            if (m_speedtimer) m_speedtimer->setColor(color);
            if (m_speedtimerMs) m_speedtimerMs->setColor(color);

            log::info("Speedrun timer started");
        } else {
            if (m_speedtimer) m_speedtimer->setColor(m_colStart);
            if (m_speedtimerMs) m_speedtimerMs->setColor(m_colStart);

            log::info("Speedrun timer stopped at {} seconds", m_speedTime);
        };
    };
};

void RunTimer::pauseTimer(bool pause) {
    if (m_speedtimerPaused == pause) {
        log::info("Speedrun timer is already {}", pause ? "paused" : "unpaused");
    } else {
        m_speedtimerPaused = pause;

        if (m_speedtimerPaused) {
            if (m_speedtimer) m_speedtimer->setColor(m_colPause);
            if (m_speedtimerMs) m_speedtimerMs->setColor(m_colPause);

            log::info("Speedrun timer paused at {} seconds", m_speedTime);
        } else {
            if (m_speedtimer) m_speedtimer->setColor(m_col);
            if (m_speedtimerMs) m_speedtimerMs->setColor(m_col);

            log::info("Speedrun timer unpaused");
        };
    };
};

void RunTimer::createSplit() {
    if (m_speedtimerOn) {
        auto splitDelta = m_speedTime - m_lastSplitTime;
        m_lastSplitTime = m_speedTime;

        if (auto splitNode = SplitSegment::create(m_speedTime, splitDelta)) {

            auto limit = as<int>(m_srtMod->getSettingValue<int64_t>("split-limit"));
            auto withinLimit = (limit - 1) >= m_splitList->m_contentLayer->getChildrenCount();

            if (m_splitList) {
                if (withinLimit) {
                    log::info("Adding split node to split list");
                } else {
                    log::warn("Split limit reached, removing first split node");

                    auto children = m_splitList->m_contentLayer->getChildren();
                    if (children && children->count() > 0) {
                        auto firstChild = as<CCNode*>(children->objectAtIndex(0));
                        if (firstChild) firstChild->removeMeAndCleanup();
                    } else {
                        log::warn("No split nodes to remove");
                    };
                };

                m_splitList->m_contentLayer->addChild(splitNode);
                m_splitList->m_contentLayer->updateLayout(true);
            } else {
                splitNode->removeMeAndCleanup();
                log::error("Split list not found");
            };

            log::info("Speedrun split created at {} seconds{}", m_speedTime, withinLimit ? "" : " (limit reached, node not added)");
        } else {
            log::error("Failed to create speedrun split node");
        };
    } else {
        log::error("Speedrun timer is not active");
    };
};

void RunTimer::resetAll() {
    m_speedTime = 0.f;

    m_speedtimerPaused = true;

    if (m_speedtimer) {
        m_speedtimer->setCString("0");
        m_speedtimer->setColor(m_colStart);
    } else {
        log::error("Speedrun timer label not found");
    };

    if (m_speedtimerMs) {
        m_speedtimerMs->setCString(".0");
        m_speedtimerMs->setColor(m_colStart);
    } else {
        log::error("Speedrun timer milliseconds label not found");
    };

    if (m_splitList) {
        m_splitList->m_contentLayer->removeAllChildren();
        m_splitList->m_contentLayer->updateLayout(true);
    } else {
        log::error("Split list not found");
    };

    log::info("Speedrun timer reset");
};

bool RunTimer::isTimerPaused() {
    return m_speedtimerPaused;
};

RunTimer* RunTimer::create() {
    RunTimer* ret = new RunTimer();

    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};