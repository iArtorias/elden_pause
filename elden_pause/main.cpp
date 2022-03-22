#include "pch.h"

// Various
#include <misc.h>

// mem (https://github.com/0x1F9F1/mem)
#include <mem/pattern.h>

// inireader (https://github.com/X-rays5/inireader)
#include <inireader/inireader.hpp>

// The current patch thread state
volatile bool m_ThreadRunning{ true };

// The flag indicating that the button was pressed
volatile bool m_WasPressed{ false };

// The default button state
EButtonState m_CurrentState{ EButtonState::k_Deactivated };


// Find the required address by the given pattern
inline LPVOID find_address( const char* bytes, const uint8_t* data, size_t length )
{
    mem::pattern pattern( bytes );
    mem::simd_scanner scanner( pattern );

    LPVOID result{};

    scanner( { data, length }, [&] ( mem::pointer res )
    {
        result = res.as<LPVOID>();
        return false;
    } );

    return result;
}


// Patch the memory
inline void patch( const LPVOID address, const uint8_t* patch, const size_t size )
{
    try
    {
        DWORD old_protect{};
        VirtualProtect( address, size, PAGE_EXECUTE_READWRITE, &old_protect );
        memcpy( address, reinterpret_cast<const VOID*>(patch), size );
        VirtualProtect( address, size, old_protect, &old_protect );
    }
    catch (...)
    {
        return;
    }
}


// Read the current configuration file
inline void read_config( const fs::path& cfg )
{
    ini::Parser ini;

    try
    {
        std::ifstream config_in( cfg, std::ios::in );

        if (config_in.is_open())
        {
            ini.Parse( config_in );

            if (ini[config::SECTION_SETTINGS].HasValue( config::KEY_KEYBOARD_BUTTON ))
            {
                auto const button_keyboard = ini[config::SECTION_SETTINGS][config::KEY_KEYBOARD_BUTTON].as<int32_t>();
                config::OPTION_KEYBOARD_BUTTON = button_keyboard;
            }

            if (ini[config::SECTION_SETTINGS].HasValue( config::KEY_CONTROLLER_ENABLED ))
            {
                std::string controller_enabled = ini[config::SECTION_SETTINGS][config::KEY_CONTROLLER_ENABLED].as<std::string>();
                config::OPTION_CONTROLLER_ENABLED = controller_enabled;
            }

            if (config::OPTION_CONTROLLER_ENABLED == "true" || config::OPTION_CONTROLLER_ENABLED == "1")
            {
                if (ini[config::SECTION_SETTINGS].HasValue( config::KEY_CONTROLLER_BUTTON ))
                {
                    auto const button_controller = ini[config::SECTION_SETTINGS][config::KEY_CONTROLLER_BUTTON].as<int32_t>();
                    config::OPTION_CONTROLLER_BUTTON = button_controller;
                }
                if (ini[config::SECTION_SETTINGS].HasValue( config::KEY_CONTROLLER_BUTTON2 ))
                {
                    auto const button_controller2 = ini[config::SECTION_SETTINGS][config::KEY_CONTROLLER_BUTTON2].as<int32_t>();
                    config::OPTION_CONTROLLER_BUTTON2 = button_controller2;
                }
            }
        }
    }
    catch (std::runtime_error&)
    {
        return;
    }
}


// Update the current button state
void update_state( const LPVOID address )
{
    switch (m_CurrentState)
    {
        case EButtonState::k_Activated:
        {
            uint8_t data[]{ 0x0F, 0x84 }; // je
            patch( address, data, 0x2 );

            m_CurrentState = EButtonState::k_Deactivated;
            break;
        }

        case EButtonState::k_Deactivated:
        {
            uint8_t data[]{ 0x0F, 0x85 }; // jne
            patch( address, data, 0x2 );

            m_CurrentState = EButtonState::k_Activated;
            break;
        }
    }
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            auto result = CProxyStub::instance().resolve( hModule );

            if (result)
            {
                // Ge the current module base
                auto const base = reinterpret_cast<uint64_t>(GetModuleHandleW( nullptr ));

                // First section header structure
                auto const section = IMAGE_FIRST_SECTION( reinterpret_cast<PIMAGE_NT_HEADERS>(base + reinterpret_cast<PIMAGE_DOS_HEADER>(base)->e_lfanew) );

                // First section start address (equivalent to ".text")
                auto const start = reinterpret_cast<uint8_t*>(base + section->VirtualAddress);

                // The virtual size of that section
                auto const size = section->Misc.VirtualSize;

                // Read the data from configuration file
                read_config( CProxyStub::instance().get_proxy_path() / "elden_pause.ini" );

                /*
                    je eldenring.XXXXXXXXXXXX
                    mov byte ptr ds:[rbx+XXX], 0
                    lea rsi, qword ptr ds:[XXXXXXXXXXXX]
                    mov qword ptr ss:[rbp-XX], rsi
                    mov dword ptr ss:[rbp-XX], edi
                */
                static auto const address = find_address( "0F 84 ? ? ? ? C6 83 ? ? 00 00 00 48 8D ? ? ? ? ? 48 89 ? ? 89", start, size );

                if (address != nullptr)
                {
                    // Run keyboard specific thread
                    auto keyboard_thread = [] ( LPVOID data ) -> DWORD
                    {
                        while (m_ThreadRunning)
                        {
                            if ((GetAsyncKeyState( config::OPTION_KEYBOARD_BUTTON ) & 0x01))
                            {
                                update_state( address );
                            }
                        }

                        return 0x0;
                    };

                    CreateThread( nullptr, 0x0, keyboard_thread, nullptr, 0x0, nullptr );


                    // Proceed in case the controller option is enabled
                    if (config::OPTION_CONTROLLER_ENABLED == "true" || config::OPTION_CONTROLLER_ENABLED == "1")
                    {
                        // Run controller specific thread
                        auto controller_thread = [] ( LPVOID data ) -> DWORD
                        {
                            while (m_ThreadRunning)
                            {
                                // The number of detected devices
                                int32_t devices_num{ 0x0 };

                                for (DWORD i{ 0x0 }; i < XUSER_MAX_COUNT; i++)
                                {
                                    XINPUT_STATE state{};
                                    ZeroMemory( &state, sizeof( XINPUT_STATE ) );

                                    // Get the current state of the controller.
                                    auto const result = ::XInputGetState( i, &state );

                                    if (result == 0x0) // ERROR_SUCCESS
                                    {
                                        bool key_pressed = ((state.Gamepad.wButtons & config::OPTION_CONTROLLER_BUTTON) != 0);
                                        bool modifier_pressed = ((state.Gamepad.wButtons & config::OPTION_CONTROLLER_BUTTON2) != 0);

                                        if (key_pressed && modifier_pressed && !m_WasPressed) // Check if both buttons are pressed and if buttons are not held down
                                        {
                                            m_WasPressed = true;
                                            update_state( address );
                                        }
                                        else if (!key_pressed || !modifier_pressed && m_WasPressed) // Check if button was released
                                        {
                                            m_WasPressed = false;
                                        }
                                    }
                                }

                                ::Sleep( (devices_num > 0x0) ? xinput::SLEEP_ACTIVE : xinput::SLEEP_INACTIVE );
                            }

                            return 0x0;
                        };

                        CreateThread( nullptr, 0x0, controller_thread, nullptr, 0x0, nullptr );
                    }
                }
            }

            break;
        }

        case DLL_PROCESS_DETACH:
        {
            m_ThreadRunning = false;
            break;
        }
    }

    return TRUE;
};