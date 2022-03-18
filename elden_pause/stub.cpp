#include "pch.h"

EXTERN_API HRESULT WINAPI DirectInput8Create(
        HINSTANCE hinst,
        DWORD dwVersion,
        const IID* const riidltf,
        LPVOID* ppvOut,
        LPUNKNOWN punkOuter )
{
    return CProxyStub::instance().m_DirectInput8Create( hinst, dwVersion, riidltf, ppvOut, punkOuter );
}


#pragma comment(linker, "/export:DllCanUnloadNow=_DllCanUnloadNow,PRIVATE")
EXTERN_API HRESULT WINAPI _DllCanUnloadNow()
{
  return CProxyStub::instance().m_DllCanUnloadNow();
}


#pragma comment(linker, "/export:DllGetClassObject=_DllGetClassObject,PRIVATE")
EXTERN_API HRESULT WINAPI _DllGetClassObject(
    const IID* const rclsid,
    const IID* const riid,
    LPVOID* ppv )
{
    return CProxyStub::instance().m_DllGetClassObject( rclsid, riid, ppv );
}


#pragma comment(linker, "/export:DllRegisterServer=_DllRegisterServer,PRIVATE")
EXTERN_API HRESULT WINAPI _DllRegisterServer()
{
    return CProxyStub::instance().m_DllRegisterServer();
}


#pragma comment(linker, "/export:DllUnregisterServer=_DllUnregisterServer,PRIVATE")
EXTERN_API HRESULT WINAPI _DllUnregisterServer()
{
    return CProxyStub::instance().m_DllUnregisterServer();
}


EXTERN_API LPCDIDATAFORMAT WINAPI GetdfDIJoystick()
{
    return CProxyStub::instance().m_GetdfDIJoystick();
}