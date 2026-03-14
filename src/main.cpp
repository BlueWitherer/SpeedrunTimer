#include "./headers/RunTimer.hpp"

#include <Geode/Geode.hpp>

#include <Geode/utils/Keyboard.hpp>

#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// it's modding time :3
static auto srt = Mod::get();

class $modify(SpeedrunPlayLayer, PlayLayer) {
    struct Fields {
        bool enabled = srt->getSettingValue<bool>("enabled");
        bool platformerOnly = srt->getSettingValue<bool>("platformer-only");

        WeakRef<CheckpointGameObject> checkpointObject = nullptr;  // Latest checkpoint

        RunTimer* runTimer = nullptr;  // The speedrun node for the timer

        CCMenu* mobileMenu = nullptr;  // Mobile controls menu

        CCMenuItemToggler* pauseTimerBtn = nullptr;      // Pause timer button for mobile controls
        CCMenuItemSpriteExtra* splitTimerBtn = nullptr;  // Split timer button for mobile controls
        CCMenuItemSpriteExtra* resetTimerBtn = nullptr;  // Reset timer button for mobile controls

        CCMenuItemSpriteExtra* visibleBtn = nullptr;

        bool manualPause = true;
    };

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();

        auto f = m_fields.self();

        if (f->enabled && f->platformerOnly ? m_level->isPlatformer() : true) {
            log::trace("Creating speedrun timer...");

            // create speedrun timer label
            if (auto timer = RunTimer::create()) {
                auto const size = m_uiLayer->getScaledContentSize();

                timer->setAnchorPoint({1, 1});
                timer->setPosition({size.width - 25.f, size.height - 25.f});

                timer->toggleTimer(true);  // enable the timer

                f->runTimer = timer;

                m_uiLayer->addChild(f->runTimer, 99);

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
                    f->mobileMenu = CCMenu::create();
                    f->mobileMenu->setID("mobile-controls"_spr);
                    f->mobileMenu->setAnchorPoint({0, 1});
                    f->mobileMenu->setPosition({25.f, size.height - 25.f});
                    f->mobileMenu->setLayout(btnMenuLayout);

                    auto pauseTimerBtnSpriteOn = CCSprite::createWithSpriteFrameName("GJ_stopEditorBtn_001.png");
                    pauseTimerBtnSpriteOn->setScale(0.8f);
                    pauseTimerBtnSpriteOn->setOpacity(opacity);

                    auto pauseTimerBtnSpriteOff = CCSprite::createWithSpriteFrameName("GJ_playEditorBtn_001.png");
                    pauseTimerBtnSpriteOff->setScale(0.8f);
                    pauseTimerBtnSpriteOff->setOpacity(opacity);

                    // create the pause button
                    f->pauseTimerBtn = CCMenuItemToggler::create(
                        pauseTimerBtnSpriteOn,
                        pauseTimerBtnSpriteOff,
                        this,
                        menu_selector(SpeedrunPlayLayer::pauseTimer));
                    f->pauseTimerBtn->setID("pause-timer-btn");

                    auto splitTimerBtnSprite = CCSprite::createWithSpriteFrameName("GJ_practiceBtn_001.png");
                    splitTimerBtnSprite->setScale(0.5f);
                    splitTimerBtnSprite->setOpacity(opacity);

                    // create the split button
                    f->splitTimerBtn = CCMenuItemSpriteExtra::create(
                        splitTimerBtnSprite,
                        this,
                        menu_selector(SpeedrunPlayLayer::createSplit));
                    f->splitTimerBtn->setID("split-run-btn");

                    auto resetTimerBtnSprite = CCSprite::createWithSpriteFrameName("GJ_replayBtn_001.png");
                    resetTimerBtnSprite->setScale(0.5f);
                    resetTimerBtnSprite->setOpacity(opacity);

                    // create the reset button
                    f->resetTimerBtn = CCMenuItemSpriteExtra::create(
                        resetTimerBtnSprite,
                        this,
                        menu_selector(SpeedrunPlayLayer::resetAll));
                    f->resetTimerBtn->setID("reset-time-btn");

                    f->pauseTimerBtn->toggle(f->runTimer->isTimerPaused());  // set the initial state of the pause button

                    f->mobileMenu->addChild(f->pauseTimerBtn);
                    f->mobileMenu->addChild(f->splitTimerBtn);
                    f->mobileMenu->addChild(f->resetTimerBtn);

                    auto mobileScale = static_cast<float>(srt->getSettingValue<double>("mobile-btns-scale"));

                    f->mobileMenu->setScale(mobileScale);

                    m_uiLayer->addChild(f->mobileMenu, 99);

                    f->mobileMenu->updateLayout();

                    if (srt->getSettingValue<bool>("mobile-visible-btn")) {
                        auto visibleMenu = CCMenu::create();
                        visibleMenu->setID("mobile-visibility-menu"_spr);
                        visibleMenu->setAnchorPoint(f->mobileMenu->getAnchorPoint());
                        visibleMenu->setPosition({f->mobileMenu->getPositionX(), f->mobileMenu->getPositionY() - f->mobileMenu->getScaledContentHeight() - 2.5f});
                        visibleMenu->setLayout(btnMenuLayout);

                        auto visibleBtnSprite = CCSprite::createWithSpriteFrameName("hideBtn_001.png");
                        visibleBtnSprite->setScale(0.875f);
                        visibleBtnSprite->setOpacity(200);

                        f->visibleBtn = CCMenuItemSpriteExtra::create(
                            visibleBtnSprite,
                            this,
                            menu_selector(SpeedrunPlayLayer::onToggleViewTimer));
                        f->visibleBtn->setID("toggle-interface-btn");

                        visibleMenu->addChild(f->visibleBtn);
                        visibleMenu->updateLayout();

                        visibleMenu->setScale(mobileScale);

                        m_uiLayer->addChild(visibleMenu, 99);
                    } else {
                        log::warn("Mobile UI visibility button is disabled");
                    };
                } else {
                    log::warn("Mobile controls are disabled");
                };

                addEventListener(
                    KeybindSettingPressedEventV3(Mod::get(), "key-hide"),
                    [this](Keybind const&, bool down, bool repeat, double) {
                        if (down && !repeat) {
                            toggleTimerVisibility();
                            log::warn("Speedrun timer view toggled by keybind");
                        };
                    });

                log::info("Speedrun timer is ready!");
            } else {
                log::error("Failed to create timer!");
            };
        } else {
            log::error("Level is not a platformer");
        };
    };

    void toggleTimerVisibility() {
        auto f = m_fields.self();

        if (f->runTimer) f->runTimer->setVisible(!f->runTimer->isVisible());
        if (f->mobileMenu) f->mobileMenu->setVisible(!f->mobileMenu->isVisible());
    };

    void onToggleViewTimer(CCObject*) {
        toggleTimerVisibility();

        auto f = m_fields.self();

        if (f->runTimer) {
            auto visible = f->runTimer->isVisible();

            if (f->visibleBtn) f->visibleBtn->setOpacity(visible ? 255 : 125);
            log::warn("Speedrun timer view toggled {} by UI", visible ? "on" : "off");
        } else {
            log::error("Couldn't find speedrun timer");
        };
    };

    void pauseTimer(CCObject*) {
        auto f = m_fields.self();

        if (f->runTimer) {
            f->manualPause = !f->pauseTimerBtn->isToggled();
            if (f->pauseTimerBtn) f->runTimer->pauseTimer(f->manualPause);
        } else {
            log::error("Failed to get speedrun node");
        };
    };

    void createSplit(CCObject*) {
        auto f = m_fields.self();
        if (f->runTimer) f->runTimer->createSplit();
    };

    void resetAll(CCObject*) {
        auto f = m_fields.self();

        if (f->runTimer) {
            f->runTimer->resetAll();
            if (f->pauseTimerBtn) f->pauseTimerBtn->toggle(f->runTimer->isTimerPaused());  // update the button state
        } else {
            log::error("Failed to get speedrun node");
        };
    };

    void destroyPlayer(PlayerObject* p0, GameObject* p1) {
        PlayLayer::destroyPlayer(p0, p1);

        if (srt->getSettingValue<bool>("reset-death")) {  // check if reset after death is enabled
            if (p0->m_isDead) {
                auto f = m_fields.self();

                if (f->runTimer) {
                    log::info("Pausing timer");

                    f->runTimer->pauseTimer();
                    if (f->pauseTimerBtn) f->pauseTimerBtn->toggle(f->runTimer->isTimerPaused());  // update the button state
                };
            };
        };
    };

    void resetLevel() {
        if (srt->getSettingValue<bool>("reset-death")) {  // check if reset after death is enabled
            log::info("Resuming timer");

            auto f = m_fields.self();

            if (f->runTimer) f->runTimer->pauseTimer(f->manualPause);
            if (f->pauseTimerBtn) f->pauseTimerBtn->toggle(f->runTimer->isTimerPaused());  // update the button state
        } else {
            log::info("Timer will not resume on restart");
        };

        PlayLayer::resetLevel();
    };

    void resetLevelFromStart() {
        auto f = m_fields.self();

        f->checkpointObject = nullptr;  // reset the checkpoint object

        if (srt->getSettingValue<bool>("reset-death")) {  // check if reset after death is enabled
            log::info("Resetting timer");

            if (f->runTimer) f->manualPause ? f->runTimer->pauseTimer(f->manualPause) : f->runTimer->resetAll();
            if (f->pauseTimerBtn) f->pauseTimerBtn->toggle(f->runTimer->isTimerPaused());  // update the button state
        } else {
            log::info("Timer will not reset on level restart");
        };

        PlayLayer::resetLevelFromStart();
    };

    void checkpointActivated(CheckpointGameObject* p0) {
        auto f = m_fields.self();

        if (srt->getSettingValue<bool>("checkpoint-split")) {  // check if split on checkpoint is enabled
            if (auto checkpoint = f->checkpointObject.lock()) {
                if (p0 == checkpoint) {
                    log::warn("Checkpoint split is already active");
                } else {
                    if (f->runTimer) f->runTimer->createSplit();
                };

                f->checkpointObject = p0;
            };
        } else {
            log::warn("Checkpoint split is disabled");
        };

        if (srt->getSettingValue<bool>("reset-death")) {                                   // check if reset after death is enabled
            if (f->runTimer) f->runTimer->pauseTimer(false);                               // force resume
            if (f->pauseTimerBtn) f->pauseTimerBtn->toggle(f->runTimer->isTimerPaused());  // update the button state
        } else {
            log::info("Timer will not resume on checkpoint activation");
        };

        PlayLayer::checkpointActivated(p0);
    };

    void levelComplete() {
        if (srt->getSettingValue<bool>("stop-complete")) {  // check if stop on level complete is enabled
            auto f = m_fields.self();

            if (f->runTimer) f->runTimer->toggleTimer();
            if (f->pauseTimerBtn) f->pauseTimerBtn->toggle(!f->runTimer->isTimerPaused());  // update the button state
        } else {
            log::info("Timer will not stop on level completion");
        };

        PlayLayer::levelComplete();
    };
};