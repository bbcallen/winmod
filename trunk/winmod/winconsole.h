/**
* @file    winconsole.h
* @brief   ...
* @author  bbcallen
* @date    2011-04-10 11:33
*/

#ifndef WINCONSOLE_H
#define WINCONSOLE_H

#ifndef _ATL_MIN_CRT
#include <io.h>
#include <fcntl.h>
#endif
#include "winmodbase.h"
#include "winrunnable.h"

NS_WINMOD_BEGIN

class CWinConsole
{
public:
    static HRESULT SetError(HRESULT hrError);
    static BOOL    IsValid();
    static HRESULT CreateConsole();

#ifndef _ATL_MIN_CRT
    static HRESULT CreateConsoleCRT();
    static HRESULT CRTRedirectStdIn();
    static HRESULT CRTRedirectStdOut();
    static HRESULT CRTRedirectStdErr();
#endif

protected:
    static HRESULT sm_hrError;
};

__declspec(selectany) HRESULT CWinConsole::sm_hrError = ULONG_MAX;

inline HRESULT CWinConsole::SetError(HRESULT hrError)
{
    sm_hrError = hrError;
    return hrError;
}

inline BOOL CWinConsole::IsValid()
{
    return SUCCEEDED(sm_hrError);
}

inline HRESULT CWinConsole::CreateConsole()
{
    if (SUCCEEDED(sm_hrError))
        return S_FALSE;

    if (::AllocConsole())
    {
        ::BringWindowToTop(::GetConsoleWindow());

        COORD size = {80, 500};
        ::SetConsoleScreenBufferSize(::GetStdHandle(STD_OUTPUT_HANDLE), size);
    }

    return SetError(S_OK);
}

#ifndef _ATL_MIN_CRT
inline HRESULT CWinConsole::CreateConsoleCRT()
{
    HRESULT hr = CreateConsole();
    if (FAILED(hr))
        return hr;

    CRTRedirectStdIn();
    CRTRedirectStdOut();
    CRTRedirectStdErr();
    return S_OK;
}

inline HRESULT CWinConsole::CRTRedirectStdIn()
{
    int nHandle = _open_osfhandle((INT_PTR)::GetStdHandle(STD_INPUT_HANDLE), _O_TEXT);
    if (nHandle == -1)
        return SetError(E_FAIL);

    *stdin = *_wfdopen(nHandle, L"rt");
    return SetError(S_OK);
}

inline HRESULT CWinConsole::CRTRedirectStdOut()
{
    int nHandle = _open_osfhandle((INT_PTR)::GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT | _O_APPEND);
    if (nHandle == -1)
        return SetError(E_FAIL);

    *stdout = *_wfdopen(nHandle, L"wt");
    return SetError(S_OK);
}

inline HRESULT CWinConsole::CRTRedirectStdErr()
{
    int nHandle = _open_osfhandle((INT_PTR)::GetStdHandle(STD_ERROR_HANDLE), _O_TEXT | _O_APPEND);
    if (nHandle == -1)
        return SetError(E_FAIL);

    *stderr = *_wfdopen(nHandle, L"wt");
    return SetError(S_OK);
}

#endif

NS_WINMOD_END

#endif//WINCONSOLE_H