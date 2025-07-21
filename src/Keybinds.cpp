#include <Geode/Geode.hpp>
#include <Geode/DefaultInclude.hpp>

#include <Geode/utils/cocos.hpp>

#include <geode.custom-keybinds/include/Keybinds.hpp>

using namespace geode::prelude;
using namespace keybinds;

$execute
{
    BindManager::get()->registerBindable({
        "toggle-timer"_spr,
        "Timer", "Toggle the speedrun timer.",
        { Keybind::create(KEY_NumPad1, Modifier::None)},
        "Speedrun/Tools"
    });
};