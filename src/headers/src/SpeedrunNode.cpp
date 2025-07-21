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
            ->setDefaultScaleLimits(0.625f, 0.875f)
            ->setAxisAlignment(AxisAlignment::End)
            ->setCrossAxisAlignment(AxisAlignment::Start)
            ->setCrossAxisLineAlignment(AxisAlignment::Start)
            ->setCrossAxisReverse(true)
            ->setGrowCrossAxis(false)
            ->setAutoGrowAxis(125.f)
            ->setAxisReverse(true)
            ->setAutoScale(false)
            ->setGap(0.f);

        auto menu = CCMenu::create();
        menu->setID("timer-menu");
        menu->setAnchorPoint({ 0, 0 });
        menu->setPosition({ 0.f, 0.f });
        menu->setZOrder(2);
        menu->setLayout(layout);

        addChild(menu);

        m_speedtimer = CCLabelBMFont::create("0", "gjFont17.fnt");
        m_speedtimer->setID("timer-seconds");
        m_speedtimer->setColor(m_srtMod->getSettingValue<ccColor3B>("color"));
        m_speedtimer->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
        m_speedtimer->setAnchorPoint({ 1, 0 });
        m_speedtimer->setScale(0.875f);

        m_speedtimerMs = CCLabelBMFont::create(".0", "gjFont17.fnt");
        m_speedtimerMs->setID("timer-milliseconds");
        m_speedtimerMs->setColor(m_srtMod->getSettingValue<ccColor3B>("color"));
        m_speedtimerMs->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
        m_speedtimerMs->setAnchorPoint({ 0, 0 });
        m_speedtimerMs->setScale(0.625f);

        menu->addChild(m_speedtimerMs);
        menu->addChild(m_speedtimer);

        menu->updateLayout(true);

        // Create layout for scroll layer
        auto scrollLayerLayout = ColumnLayout::create()
            ->setAxisAlignment(AxisAlignment::End)
            ->setAxisReverse(true)
            ->setAutoGrowAxis(0.f)
            ->setGrowCrossAxis(false)
            ->setGap(5.f);

        // m_splitList = ScrollLayer::create({ getContentSize().width, 625.f });
        // m_splitList->setID("split-list");
        // m_splitList->setAnchorPoint({ 0, 1 });
        // m_splitList->setPosition({ 0.f, 0.f });

        // m_splitList->m_contentLayer->setLayout(scrollLayerLayout);
        // m_splitList->m_contentLayer->updateLayout(true);

        // addChild(m_splitList);

        m_scheduler->scheduleUpdateForTarget(this, 0, false);

        auto bg = CCLayerColor::create({ 0,0,0,255 });
        bg->setID("timer-bg");
        bg->setAnchorPoint(menu->getAnchorPoint());
        bg->setPosition(menu->getPosition());
        bg->setScaledContentSize(menu->getScaledContentSize());
        bg->setZOrder(1);

        addChild(bg);

        // toggle timer
        this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
            if (event->isDown()) pauseTimer(!m_speedtimerPaused); // toggle the timer on or off
            log::info("Speedrun timer set to {}", m_speedtimerPaused ? "paused" : "resumed");

            return ListenerResult::Propagate;
                                                          }, "pause-timer"_spr);

        // create a split
        this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
            if (event->isDown()) log::info("Speedrun split created at {} seconds", m_speedTime);

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

    std::string secStr = std::to_string(as<int>(m_speedTime));

    if (m_srtMod->getSettingValue<bool>("format-minutes")) {
        int minutes = as<int>(m_speedTime / 60.f);
        int seconds = as<int>(m_speedTime) % 60;

        if (minutes > 0) {
            std::ostringstream oss;
            oss << minutes << ":" << (seconds > 9 ? "" : "0") << seconds;
            secStr = oss.str();
        } else {
            secStr = std::to_string(seconds);
        };
    } else {
        secStr = std::to_string(as<int>(m_speedTime));
    };

    if (m_speedtimer) m_speedtimer->setCString(secStr.c_str()); // seconds

    if (m_speedtimerMs) { // ms
        std::string msStr = "." + std::to_string(as<int>(m_speedTime * 100) % 100);
        m_speedtimerMs->setCString(msStr.c_str());
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
            m_speedtimerOn = true;
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

    m_speedtimerOn = false;
    m_speedtimerPaused = true;

    if (m_speedtimer) m_speedtimer->setCString("0");
    if (m_speedtimerMs) m_speedtimerMs->setCString(".0");

    if (m_splitList) {
        m_splitList->removeAllChildren();
        m_splitList->updateLayout(true);
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