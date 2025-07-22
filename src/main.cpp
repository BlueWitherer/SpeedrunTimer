#include "./headers/SpeedrunNode.hpp"

#include <Geode/Geode.hpp>

#include <Geode/utils/terminate.hpp>

#include <Geode/modify/PlayLayer.hpp>

#include <geode.custom-keybinds/include/Keybinds.hpp>

using namespace geode::prelude;
using namespace keybinds;

// it's modding time :3
auto srt = getMod();

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        GameObject* m_destroyPlayerObject = nullptr; // Destroyed player object
        CheckpointGameObject* m_checkpointObject = nullptr; // Latest checkpoint

        SpeedrunNode* m_speedrunNode = nullptr; // The speedrun node for the timer

        CCMenuItemSpriteExtra* m_mobilePause = nullptr; // The toggle button for mobile players
        CCMenuItemSpriteExtra* m_mobileSplit = nullptr; // The split button for mobile players
        CCMenuItemSpriteExtra* m_mobileReset = nullptr; // The reset button for mobile players
    };

    bool init(GJGameLevel * level, bool useReplay, bool dontCreateObjects) {
        if (PlayLayer::init(level, useReplay, dontCreateObjects)) {
            if (srt->getSettingValue<bool>("enabled")) if (srt->getSettingValue<bool>("platformer-only") ? level->isPlatformer() : true) {
                auto [widthCS, heightCS] = getScaledContentSize();

                // create speedrun timer label
                if (auto sr = SpeedrunNode::create()) {
                    sr->setZOrder(101);
                    sr->setAnchorPoint({ 1, 1 });
                    sr->setPosition({ widthCS - 20.f, heightCS - 30.f });

                    sr->toggleTimer(true); // enable the timer

                    m_fields->m_speedrunNode = sr;

                    addChild(m_fields->m_speedrunNode);

                    // delete timer
                    this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
                        if (event->isDown()) m_fields->m_speedrunNode->removeMeAndCleanup();
                        m_fields->m_speedrunNode = nullptr;

                        log::warn("Speedrun timer removed by keybind");

                        return ListenerResult::Propagate;
                                                                      }, "remove-timer"_spr);
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
        if (srt->getSettingValue<bool>("reset-death")) { // check if reset after death is enabled
            if (p1 == m_fields->m_destroyPlayerObject) {
                log::warn("Player object is already destroyed");
            } else {
                if (m_fields->m_speedrunNode) m_fields->m_speedrunNode->pauseTimer(true);
            };
        } else {
            log::warn("Death reset is disabled");
        };

        PlayLayer::destroyPlayer(p0, p1);
    };

    void resetLevel() {
        if (srt->getSettingValue<bool>("reset-death")) { // check if reset after death is enabled
            if (m_fields->m_speedrunNode) m_fields->m_speedrunNode->pauseTimer(false);
        } else {
            log::info("Timer will not resume on restart");
        };

        PlayLayer::resetLevel();
    };

    void resetLevelFromStart() {
        m_fields->m_checkpointObject = nullptr; // reset the checkpoint object

        if (srt->getSettingValue<bool>("reset-death")) { // check if reset after death is enabled
            if (m_fields->m_speedrunNode) m_fields->m_speedrunNode->resetAll();
        } else {
            log::info("Timer will not reset on level restart");
        };

        PlayLayer::resetLevelFromStart();
    };

    void checkpointActivated(CheckpointGameObject * p0) {
        if (srt->getSettingValue<bool>("checkpoint-split")) { // check if split on checkpoint is enabled
            if (p0 == m_fields->m_checkpointObject) {
                log::warn("Checkpoint split is already active");
            } else {
                if (m_fields->m_speedrunNode) m_fields->m_speedrunNode->createSplit();
            };

            m_fields->m_checkpointObject = p0;
        } else {
            log::warn("Checkpoint split is disabled");
        };

        PlayLayer::checkpointActivated(p0);
    };

    void levelComplete() {
        if (srt->getSettingValue<bool>("stop-complete")) { // check if stop on level complete is enabled
            if (m_fields->m_speedrunNode) m_fields->m_speedrunNode->toggleTimer(false);
        } else {
            log::info("Timer will not stop on level completion");
        };

        PlayLayer::levelComplete();
    };
};