#pragma once

#define _AMD64_

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#define EXTERN_API extern "C" _declspec(dllexport)

// Function typedef
#define __typedef_func(func, ret, ...) using func##_t = ret(WINAPI*)(##__VA_ARGS__); func##_t m_##func = nullptr;

// Resolve the specified function proc
#define __setup_proc(func, proxy) m_##func = reinterpret_cast<func##_t>(::GetProcAddress(##proxy, #func));

// Set the proc back to nullptr
#define __clear_proc(func) if(m_##func) m_##func = nullptr;

// Windows Header Files
#include <minwindef.h>
#include <sysinfoapi.h>
#include <winternl.h>
#include <processthreadsapi.h>
#include <memoryapi.h>
#include <synchapi.h>
#include <WinUser.h>
#include <Unknwn.h>
#include <filesystem>
#include <Xinput.h>

// WinAPI stub
#include <stub.h>

// Button state enum
#include <button_state.h>
