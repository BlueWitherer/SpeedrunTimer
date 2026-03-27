#include "../RunTimer.h"

#include "../SplitSegment.h"

#include <Geode/Geode.hpp>

#include <Geode/utils/Keyboard.hpp>

using namespace geode::prelude;

// it's modding time :3
static auto srt = Mod::get();

class RunTimer::Impl final {
public:
    ccColor3B col = srt->getSettingValue<ccColor3B>("color");             // The color of the speedrun timer
    ccColor3B colPause = srt->getSettingValue<ccColor3B>("color-pause");  // The color of the speedrun timer when paused
    ccColor3B colStart = srt->getSettingValue<ccColor3B>("color-start");  // The color of the speedrun timer before starting

    CCScheduler* scheduler = nullptr;  // The scheduler for the speedrun timer

    float runTime = 0.f;        // Current time
    float lastSplitTime = 0.f;  // Last time a split was created

    bool speedtimerOn = false;     // If the speedrun is active
    bool speedtimerPaused = true;  // If the speedrun is paused

    CCMenu* timeMenu = nullptr;  // The menu that contains the timers

    CCLabelBMFont* speedtimer = nullptr;    // The text label for the time
    CCLabelBMFont* speedtimerMs = nullptr;  // The text label for the time in milliseconds

    ScrollLayer* splitList = nullptr;  // The scrolling list of timer splits
};

RunTimer::RunTimer() : m_impl(std::make_unique<Impl>()) {};
RunTimer::~RunTimer() {};

bool RunTimer::init() {
    m_impl->scheduler = CCScheduler::get();

    if (!CCNode::init()) return false;

    setID("timer"_spr);
    setContentSize({125.f, 37.5f});

    // assign the milliseconds timer
    m_impl->speedtimerMs = CCLabelBMFont::create(".00", "gjFont16.fnt");
    m_impl->speedtimerMs->setID("timer-milliseconds");
    m_impl->speedtimerMs->setScale(0.5f);
    m_impl->speedtimerMs->setColor(m_impl->colStart);
    m_impl->speedtimerMs->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    m_impl->speedtimerMs->setPosition({getScaledContentWidth() - (m_impl->speedtimerMs->getScaledContentWidth() + 2.5f), 0.5f});
    m_impl->speedtimerMs->setAnchorPoint({0, 0});

    // assign the seconds timer
    m_impl->speedtimer = CCLabelBMFont::create("0", "gjFont16.fnt");
    m_impl->speedtimer->setID("timer-seconds");
    m_impl->speedtimer->setScale(0.875f);
    m_impl->speedtimer->setColor(m_impl->colStart);
    m_impl->speedtimer->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
    m_impl->speedtimer->setPosition({getScaledContentWidth() - (m_impl->speedtimerMs->getScaledContentWidth() + 3.75f), 0.5f});
    m_impl->speedtimer->setAnchorPoint({1, 0});

    addChild(m_impl->speedtimerMs);
    addChild(m_impl->speedtimer);

    // Create layout for scroll layer
    auto scrollLayerLayout = ColumnLayout::create()
                                 ->setAxisAlignment(AxisAlignment::End)
                                 ->setCrossAxisAlignment(AxisAlignment::End)
                                 ->setCrossAxisLineAlignment(AxisAlignment::End)
                                 ->setAutoGrowAxis(getScaledContentHeight())
                                 ->setGrowCrossAxis(false)
                                 ->setAxisReverse(true)
                                 ->setGap(0.625f);

    if (auto scroll = ScrollLayer::create({getContentSize().width, 275.f})) {
        scroll->setID("split-list");
        scroll->ignoreAnchorPointForPosition(false);
        scroll->setAnchorPoint({1, 1});
        scroll->setPosition({getScaledContentWidth(), getScaledContentHeight() - 1.f});
        scroll->setTouchEnabled(false);

        scroll->m_contentLayer->setAnchorPoint({0, 1});
        scroll->m_contentLayer->setLayout(scrollLayerLayout);
        scroll->m_contentLayer->setTouchEnabled(false);

        m_impl->splitList = scroll;
        addChild(m_impl->splitList);

        m_impl->splitList->m_contentLayer->updateLayout();
        m_impl->splitList->scrollToTop();
    } else {
        log::error("Failed to create scroll layer for speedrun splits");
    };

    auto bgOpacity = srt->getSettingValue<int>("bg-opacity");

    auto bg = CCLayerColor::create({0, 0, 0, 255});
    bg->setID("background");
    bg->setOpacity(bgOpacity);
    bg->setAnchorPoint({0, 0});
    bg->setPosition({0.f, 0.f});
    bg->setScaledContentSize(getScaledContentSize());

    addChild(bg, -1);

    addEventListener(
        KeybindSettingPressedEventV3(Mod::get(), "key-pause"),
        [this](Keybind const&, bool down, bool repeat, double) {
            if (down && !repeat) {
                pauseTimer(!m_impl->speedtimerPaused);  // toggle the timer on or off
                log::info("Speedrun timer {}", isTimerPaused() ? "paused" : "resumed");
            };
        });

    addEventListener(
        KeybindSettingPressedEventV3(Mod::get(), "key-split"),
        [this](Keybind const&, bool down, bool repeat, double) {
            if (down && !repeat) {
                createSplit();  // create a split at the current time
                log::info("Speedrun split created at {} seconds", m_impl->runTime);
            };
        });

    addEventListener(
        KeybindSettingPressedEventV3(Mod::get(), "key-reset"),
        [this](Keybind const&, bool down, bool repeat, double) {
            if (down && !repeat) {
                resetAll();  // reset the entire speedrun
                log::info("Speedrun fully reset");
            };
        });

    setScale(static_cast<float>(srt->getSettingValue<double>("scale")));

    if (m_impl->scheduler) m_impl->scheduler->scheduleUpdateForTarget(this, 0, false);

    return true;
};

void RunTimer::update(float dt) {
    // always go at 1x speed
    auto dtScale = m_impl->scheduler->getTimeScale();
    if (dtScale != 0.f) dt = dt / dtScale;

    if (m_impl->speedtimerOn) m_impl->runTime += m_impl->speedtimerPaused ? 0.f : dt;

    int newTime = static_cast<int>(m_impl->runTime);
    std::string secStr = utils::numToString(newTime);

    if (srt->getSettingValue<bool>("format-minutes")) {
        int minutes = static_cast<int>(m_impl->runTime / 60.f);
        int seconds = newTime % 60;

        if (minutes > 0) {
            secStr = fmt::format("{}:{}{}", minutes, seconds > 9 ? "" : "0", seconds);
        } else {
            secStr = utils::numToString(seconds);
        };
    } else {
        secStr = utils::numToString(m_impl->runTime);
    };

    if (m_impl->speedtimer) m_impl->speedtimer->setString(std::move(secStr).c_str());  // seconds

    if (m_impl->speedtimerMs) {  // ms
        int ms = static_cast<int>(m_impl->runTime * 100) % 100;
        m_impl->speedtimerMs->setString(fmt::format(".{}{}", ms > 9 ? "" : "0", ms).c_str());
    };
};

void RunTimer::toggleTimer(bool toggle) {
    if (m_impl->speedtimerOn != toggle) {
        m_impl->speedtimerOn = toggle;

        if (m_impl->speedtimerOn) {
            auto color = m_impl->runTime < 0.f ? m_impl->col : m_impl->colStart;

            if (m_impl->speedtimer) m_impl->speedtimer->setColor(color);
            if (m_impl->speedtimerMs) m_impl->speedtimerMs->setColor(color);

            log::info("Speedrun timer started");
        } else {
            if (m_impl->speedtimer) m_impl->speedtimer->setColor(m_impl->colStart);
            if (m_impl->speedtimerMs) m_impl->speedtimerMs->setColor(m_impl->colStart);

            log::info("Speedrun timer stopped at {} seconds", m_impl->runTime);
        };
    };
};

void RunTimer::pauseTimer(bool pause) {
    if (m_impl->speedtimerPaused != pause) {
        m_impl->speedtimerPaused = pause;

        if (m_impl->speedtimerPaused) {
            if (m_impl->speedtimer) m_impl->speedtimer->setColor(m_impl->colPause);
            if (m_impl->speedtimerMs) m_impl->speedtimerMs->setColor(m_impl->colPause);

            log::info("Speedrun timer paused at {} seconds", m_impl->runTime);
        } else {
            if (m_impl->speedtimer) m_impl->speedtimer->setColor(m_impl->col);
            if (m_impl->speedtimerMs) m_impl->speedtimerMs->setColor(m_impl->col);

            log::info("Speedrun timer unpaused");
        };
    };
};

void RunTimer::createSplit() {
    if (m_impl->speedtimerOn) {
        auto splitDelta = m_impl->runTime - m_impl->lastSplitTime;
        m_impl->lastSplitTime = m_impl->runTime;

        if (m_impl->splitList) {
            if (auto splitNode = SplitSegment::create(m_impl->runTime, splitDelta)) {
                auto limit = srt->getSettingValue<int>("split-limit");
                auto withinLimit = (limit - 1) >= m_impl->splitList->m_contentLayer->getChildrenCount();

                if (withinLimit) {
                    log::info("Adding split node to split list");
                } else {
                    log::warn("Split limit reached, removing first split node");

                    auto children = m_impl->splitList->m_contentLayer->getChildren();
                    if (children && children->count() > 0) {
                        if (auto first = typeinfo_cast<CCNode*>(children->objectAtIndex(0))) first->removeMeAndCleanup();
                    } else {
                        log::warn("No split nodes to remove");
                    };
                };

                m_impl->splitList->m_contentLayer->addChild(splitNode);
                m_impl->splitList->m_contentLayer->updateLayout();

                log::info("Speedrun split created at {} seconds{}", m_impl->runTime, withinLimit ? "" : " (limit reached, node not added)");
            } else {
                log::error("Failed to create speedrun split node");
            };
        } else {
            log::error("Split list not found");
        };
    } else {
        log::error("Speedrun timer is not active");
    };
};

void RunTimer::resetAll() {
    m_impl->runTime = 0.f;
    m_impl->lastSplitTime = 0.f;

    m_impl->speedtimerPaused = true;

    if (m_impl->speedtimer) {
        m_impl->speedtimer->setString("0");
        m_impl->speedtimer->setColor(m_impl->colStart);
    };

    if (m_impl->speedtimerMs) {
        m_impl->speedtimerMs->setString(".00");
        m_impl->speedtimerMs->setColor(m_impl->colStart);
    };

    if (m_impl->splitList) {
        m_impl->splitList->m_contentLayer->removeAllChildren();
        m_impl->splitList->m_contentLayer->updateLayout();
    };

    log::info("Speedrun timer reset");
};

bool RunTimer::isTimerPaused() const noexcept {
    return m_impl->speedtimerPaused;
};

RunTimer* RunTimer::create() {
    auto ret = new RunTimer();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    };

    delete ret;
    return nullptr;
};