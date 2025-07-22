#include "./headers/SpeedrunNode.hpp"

#include <Geode/Geode.hpp>

#include <Geode/utils/terminate.hpp>

#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

#ifndef GEODE_IS_IOS
#include <geode.custom-keybinds/include/Keybinds.hpp>
using namespace keybinds;
#endif

// it's modding time :3
auto srt = getMod();

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        GameObject* m_destroyPlayerObject = nullptr; // Destroyed player object
        CheckpointGameObject* m_checkpointObject = nullptr; // Latest checkpoint

        SpeedrunNode* m_speedrunNode = nullptr; // The speedrun node for the timer

        CCMenu* m_mobileMenu = nullptr; // Mobile controls menu

        bool m_draggingMobile = false; // If the player is dragging the mobile menu
    };

    bool init(GJGameLevel * level, bool useReplay, bool dontCreateObjects) {
        if (PlayLayer::init(level, useReplay, dontCreateObjects)) {
            if (srt->getSettingValue<bool>("enabled")) if (srt->getSettingValue<bool>("platformer-only") ? level->isPlatformer() : true) {
                auto [widthCS, heightCS] = getScaledContentSize();

                // create speedrun timer label
                if (auto sr = SpeedrunNode::create()) {
                    sr->setZOrder(101);
                    sr->setAnchorPoint({ 1, 1 });
                    sr->setPosition({ widthCS - 25.f, heightCS - 25.f });

                    sr->toggleTimer(true); // enable the timer

                    m_fields->m_speedrunNode = sr;

                    addChild(m_fields->m_speedrunNode);

#ifndef GEODE_IS_IOS
                    // remove timer
                    this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
                        if (event->isDown()) m_fields->m_speedrunNode->setVisible(!m_fields->m_speedrunNode->isVisible());

                        log::warn("Speedrun timer removed by keybind");

                        return ListenerResult::Propagate;
                                                                      }, "hide-timer"_spr);
#endif

                    // create mobile controls
                    if (srt->getSettingValue<bool>("mobile-btns")) {
                        auto btnMenuLayout = AxisLayout::create(Axis::Row)
                            ->setDefaultScaleLimits(0.5f, 0.875f)
                            ->setAxisAlignment(AxisAlignment::Start)
                            ->setCrossAxisAlignment(AxisAlignment::Start)
                            ->setCrossAxisLineAlignment(AxisAlignment::Start)
                            ->setCrossAxisReverse(true)
                            ->setGrowCrossAxis(false)
                            ->setAutoGrowAxis(125.f)
                            ->setAxisReverse(false)
                            ->setAutoScale(false)
                            ->setGap(3.75f);

                        auto opacity = as<int>(srt->getSettingValue<int64_t>("mobile-opacity"));

                        // menu for mobile controls
                        m_fields->m_mobileMenu = CCMenu::create();
                        m_fields->m_mobileMenu->setID("mobile-controls"_spr);
                        m_fields->m_mobileMenu->setAnchorPoint({ 0, 1 });
                        m_fields->m_mobileMenu->setPosition({ 20.f, getScaledContentHeight() - 20.f });
                        m_fields->m_mobileMenu->setZOrder(102);
                        m_fields->m_mobileMenu->setLayout(btnMenuLayout);

                        auto pauseTimerBtnSprite = CCSprite::createWithSpriteFrameName("GJ_pauseBtn_001.png");
                        pauseTimerBtnSprite->setScale(0.8f);
                        pauseTimerBtnSprite->setOpacity(opacity);

                        // create the pause button
                        auto pauseTimerBtn = CCMenuItemSpriteExtra::create(
                            pauseTimerBtnSprite,
                            this,
                            menu_selector(MyPlayLayer::pauseTimer)
                        );
                        pauseTimerBtn->setID("mobile-pause");

                        auto splitTimerBtnSprite = CCSprite::createWithSpriteFrameName("GJ_practiceBtn_001.png");
                        splitTimerBtnSprite->setScale(0.5f);
                        splitTimerBtnSprite->setOpacity(opacity);

                        // create the split button
                        auto splitTimerBtn = CCMenuItemSpriteExtra::create(
                            splitTimerBtnSprite,
                            this,
                            menu_selector(MyPlayLayer::createSplit)
                        );
                        splitTimerBtn->setID("mobile-split");

                        auto resetTimerBtnSprite = CCSprite::createWithSpriteFrameName("GJ_replayBtn_001.png");
                        resetTimerBtnSprite->setScale(0.5f);
                        resetTimerBtnSprite->setOpacity(opacity);

                        // create the reset button
                        auto resetTimerBtn = CCMenuItemSpriteExtra::create(
                            resetTimerBtnSprite,
                            this,
                            menu_selector(MyPlayLayer::resetAll)
                        );
                        resetTimerBtn->setID("mobile-reset");

                        m_fields->m_mobileMenu->addChild(pauseTimerBtn);
                        m_fields->m_mobileMenu->addChild(splitTimerBtn);
                        m_fields->m_mobileMenu->addChild(resetTimerBtn);

                        addChild(m_fields->m_mobileMenu);

                        m_fields->m_mobileMenu->updateLayout();
                    } else {
                        log::warn("Mobile controls are disabled");
                    };
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

    void pauseTimer(CCObject*) {
        if (m_fields->m_speedrunNode) {
            m_fields->m_speedrunNode->pauseTimer(!m_fields->m_speedrunNode->m_speedtimerPaused);
        } else {
            log::error("Speedrun node is not initialized");
        };
    };

    void createSplit(CCObject*) {
        if (m_fields->m_speedrunNode) {
            m_fields->m_speedrunNode->createSplit();
        } else {
            log::error("Speedrun node is not initialized");
        };
    };

    void resetAll(CCObject*) {
        if (m_fields->m_speedrunNode) {
            m_fields->m_speedrunNode->resetAll();
        } else {
            log::error("Speedrun node is not initialized");
        };
    };

    void destroyPlayer(PlayerObject * p0, GameObject * p1) {
        if (srt->getSettingValue<bool>("reset-death")) { // check if reset after death is enabled
            if (p1 == m_fields->m_destroyPlayerObject) {
                log::warn("Player already destroyed");
            } else {
                if (m_fields->m_speedrunNode) m_fields->m_speedrunNode->pauseTimer(true);
            };
        } else {
            log::warn("Death reset is disabled");
        };

        m_fields->m_destroyPlayerObject = p1;

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

        if (srt->getSettingValue<bool>("reset-death")) { // check if reset after death is enabled
            if (m_fields->m_speedrunNode) m_fields->m_speedrunNode->pauseTimer(false); // force resume
        } else {
            log::info("Timer will not resume on checkpoint activation");
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

    // player starts touching the menu
    bool ccTouchBegan(CCTouch * touch, CCEvent * event) {
        if (srt->getSettingValue<bool>("mobile-btns") &&
            m_fields->m_speedrunNode &&
            m_fields->m_speedrunNode->isVisible() &&
            m_fields->m_mobileMenu) {

            auto touchLocation = touch->getLocation();
            if (m_fields->m_mobileMenu->boundingBox().containsPoint(touchLocation)) m_fields->m_draggingMobile = true;
        };

        return PlayLayer::ccTouchBegan(touch, event);
    };

    // player is dragging the menu
    void ccTouchMoved(CCTouch * touch, CCEvent * event) {
        if (srt->getSettingValue<bool>("mobile-btns") &&
            m_fields->m_draggingMobile &&
            m_fields->m_mobileMenu) {

            auto touchLocation = touch->getLocation();
            m_fields->m_mobileMenu->setPosition(touchLocation);
        };

        PlayLayer::ccTouchMoved(touch, event);
    };

    // player stops touching the menu
    void ccTouchEnded(CCTouch * touch, CCEvent * event) {
        if (srt->getSettingValue<bool>("mobile-btns") &&
            m_fields->m_draggingMobile) {

            m_fields->m_draggingMobile = false;
        };

        PlayLayer::ccTouchEnded(touch, event);
    };
};