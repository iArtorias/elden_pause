#pragma once

#define XINPUT_GAMEPAD_NONE 0xFFFF

// Xinput specific
namespace xinput
{
    const DWORD SLEEP_ACTIVE{ 10 }; // 10ms
    const DWORD SLEEP_INACTIVE{ 100 }; // 100ms
}

// Configuration options
namespace config
{
    const std::string SECTION_SETTINGS{ "Settings" };
    const std::string KEY_CONTROLLER_ENABLED{ "ControllerEnabled" };
    const std::string KEY_KEYBOARD_BUTTON{ "KeyboardButton" };
    const std::string KEY_CONTROLLER_BUTTON{ "ControllerButton" };
    const std::string KEY_CONTROLLER_BUTTON2{ "ControllerButton2" };
    const std::string KEY_PAUSE_WHEN_UNFOCUSED{ "PauseWhenUnfocused" };
    int32_t OPTION_KEYBOARD_BUTTON{ 0x50 }; // "P" button by default
    int32_t OPTION_CONTROLLER_BUTTON{ XINPUT_GAMEPAD_START }; // "Start" button by default
    int32_t OPTION_CONTROLLER_BUTTON2{ XINPUT_GAMEPAD_NONE }; // Disabled by default
    bool OPTION_CONTROLLER_ENABLED{ true }; // Disable or enable the usage of controller code. Enabled by default
    bool OPTION_PAUSE_WHEN_UNFOCUSED{ false }; // Pause the game while unfocused. Disabled by default.
}