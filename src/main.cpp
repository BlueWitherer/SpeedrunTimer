#include "./headers/RunTimer.hpp"

#include <Geode/Geode.hpp>

#include <Geode/utils/terminate.hpp>

#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

#ifndef GEODE_IS_IOS
#include <geode.custom-keybinds/include/Keybinds.hpp>
using namespace keybinds;
#endif

// it's modding time :3
static auto srt = getMod();

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        CheckpointGameObject* m_checkpointObject = nullptr; // Latest checkpoint

        RunTimer* m_speedrunNode = nullptr; // The speedrun node for the timer

        CCMenu* m_mobileMenu = nullptr; // Mobile controls menu

        CCMenuItemToggler* m_pauseTimerBtn = nullptr; // Pause timer button for mobile controls
        CCMenuItemSpriteExtra* m_splitTimerBtn = nullptr; // Split timer button for mobile controls
        CCMenuItemSpriteExtra* m_resetTimerBtn = nullptr; // Reset timer button for mobile controls

        CCMenuItemSpriteExtra* m_visibleBtn = nullptr;

        bool m_manualPause = true;

        bool m_draggingMobile = false; // If the player is dragging the mobile menu
    };

    bool init(GJGameLevel * level, bool useReplay, bool dontCreateObjects) {
        if (PlayLayer::init(level, useReplay, dontCreateObjects)) {
            if (srt->getSettingValue<bool>("enabled")) if (srt->getSettingValue<bool>("platformer-only") ? level->isPlatformer() : true) {
                auto [widthCS, heightCS] = getScaledContentSize();

                // create speedrun timer label
                if (auto sr = RunTimer::create()) {
                    sr->setZOrder(99);
                    sr->setAnchorPoint({ 1, 1 });
                    sr->setPosition({ widthCS - 25.f, heightCS - 25.f });

                    sr->toggleTimer(true); // enable the timer

                    m_fields->m_speedrunNode = sr;

                    addChild(m_fields->m_speedrunNode);

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

                        auto opacity = static_cast<int>(srt->getSettingValue<int64_t>("mobile-btns-opacity"));

                        // menu for mobile controls
                        m_fields->m_mobileMenu = CCMenu::create();
                        m_fields->m_mobileMenu->setID("mobile-controls"_spr);
                        m_fields->m_mobileMenu->setAnchorPoint({ 0, 1 });
                        m_fields->m_mobileMenu->setPosition({ 25.f, heightCS - 25.f });
                        m_fields->m_mobileMenu->setZOrder(102);
                        m_fields->m_mobileMenu->setLayout(btnMenuLayout);

                        auto pauseTimerBtnSpriteOn = CCSprite::createWithSpriteFrameName("GJ_stopEditorBtn_001.png");
                        pauseTimerBtnSpriteOn->setScale(0.8f);
                        pauseTimerBtnSpriteOn->setOpacity(opacity);

                        auto pauseTimerBtnSpriteOff = CCSprite::createWithSpriteFrameName("GJ_playEditorBtn_001.png");
                        pauseTimerBtnSpriteOff->setScale(0.8f);
                        pauseTimerBtnSpriteOff->setOpacity(opacity);

                        // create the pause button
                        m_fields->m_pauseTimerBtn = CCMenuItemToggler::create(
                            pauseTimerBtnSpriteOn,
                            pauseTimerBtnSpriteOff,
                            this,
                            menu_selector(MyPlayLayer::pauseTimer)
                        );
                        m_fields->m_pauseTimerBtn->setID("mobile-pause");

                        auto splitTimerBtnSprite = CCSprite::createWithSpriteFrameName("GJ_practiceBtn_001.png");
                        splitTimerBtnSprite->setScale(0.5f);
                        splitTimerBtnSprite->setOpacity(opacity);

                        // create the split button
                        m_fields->m_splitTimerBtn = CCMenuItemSpriteExtra::create(
                            splitTimerBtnSprite,
                            this,
                            menu_selector(MyPlayLayer::createSplit)
                        );
                        m_fields->m_splitTimerBtn->setID("mobile-split");

                        auto resetTimerBtnSprite = CCSprite::createWithSpriteFrameName("GJ_replayBtn_001.png");
                        resetTimerBtnSprite->setScale(0.5f);
                        resetTimerBtnSprite->setOpacity(opacity);

                        // create the reset button
                        m_fields->m_resetTimerBtn = CCMenuItemSpriteExtra::create(
                            resetTimerBtnSprite,
                            this,
                            menu_selector(MyPlayLayer::resetAll)
                        );
                        m_fields->m_resetTimerBtn->setID("mobile-reset");

                        m_fields->m_pauseTimerBtn->toggle(m_fields->m_speedrunNode->isTimerPaused()); // set the initial state of the pause button

                        m_fields->m_mobileMenu->addChild(m_fields->m_pauseTimerBtn);
                        m_fields->m_mobileMenu->addChild(m_fields->m_splitTimerBtn);
                        m_fields->m_mobileMenu->addChild(m_fields->m_resetTimerBtn);

                        auto mobileScale = static_cast<float>(srt->getSettingValue<double>("mobile-btns-scale"));

                        m_fields->m_mobileMenu->setScale(mobileScale);

                        addChild(m_fields->m_mobileMenu);

                        m_fields->m_mobileMenu->updateLayout(true);

                        if (srt->getSettingValue<bool>("mobile-visible-btn")) {
                            auto visibleMenu = CCMenu::create();
                            visibleMenu->setID("mobile-visibility-menu"_spr);
                            visibleMenu->setAnchorPoint(m_fields->m_mobileMenu->getAnchorPoint());
                            visibleMenu->setPosition({ m_fields->m_mobileMenu->getPositionX(), m_fields->m_mobileMenu->getPositionY() - m_fields->m_mobileMenu->getScaledContentHeight() - 2.5f });
                            visibleMenu->setZOrder(m_fields->m_mobileMenu->getZOrder());
                            visibleMenu->setLayout(btnMenuLayout);

                            auto visibleBtnSprite = CCSprite::createWithSpriteFrameName("hideBtn_001.png");
                            visibleBtnSprite->setScale(0.875f);
                            visibleBtnSprite->setOpacity(200);

                            m_fields->m_visibleBtn = CCMenuItemSpriteExtra::create(
                                visibleBtnSprite,
                                this,
                                menu_selector(MyPlayLayer::onToggleViewTimer)
                            );
                            m_fields->m_visibleBtn->setID("visible-button");

                            visibleMenu->addChild(m_fields->m_visibleBtn);
                            visibleMenu->updateLayout(true);

                            visibleMenu->setScale(mobileScale);

                            addChild(visibleMenu);
                        } else {
                            log::warn("Mobile UI visibility button is disabled");
                        };
                    } else {
                        log::warn("Mobile controls are disabled");
                    };

#ifndef GEODE_IS_IOS
                    // remove timer
                    this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
                        if (event->isDown()) {
                            toggleTimerVisibility();

                            log::warn("Speedrun timer view toggled by keybind");
                        };

                        return ListenerResult::Propagate;
                                                                      }, "hide-timer"_spr);
#endif

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

    void toggleTimerVisibility() {
        if (m_fields->m_speedrunNode) m_fields->m_speedrunNode->setVisible(!m_fields->m_speedrunNode->isVisible());
        if (m_fields->m_mobileMenu) m_fields->m_mobileMenu->setVisible(!m_fields->m_mobileMenu->isVisible());
    };

    void onToggleViewTimer(CCObject*) {
        toggleTimerVisibility();

        if (m_fields->m_speedrunNode) {
            auto visible = m_fields->m_speedrunNode->isVisible();

            if (m_fields->m_visibleBtn) m_fields->m_visibleBtn->setOpacity(visible ? 255 : 125);
            log::warn("Speedrun timer view toggled {} by UI", visible ? "on" : "off");
        } else {
            log::error("Couldn't find speedrun timer");
        };
    };

    void pauseTimer(CCObject*) {
        if (m_fields->m_speedrunNode) {
            m_fields->m_manualPause = !m_fields->m_pauseTimerBtn->isToggled();
            if (m_fields->m_pauseTimerBtn) m_fields->m_speedrunNode->pauseTimer(m_fields->m_manualPause);
        } else {
            log::error("Failed to get speedrun node");
        };
    };

    void createSplit(CCObject*) {
        if (m_fields->m_speedrunNode) m_fields->m_speedrunNode->createSplit();
    };

    void resetAll(CCObject*) {
        if (m_fields->m_speedrunNode) {
            m_fields->m_speedrunNode->resetAll();
            if (m_fields->m_pauseTimerBtn) m_fields->m_pauseTimerBtn->toggle(m_fields->m_speedrunNode->isTimerPaused()); // update the button state
        } else {
            log::error("Failed to get speedrun node");
        };
    };

    void destroyPlayer(PlayerObject * p0, GameObject * p1) {
        bool wasDead = p0 ? p0->m_isDead : true;

        PlayLayer::destroyPlayer(p0, p1);

        if (srt->getSettingValue<bool>("reset-death")) { // check if reset after death is enabled
            if (!wasDead && p0->m_isDead) {
                if (m_fields->m_speedrunNode) {
                    log::info("Pausing timer");

                    m_fields->m_speedrunNode->pauseTimer(true);
                    if (m_fields->m_pauseTimerBtn) m_fields->m_pauseTimerBtn->toggle(m_fields->m_speedrunNode->isTimerPaused()); // update the button state
                } else {
                    log::error("Failed to get speedrun node");
                };
            } else {
                log::warn("Player hit anti-cheat object");
            };
        };
    };

    void resetLevel() {
        if (srt->getSettingValue<bool>("reset-death")) { // check if reset after death is enabled
            log::info("Resuming timer");

            if (m_fields->m_speedrunNode) m_fields->m_speedrunNode->pauseTimer(m_fields->m_manualPause);
            if (m_fields->m_pauseTimerBtn) m_fields->m_pauseTimerBtn->toggle(m_fields->m_speedrunNode->isTimerPaused()); // update the button state
        } else {
            log::info("Timer will not resume on restart");
        };

        PlayLayer::resetLevel();
    };

    void resetLevelFromStart() {
        m_fields->m_checkpointObject = nullptr; // reset the checkpoint object

        if (srt->getSettingValue<bool>("reset-death")) { // check if reset after death is enabled
            log::info("Resetting timer");

            if (m_fields->m_speedrunNode) m_fields->m_manualPause ? m_fields->m_speedrunNode->pauseTimer(m_fields->m_manualPause) : m_fields->m_speedrunNode->resetAll();
            if (m_fields->m_pauseTimerBtn) m_fields->m_pauseTimerBtn->toggle(m_fields->m_speedrunNode->isTimerPaused()); // update the button state
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
            if (m_fields->m_pauseTimerBtn) m_fields->m_pauseTimerBtn->toggle(m_fields->m_speedrunNode->isTimerPaused()); // update the button state
        } else {
            log::info("Timer will not resume on checkpoint activation");
        };

        PlayLayer::checkpointActivated(p0);
    };

    void levelComplete() {
        if (srt->getSettingValue<bool>("stop-complete")) { // check if stop on level complete is enabled
            if (m_fields->m_speedrunNode) m_fields->m_speedrunNode->toggleTimer(false);
            if (m_fields->m_pauseTimerBtn) m_fields->m_pauseTimerBtn->toggle(!m_fields->m_speedrunNode->isTimerPaused()); // update the button state
        } else {
            log::info("Timer will not stop on level completion");
        };

        PlayLayer::levelComplete();
    };
};