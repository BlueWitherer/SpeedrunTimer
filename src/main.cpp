#include "./headers/RunTimer.hpp"

#include <Geode/Geode.hpp>

#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

#ifndef GEODE_IS_IOS
#include <geode.custom-keybinds/include/Keybinds.hpp>
using namespace keybinds;
#endif

// it's modding time :3
static auto srt = Mod::get();

class $modify(SpeedrunPlayLayer, PlayLayer) {
    struct Fields {
        bool enabled = srt->getSettingValue<bool>("enabled");
        bool platformerOnly = srt->getSettingValue<bool>("platformer-only");

        Ref<CheckpointGameObject> m_checkpointObject = nullptr; // Latest checkpoint

        RunTimer* m_runTimer = nullptr; // The speedrun node for the timer

        CCMenu* m_mobileMenu = nullptr; // Mobile controls menu

        CCMenuItemToggler* m_pauseTimerBtn = nullptr; // Pause timer button for mobile controls
        CCMenuItemSpriteExtra* m_splitTimerBtn = nullptr; // Split timer button for mobile controls
        CCMenuItemSpriteExtra* m_resetTimerBtn = nullptr; // Reset timer button for mobile controls

        CCMenuItemSpriteExtra* m_visibleBtn = nullptr;

        bool m_manualPause = true;

        bool m_draggingMobile = false; // If the player is dragging the mobile menu
    };

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();

        auto f = m_fields.self();

        if (f->enabled) if (f->platformerOnly ? m_level->isPlatformer() : true) {
            log::debug("Creating speedrun timer...");

            // create speedrun timer label
            if (auto timer = RunTimer::create()) {
                auto const [widthCS, heightCS] = m_uiLayer->getScaledContentSize();

                timer->setAnchorPoint({ 1, 1 });
                timer->setPosition({ widthCS - 25.f, heightCS - 25.f });

                timer->toggleTimer(true); // enable the timer

                f->m_runTimer = timer;

                m_uiLayer->addChild(f->m_runTimer, 99);

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

                    auto opacity = srt->getSettingValue<int64_t>("mobile-btns-opacity");

                    // menu for mobile controls
                    f->m_mobileMenu = CCMenu::create();
                    f->m_mobileMenu->setID("mobile-controls"_spr);
                    f->m_mobileMenu->setAnchorPoint({ 0, 1 });
                    f->m_mobileMenu->setPosition({ 25.f, heightCS - 25.f });
                    f->m_mobileMenu->setLayout(btnMenuLayout);

                    auto pauseTimerBtnSpriteOn = CCSprite::createWithSpriteFrameName("GJ_stopEditorBtn_001.png");
                    pauseTimerBtnSpriteOn->setScale(0.8f);
                    pauseTimerBtnSpriteOn->setOpacity(opacity);

                    auto pauseTimerBtnSpriteOff = CCSprite::createWithSpriteFrameName("GJ_playEditorBtn_001.png");
                    pauseTimerBtnSpriteOff->setScale(0.8f);
                    pauseTimerBtnSpriteOff->setOpacity(opacity);

                    // create the pause button
                    f->m_pauseTimerBtn = CCMenuItemToggler::create(
                        pauseTimerBtnSpriteOn,
                        pauseTimerBtnSpriteOff,
                        this,
                        menu_selector(SpeedrunPlayLayer::pauseTimer)
                    );
                    f->m_pauseTimerBtn->setID("pause-timer-btn");

                    auto splitTimerBtnSprite = CCSprite::createWithSpriteFrameName("GJ_practiceBtn_001.png");
                    splitTimerBtnSprite->setScale(0.5f);
                    splitTimerBtnSprite->setOpacity(opacity);

                    // create the split button
                    f->m_splitTimerBtn = CCMenuItemSpriteExtra::create(
                        splitTimerBtnSprite,
                        this,
                        menu_selector(SpeedrunPlayLayer::createSplit)
                    );
                    f->m_splitTimerBtn->setID("split-run-btn");

                    auto resetTimerBtnSprite = CCSprite::createWithSpriteFrameName("GJ_replayBtn_001.png");
                    resetTimerBtnSprite->setScale(0.5f);
                    resetTimerBtnSprite->setOpacity(opacity);

                    // create the reset button
                    f->m_resetTimerBtn = CCMenuItemSpriteExtra::create(
                        resetTimerBtnSprite,
                        this,
                        menu_selector(SpeedrunPlayLayer::resetAll)
                    );
                    f->m_resetTimerBtn->setID("reset-time-btn");

                    f->m_pauseTimerBtn->toggle(f->m_runTimer->isTimerPaused()); // set the initial state of the pause button

                    f->m_mobileMenu->addChild(f->m_pauseTimerBtn);
                    f->m_mobileMenu->addChild(f->m_splitTimerBtn);
                    f->m_mobileMenu->addChild(f->m_resetTimerBtn);

                    auto mobileScale = static_cast<float>(srt->getSettingValue<double>("mobile-btns-scale"));

                    f->m_mobileMenu->setScale(mobileScale);

                    m_uiLayer->addChild(f->m_mobileMenu, 99);

                    f->m_mobileMenu->updateLayout();

                    if (srt->getSettingValue<bool>("mobile-visible-btn")) {
                        auto visibleMenu = CCMenu::create();
                        visibleMenu->setID("mobile-visibility-menu"_spr);
                        visibleMenu->setAnchorPoint(f->m_mobileMenu->getAnchorPoint());
                        visibleMenu->setPosition({ f->m_mobileMenu->getPositionX(), f->m_mobileMenu->getPositionY() - f->m_mobileMenu->getScaledContentHeight() - 2.5f });
                        visibleMenu->setLayout(btnMenuLayout);

                        auto visibleBtnSprite = CCSprite::createWithSpriteFrameName("hideBtn_001.png");
                        visibleBtnSprite->setScale(0.875f);
                        visibleBtnSprite->setOpacity(200);

                        f->m_visibleBtn = CCMenuItemSpriteExtra::create(
                            visibleBtnSprite,
                            this,
                            menu_selector(SpeedrunPlayLayer::onToggleViewTimer)
                        );
                        f->m_visibleBtn->setID("toggle-interface-btn");

                        visibleMenu->addChild(f->m_visibleBtn);
                        visibleMenu->updateLayout();

                        visibleMenu->setScale(mobileScale);

                        m_uiLayer->addChild(visibleMenu, 99);
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
                                                                  },
                                                                  "hide-timer"_spr);
#endif

                log::info("Speedrun timer created!");
            } else {
                log::error("Failed to create timer!");
            };
        } else {
            log::error("Level is not a platformer");
        };
    };

    void toggleTimerVisibility() {
        auto f = m_fields.self();

        if (f->m_runTimer) f->m_runTimer->setVisible(!f->m_runTimer->isVisible());
        if (f->m_mobileMenu) f->m_mobileMenu->setVisible(!f->m_mobileMenu->isVisible());
    };

    void onToggleViewTimer(CCObject*) {
        toggleTimerVisibility();

        auto f = m_fields.self();

        if (f->m_runTimer) {
            auto visible = f->m_runTimer->isVisible();

            if (f->m_visibleBtn) f->m_visibleBtn->setOpacity(visible ? 255 : 125);
            log::warn("Speedrun timer view toggled {} by UI", visible ? "on" : "off");
        } else {
            log::error("Couldn't find speedrun timer");
        };
    };

    void pauseTimer(CCObject*) {
        auto f = m_fields.self();

        if (f->m_runTimer) {
            f->m_manualPause = !f->m_pauseTimerBtn->isToggled();
            if (f->m_pauseTimerBtn) f->m_runTimer->pauseTimer(f->m_manualPause);
        } else {
            log::error("Failed to get speedrun node");
        };
    };

    void createSplit(CCObject*) {
        auto f = m_fields.self();
        if (f->m_runTimer) f->m_runTimer->createSplit();
    };

    void resetAll(CCObject*) {
        auto f = m_fields.self();

        if (f->m_runTimer) {
            f->m_runTimer->resetAll();
            if (f->m_pauseTimerBtn) f->m_pauseTimerBtn->toggle(f->m_runTimer->isTimerPaused()); // update the button state
        } else {
            log::error("Failed to get speedrun node");
        };
    };

    void destroyPlayer(PlayerObject * p0, GameObject * p1) {
        bool wasDead = p0 ? p0->m_isDead : true;

        PlayLayer::destroyPlayer(p0, p1);

        if (srt->getSettingValue<bool>("reset-death")) { // check if reset after death is enabled
            if (!wasDead && p0->m_isDead) {
                auto f = m_fields.self();

                if (f->m_runTimer) {
                    log::info("Pausing timer");

                    f->m_runTimer->pauseTimer(true);
                    if (f->m_pauseTimerBtn) f->m_pauseTimerBtn->toggle(f->m_runTimer->isTimerPaused()); // update the button state
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

            auto f = m_fields.self();

            if (f->m_runTimer) f->m_runTimer->pauseTimer(f->m_manualPause);
            if (f->m_pauseTimerBtn) f->m_pauseTimerBtn->toggle(f->m_runTimer->isTimerPaused()); // update the button state
        } else {
            log::info("Timer will not resume on restart");
        };

        PlayLayer::resetLevel();
    };

    void resetLevelFromStart() {
        auto f = m_fields.self();

        f->m_checkpointObject = nullptr; // reset the checkpoint object

        if (srt->getSettingValue<bool>("reset-death")) { // check if reset after death is enabled
            log::info("Resetting timer");

            if (f->m_runTimer) f->m_manualPause ? f->m_runTimer->pauseTimer(f->m_manualPause) : f->m_runTimer->resetAll();
            if (f->m_pauseTimerBtn) f->m_pauseTimerBtn->toggle(f->m_runTimer->isTimerPaused()); // update the button state
        } else {
            log::info("Timer will not reset on level restart");
        };

        PlayLayer::resetLevelFromStart();
    };

    void checkpointActivated(CheckpointGameObject * p0) {
        auto f = m_fields.self();

        if (srt->getSettingValue<bool>("checkpoint-split")) { // check if split on checkpoint is enabled
            if (p0 == f->m_checkpointObject) {
                log::warn("Checkpoint split is already active");
            } else {
                if (f->m_runTimer) f->m_runTimer->createSplit();
            };

            f->m_checkpointObject = p0;
        } else {
            log::warn("Checkpoint split is disabled");
        };

        if (srt->getSettingValue<bool>("reset-death")) { // check if reset after death is enabled
            if (f->m_runTimer) f->m_runTimer->pauseTimer(false); // force resume
            if (f->m_pauseTimerBtn) f->m_pauseTimerBtn->toggle(f->m_runTimer->isTimerPaused()); // update the button state
        } else {
            log::info("Timer will not resume on checkpoint activation");
        };

        PlayLayer::checkpointActivated(p0);
    };

    void levelComplete() {
        if (srt->getSettingValue<bool>("stop-complete")) { // check if stop on level complete is enabled
            auto f = m_fields.self();

            if (f->m_runTimer) f->m_runTimer->toggleTimer(false);
            if (f->m_pauseTimerBtn) f->m_pauseTimerBtn->toggle(!f->m_runTimer->isTimerPaused()); // update the button state
        } else {
            log::info("Timer will not stop on level completion");
        };

        PlayLayer::levelComplete();
    };
};