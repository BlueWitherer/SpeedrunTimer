#include "../SpeedrunNode.hpp"

#include <Geode/Geode.hpp>

#include <Geode/cocos/CCScheduler.h>

#include <geode.custom-keybinds/include/Keybinds.hpp>

using namespace geode::prelude;
using namespace keybinds;

bool SpeedrunNode::init() {
    if (CCNode::init()) {
        m_scheduler = CCDirector::get()->getScheduler();

        auto layout = AxisLayout::create(Axis::Row)
            ->setDefaultScaleLimits(0.625f, 0.875f)
            ->setAxisAlignment(AxisAlignment::End)
            ->setCrossAxisAlignment(AxisAlignment::Start)
            ->setCrossAxisLineAlignment(AxisAlignment::Start)
            ->setGrowCrossAxis(false)
            ->setAutoGrowAxis(125.f)
            ->setAxisReverse(true)
            ->setAutoScale(false)
            ->setGap(0.f);

        setID("timer"_spr);
        setContentSize({ 125.f, 10.f });
        setLayout(layout);

        m_speedtimer = CCLabelBMFont::create("0", "gjFont17.fnt");
        m_speedtimer->setColor(m_srtMod->getSettingValue<ccColor3B>("color"));
        m_speedtimer->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
        m_speedtimer->setAnchorPoint({ 1, 0 });
        m_speedtimer->setScale(0.875f);

        m_speedtimerMs = CCLabelBMFont::create(".0", "gjFont17.fnt");
        m_speedtimerMs->setColor(m_srtMod->getSettingValue<ccColor3B>("color"));
        m_speedtimerMs->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
        m_speedtimerMs->setAnchorPoint({ 0, 0 });
        m_speedtimerMs->setScale(0.625f);

        addChild(m_speedtimerMs);
        addChild(m_speedtimer);

        updateLayout(true);

        m_scheduler->scheduleUpdateForTarget(this, 0, false);

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
        int minutes = as<int>(m_speedTime / 60);
        int seconds = as<int>(m_speedTime) % 60;

        if (minutes > 0) {
            char buf[8];
            snprintf(buf, sizeof(buf), "%d:%02d", minutes, seconds);
            secStr = buf;
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

void SpeedrunNode::resetAll() {
    m_speedTime = 0.f;

    m_speedtimerOn = false;
    m_speedtimerPaused = true;

    if (m_speedtimer) m_speedtimer->setCString("0");
    if (m_speedtimerMs) m_speedtimerMs->setCString(".0");

    if (m_splitList) {
        m_splitList->removeAllChildren();
        m_splitList->updateLayout(true);
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