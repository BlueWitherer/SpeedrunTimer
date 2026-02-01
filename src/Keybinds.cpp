// #include <Geode/Geode.hpp>

// #include <Geode/utils/cocos.hpp>

// #include <geode.custom-keybinds/include/Keybinds.hpp>

// using namespace geode::prelude;
// using namespace keybinds;

// $execute{
//     if (auto bm = BindManager::get()) {
//         bm->registerBindable({
//             "pause-timer"_spr,
//             "Pause Timer", "Pause or resume the speedrun timer.",
//             {
//                 Keybind::create(KEY_NumPad1, Modifier::None),
//             },
//             "Speedrun",
//                              });

//         bm->registerBindable({
//             "split-timer"_spr,
//             "Split Timer", "Create a timer split.",
//             {
//                 Keybind::create(KEY_NumPad2, Modifier::None),
//             },
//             "Speedrun",
//                              });

//         bm->registerBindable({
//             "reset-timer"_spr,
//             "Reset Timer", "Reset the speedrun timer and all splits.",
//             {
//                 Keybind::create(KEY_NumPad3, Modifier::None),
//             },
//             "Speedrun",
//                              });

//         bm->registerBindable({
//             "hide-timer"_spr,
//             "Hide Timer", "Turn the speedrun timer invisible. It will continue to run in the background unless paused or reset manually.",
//             {
//                 Keybind::create(KEY_Delete, Modifier::None),
//             },
//             "Speedrun",
//                              });
//     } else {
//         log::error("Failed to get keybind manager");
//     };
// };