#include "pch.h"

// mem (https://github.com/0x1F9F1/mem)
#include <mem/pattern.h>

// The current patch thread state
volatile bool m_ThreadRunning{ true };

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
                    auto work = [] ( LPVOID data ) -> DWORD
                    {
                        while (m_ThreadRunning)
                        {
                            if ((GetAsyncKeyState( 0x50 ) & 0x01)) // "P" button
                            {
                                update_state( address );
                            }
                        }

                        return 0x0;
                    };

                    CreateThread( nullptr, 0x0, work, nullptr, 0x0, nullptr );
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