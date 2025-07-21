#include "./headers/SpeedrunNode.hpp"

#include <Geode/Geode.hpp>

#include <Geode/utils/terminate.hpp>

#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

auto srt = getMod();

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        SpeedrunNode* m_speedrunNode = nullptr; // The speedrun node for the timer

        CCMenuItemSpriteExtra* m_mobileToggle = nullptr; // The toggle button for mobile players
    };

    bool init(GJGameLevel * level, bool useReplay, bool dontCreateObjects) {
        if (PlayLayer::init(level, useReplay, dontCreateObjects)) {
            if (srt->getSettingValue<bool>("platformer-only") ? level->isPlatformer() : true) {
                auto [widthCS, heightCS] = getScaledContentSize();

                if (m_fields->m_speedrunNode = SpeedrunNode::create()) {
                    m_fields->m_speedrunNode->setZOrder(101);
                    m_fields->m_speedrunNode->setAnchorPoint({ 1, 1 });
                    m_fields->m_speedrunNode->setPosition({ widthCS - 20.f, heightCS - 30.f });

                    addChild(m_fields->m_speedrunNode);
                } else {
                    log::error("Failed to create timer!");
                };
            } else {
                log::error("Level is not a platformer");
            };

            return true;
        } else {
            return false;
        };
    };

    void destroyPlayer(PlayerObject * p0, GameObject * p1) {
        if (srt->getSettingValue<bool>("reset-death")) {
            if (m_fields->m_speedrunNode) m_fields->m_speedrunNode->resetAll();
        } else {
            log::info("Speedrun timer will not reset on death");
        };

        PlayLayer::destroyPlayer(p0, p1);
    };
};