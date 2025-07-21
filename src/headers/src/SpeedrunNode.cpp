#include "../SpeedrunNode.hpp"

#include <Geode/Geode.hpp>

#include <Geode/cocos/CCScheduler.h>

#include <Geode/utils/terminate.hpp>

#include <geode.custom-keybinds/include/Keybinds.hpp>

using namespace geode::prelude;
using namespace keybinds;

// it's modding time :3
auto getThisMod = getMod();

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
        m_speedtimer->setColor(getThisMod->getSettingValue<ccColor3B>("color"));
        m_speedtimer->setAnchorPoint({ 1, 0 });
        m_speedtimer->setScale(0.875f);

        m_speedtimerMs = CCLabelBMFont::create(".0", "gjFont17.fnt");
        m_speedtimerMs->setColor(getThisMod->getSettingValue<ccColor3B>("color"));
        m_speedtimerMs->setAnchorPoint({ 0, 0 });
        m_speedtimerMs->setScale(0.625f);

        addChild(m_speedtimerMs);
        addChild(m_speedtimer);

        updateLayout(true);

        m_scheduler->scheduleUpdateForTarget(this, 0, false);

        this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
            if (event->isDown()) toggle(!m_speedtimerOn); // toggle the timer on or off
            log::info("Speedrun timer state set to {}", m_speedtimerOn ? "on" : "off");

            return ListenerResult::Propagate;
                                                          }, "toggle-timer"_spr);

        return true;
    } else {
        return false;
    };
};

void SpeedrunNode::update(float dt) {
    auto current = as<int>(m_speedTime);
    if (m_speedtimerOn) m_speedTime += dt;

    std::string secStr = std::to_string(as<int>(m_speedTime));

    if (getThisMod->getSettingValue<bool>("format-minutes")) {
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

bool SpeedrunNode::toggle(bool on) {
    if (m_speedtimerOn == on) {
        log::info("Speedrun timer is already {}", on ? "on" : "off");
        return m_speedtimerOn;
    } else {
        m_speedtimerOn = on;

        if (m_speedtimerOn) {
            log::info("Speedrun timer started");
            m_speedTime = 0.f; // reset the timer
        } else {
            log::info("Speedrun timer stopped at {} seconds", m_speedTime);
        };

        return m_speedtimerOn;
    };
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