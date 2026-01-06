#include "../RunTimer.hpp"

#include "../SplitSegment.hpp"

#include <sstream>

#include <Geode/Geode.hpp>

#include <Geode/utils/terminate.hpp>

using namespace geode::prelude;

#ifndef GEODE_IS_IOS
#include <geode.custom-keybinds/include/Keybinds.hpp>
using namespace keybinds;
#endif

class RunTimer::Impl final {
public:
    Mod* m_srtMod = getMod(); // it's modding time :3

    ccColor3B m_col = m_srtMod->getSettingValue<ccColor3B>("color"); // The color of the speedrun timer
    ccColor3B m_colPause = m_srtMod->getSettingValue<ccColor3B>("color-pause"); // The color of the speedrun timer when paused
    ccColor3B m_colStart = m_srtMod->getSettingValue<ccColor3B>("color-start"); // The color of the speedrun timer before starting

    CCScheduler* m_scheduler = nullptr; // The scheduler for the speedrun timer

    float m_runTime = 0.f; // Current time
    float m_lastSplitTime = 0.f; // Last time a split was created

    bool m_speedtimerOn = false; // If the speedrun is active
    bool m_speedtimerPaused = true; // If the speedrun is paused

    CCMenu* m_timeMenu = nullptr; // The menu that contains the timers

    CCLabelBMFont* m_speedtimer = nullptr; // The text label for the time
    CCLabelBMFont* m_speedtimerMs = nullptr; // The text label for the time in milliseconds

    ScrollLayer* m_splitList = nullptr; // The scrolling list of timer splits
};

RunTimer::RunTimer() {
    m_impl = std::make_unique<Impl>();
};

RunTimer::~RunTimer() {};

bool RunTimer::init() {
    m_impl->m_scheduler = CCScheduler::get();

    if (!CCNode::init()) return false;

    setID("timer"_spr);
    setContentSize({ 125.f, 37.5f });

    // assign the milliseconds timer
    m_impl->m_speedtimerMs = CCLabelBMFont::create(".00", "gjFont16.fnt");
    m_impl->m_speedtimerMs->setID("timer-milliseconds");
    m_impl->m_speedtimerMs->setScale(0.5f);
    m_impl->m_speedtimerMs->setColor(m_impl->m_colStart);
    m_impl->m_speedtimerMs->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    m_impl->m_speedtimerMs->setPosition({ getScaledContentWidth() - (m_impl->m_speedtimerMs->getScaledContentWidth() + 2.5f), 0.5f });
    m_impl->m_speedtimerMs->setAnchorPoint({ 0, 0 });

    // assign the seconds timer
    m_impl->m_speedtimer = CCLabelBMFont::create("0", "gjFont16.fnt");
    m_impl->m_speedtimer->setID("timer-seconds");
    m_impl->m_speedtimer->setScale(0.875f);
    m_impl->m_speedtimer->setColor(m_impl->m_colStart);
    m_impl->m_speedtimer->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
    m_impl->m_speedtimer->setPosition({ getScaledContentWidth() - (m_impl->m_speedtimerMs->getScaledContentWidth() + 3.75f), 0.5f });
    m_impl->m_speedtimer->setAnchorPoint({ 1, 0 });

    addChild(m_impl->m_speedtimerMs);
    addChild(m_impl->m_speedtimer);

    // Create layout for scroll layer
    auto scrollLayerLayout = ColumnLayout::create()
        ->setAxisAlignment(AxisAlignment::End)
        ->setCrossAxisAlignment(AxisAlignment::End)
        ->setCrossAxisLineAlignment(AxisAlignment::End)
        ->setAutoGrowAxis(getScaledContentHeight())
        ->setGrowCrossAxis(false)
        ->setAxisReverse(true)
        ->setGap(0.625f);

    if (auto scroll = ScrollLayer::create({ getContentSize().width, 275.f })) {
        scroll->setID("split-list");
        scroll->ignoreAnchorPointForPosition(false);
        scroll->setAnchorPoint({ 1, 1 });
        scroll->setPosition({ getScaledContentWidth(), getScaledContentHeight() - 1.f });
        scroll->setTouchEnabled(false);

        scroll->m_contentLayer->setAnchorPoint({ 0, 1 });
        scroll->m_contentLayer->setLayout(scrollLayerLayout);
        scroll->m_contentLayer->setTouchEnabled(false);

        m_impl->m_splitList = scroll;
        addChild(m_impl->m_splitList);

        m_impl->m_splitList->m_contentLayer->updateLayout(true);
        m_impl->m_splitList->scrollToTop();
    } else {
        log::error("Failed to create scroll layer for speedrun splits");
    };

    auto bgOpacity = static_cast<int>(m_impl->m_srtMod->getSettingValue<int64_t>("bg-opacity"));

    auto bg = CCLayerColor::create({ 0, 0, 0, 255 });
    bg->setID("background");
    bg->setOpacity(bgOpacity);
    bg->setAnchorPoint({ 0, 0 });
    bg->setPosition({ 0.f, 0.f });
    bg->setScaledContentSize(getScaledContentSize());
    bg->setZOrder(-1);

    addChild(bg);

#ifndef GEODE_IS_IOS
    // toggle timer
    this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
        if (event->isDown()) pauseTimer(!m_impl->m_speedtimerPaused); // toggle the timer on or off
        log::info("Speedrun timer set to {}", m_impl->m_speedtimerPaused ? "paused" : "resumed");

        return ListenerResult::Propagate;
                                                      }, "pause-timer"_spr);

    // create a split
    this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
        if (event->isDown()) createSplit(); // create a split at the current time
        log::info("Speedrun split created at {} seconds", m_impl->m_runTime);

        return ListenerResult::Propagate;
                                                      }, "split-timer"_spr);

    // reset everything
    this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
        if (event->isDown()) resetAll(); // reset the entire speedrun
        log::info("Speedrun fully reset");

        return ListenerResult::Propagate;
                                                      }, "reset-timer"_spr);
#endif

    setScale(static_cast<float>(m_impl->m_srtMod->getSettingValue<double>("scale")));

    if (m_impl->m_scheduler) m_impl->m_scheduler->scheduleUpdateForTarget(this, 0, false);

    return true;
};

void RunTimer::update(float dt) {
    // always go at 1x speed
    auto dtScale = m_impl->m_scheduler->getTimeScale();
    if (dtScale != 0.f) dt = dt / dtScale;

    if (m_impl->m_speedtimerOn) m_impl->m_runTime += m_impl->m_speedtimerPaused ? 0.f : dt;

    auto newTime = static_cast<int>(m_impl->m_runTime);
    std::string secStr = std::to_string(newTime);

    if (m_impl->m_srtMod->getSettingValue<bool>("format-minutes")) {
        int minutes = static_cast<int>(m_impl->m_runTime / 60.f);
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

    if (m_impl->m_speedtimer) m_impl->m_speedtimer->setCString(secStr.c_str()); // seconds

    if (m_impl->m_speedtimerMs) { // ms
        int ms = static_cast<int>(m_impl->m_runTime * 100) % 100;

        std::ostringstream oss;
        oss << "." << (ms > 9 ? "" : "0") << ms;

        m_impl->m_speedtimerMs->setCString(oss.str().c_str());
    };
};

void RunTimer::toggleTimer(bool toggle) {
    if (m_impl->m_speedtimerOn == toggle) {
        log::info("Speedrun timer is already {}", toggle ? "on" : "off");
    } else {
        m_impl->m_speedtimerOn = toggle;

        if (m_impl->m_speedtimerOn) {
            auto color = m_impl->m_runTime < 0.f ? m_impl->m_col : m_impl->m_colStart;

            if (m_impl->m_speedtimer) m_impl->m_speedtimer->setColor(color);
            if (m_impl->m_speedtimerMs) m_impl->m_speedtimerMs->setColor(color);

            log::info("Speedrun timer started");
        } else {
            if (m_impl->m_speedtimer) m_impl->m_speedtimer->setColor(m_impl->m_colStart);
            if (m_impl->m_speedtimerMs) m_impl->m_speedtimerMs->setColor(m_impl->m_colStart);

            log::info("Speedrun timer stopped at {} seconds", m_impl->m_runTime);
        };
    };
};

void RunTimer::pauseTimer(bool pause) {
    if (m_impl->m_speedtimerPaused == pause) {
        log::info("Speedrun timer is already {}", pause ? "paused" : "unpaused");
    } else {
        m_impl->m_speedtimerPaused = pause;

        if (m_impl->m_speedtimerPaused) {
            if (m_impl->m_speedtimer) m_impl->m_speedtimer->setColor(m_impl->m_colPause);
            if (m_impl->m_speedtimerMs) m_impl->m_speedtimerMs->setColor(m_impl->m_colPause);

            log::info("Speedrun timer paused at {} seconds", m_impl->m_runTime);
        } else {
            if (m_impl->m_speedtimer) m_impl->m_speedtimer->setColor(m_impl->m_col);
            if (m_impl->m_speedtimerMs) m_impl->m_speedtimerMs->setColor(m_impl->m_col);

            log::info("Speedrun timer unpaused");
        };
    };
};

void RunTimer::createSplit() {
    if (m_impl->m_speedtimerOn) {
        auto splitDelta = m_impl->m_runTime - m_impl->m_lastSplitTime;
        m_impl->m_lastSplitTime = m_impl->m_runTime;

        if (auto splitNode = SplitSegment::create(m_impl->m_runTime, splitDelta)) {

            auto limit = static_cast<int>(m_impl->m_srtMod->getSettingValue<int64_t>("split-limit"));
            auto withinLimit = (limit - 1) >= m_impl->m_splitList->m_contentLayer->getChildrenCount();

            if (m_impl->m_splitList) {
                if (withinLimit) {
                    log::info("Adding split node to split list");
                } else {
                    log::warn("Split limit reached, removing first split node");

                    auto children = m_impl->m_splitList->m_contentLayer->getChildren();
                    if (children && children->count() > 0) {
                        auto firstChild = static_cast<CCNode*>(children->objectAtIndex(0));
                        if (firstChild) firstChild->removeMeAndCleanup();
                    } else {
                        log::warn("No split nodes to remove");
                    };
                };

                m_impl->m_splitList->m_contentLayer->addChild(splitNode);
                m_impl->m_splitList->m_contentLayer->updateLayout(true);
            } else {
                splitNode->removeMeAndCleanup();
                log::error("Split list not found");
            };

            log::info("Speedrun split created at {} seconds{}", m_impl->m_runTime, withinLimit ? "" : " (limit reached, node not added)");
        } else {
            log::error("Failed to create speedrun split node");
        };
    } else {
        log::error("Speedrun timer is not active");
    };
};

void RunTimer::resetAll() {
    m_impl->m_runTime = 0.f;
    m_impl->m_lastSplitTime = 0.f;

    m_impl->m_speedtimerPaused = true;

    if (m_impl->m_speedtimer) {
        m_impl->m_speedtimer->setCString("0");
        m_impl->m_speedtimer->setColor(m_impl->m_colStart);
    } else {
        log::error("Speedrun timer label not found");
    };

    if (m_impl->m_speedtimerMs) {
        m_impl->m_speedtimerMs->setCString(".00");
        m_impl->m_speedtimerMs->setColor(m_impl->m_colStart);
    } else {
        log::error("Speedrun timer milliseconds label not found");
    };

    if (m_impl->m_splitList) {
        m_impl->m_splitList->m_contentLayer->removeAllChildren();
        m_impl->m_splitList->m_contentLayer->updateLayout(true);
    } else {
        log::error("Split list not found");
    };

    log::info("Speedrun timer reset");
};

bool RunTimer::isTimerPaused() const {
    return m_impl->m_speedtimerPaused;
};

RunTimer* RunTimer::create() {
    auto ret = new RunTimer();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};