#include <Geode/Geode.hpp>

#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        float m_speedTime = 0.f; // Current time
        bool m_speedtimerOn = false; // If the speedrun is active

        CCLabelBMFont* m_speedtimer = nullptr; // The text label for the time
        CCLabelBMFont* m_speedtimerMs = nullptr; // The text label for the time in milliseconds

        CCMenuItemSpriteExtra* mobileToggle = nullptr; // The toggle button for mobile players
    };

    bool init(GJGameLevel * level, bool useReplay, bool dontCreateObjects) {
        if (PlayLayer::init(level, useReplay, dontCreateObjects)) {
            log::info("Hooked play layer!");

            // code soon :3

            return true;
        } else {
            return false;
        };
    };

    void update(float p0) {
        PlayLayer::update(p0);

        if (m_fields->m_speedtimerOn) m_fields->m_speedTime += p0;
        updateSpeedrunLabel(p0);
    };

    void toggleSpeedrun() {
        m_fields->m_speedtimerOn = !m_fields->m_speedtimerOn;
        log::info("{} speedrun timer", m_fields->m_speedtimerOn ? "Started" : "Stopped");
    };

    void updateSpeedrunLabel(float time) {
        log::info("timer {}", time);
        if (m_fields->m_speedtimer) m_fields->m_speedtimer->setCString("blep");
    };
};