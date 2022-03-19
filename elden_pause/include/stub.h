#pragma once

namespace fs = std::filesystem;

typedef struct DIOBJECTDATAFORMAT
{
    CONST GUID* pguid;
    DWORD dwOfs;
    DWORD dwType;
    DWORD dwFlags;
} DIOBJECTDATAFORMAT, * LPDIOBJECTDATAFORMAT;

typedef struct DIDATAFORMAT
{
    DWORD dwSize;
    DWORD dwObjSize;
    DWORD dwFlags;
    DWORD dwDataSize;
    DWORD dwNumObjs;
    LPDIOBJECTDATAFORMAT rgodf;
} DIDATAFORMAT, * LPDIDATAFORMAT;
typedef const DIDATAFORMAT* LPCDIDATAFORMAT;

class CProxyStub
{
protected:

    CProxyStub()
    {
    }
    ~CProxyStub()
    {
        __clear_proc( DirectInput8Create );
        __clear_proc( DllCanUnloadNow );
        __clear_proc( DllGetClassObject );
        __clear_proc( DllRegisterServer );
        __clear_proc( DllUnregisterServer );
        __clear_proc( GetdfDIJoystick );
    }

private:

    const char* m_OldProxyDll{ "version.dll" };
    const char* m_ProxyDll{ "dinput8.dll" };

    // A full path to the proxy library
    fs::path m_proxyPath{};

public:

    /// <summary>
    /// Gets the current 'CProxyStub' class instance
    /// </summary>
    static CProxyStub& instance()
    {
        static CProxyStub instance;
        return instance;
    }

    // Get the current loaded proxy path
    fs::path get_proxy_path() const
    {
        return m_proxyPath;
    }

    // Resolve typedefs
    __typedef_func( DirectInput8Create, HRESULT, HINSTANCE, DWORD, const IID* const, LPVOID*, LPUNKNOWN );
    __typedef_func( DllCanUnloadNow, HRESULT );
    __typedef_func( DllGetClassObject, HRESULT, const IID* const, const IID* const, LPVOID* );
    __typedef_func( DllRegisterServer, HRESULT );
    __typedef_func( DllUnregisterServer, HRESULT );
    __typedef_func( GetdfDIJoystick, LPCDIDATAFORMAT );

    /// <summary>
    /// Loads an original system library and resolves all the function procs
    /// </summary>
    /// <returns>Returns "true" upon success, "false" otherwise</returns>
    bool resolve( const HMODULE& mod )
    {
        // Remove an old proxy just in case it still exists in a folder
        if (fs::exists( m_OldProxyDll ))
            fs::remove( m_OldProxyDll );

        wchar_t proxy_path[4096]{};
        GetModuleFileNameW( mod, proxy_path, (sizeof( proxy_path )) );
        m_proxyPath = fs::path( proxy_path ).parent_path();

        wchar_t sys_path[MAX_PATH]{};
        GetSystemDirectoryW( sys_path, MAX_PATH );

        // A full path to the original system library
        auto system_dll_path = fs::path( sys_path ) / m_ProxyDll;

        // Try loading an original system library
        auto proxy_dll = LoadLibraryExW( system_dll_path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 );

        if (proxy_dll == nullptr)
            return false;

        __setup_proc( DirectInput8Create, proxy_dll );
        __setup_proc( DllCanUnloadNow, proxy_dll );
        __setup_proc( DllGetClassObject, proxy_dll );
        __setup_proc( DllRegisterServer, proxy_dll );
        __setup_proc( DllUnregisterServer, proxy_dll );
        __setup_proc( GetdfDIJoystick, proxy_dll );

        return true;
    }
};