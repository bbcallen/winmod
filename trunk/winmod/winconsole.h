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

NS_WINMOD_BEGIN

class CWinConsole
{
public:
    static HRESULT SetError(HRESULT hrError);
    static BOOL    IsValid();
#ifndef _ATL_MIN_CRT
    static HRESULT CreateConsoleCRT();
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

#ifndef _ATL_MIN_CRT
inline HRESULT CWinConsole::CreateConsoleCRT()
{
    if (SUCCEEDED(sm_hrError))
        return S_FALSE;

    if (::AllocConsole())
    {
        ::BringWindowToTop(::GetConsoleWindow());

        COORD size = {80, 500};
        ::SetConsoleScreenBufferSize(::GetStdHandle(STD_OUTPUT_HANDLE), size);
    }

    int nHandle = _open_osfhandle((INT_PTR)::GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT | _O_APPEND);
    if (nHandle == -1)
        return SetError(E_FAIL);

    *stdout = *_wfdopen(nHandle, L"wt");
    return SetError(S_OK);
}
#endif

NS_WINMOD_END

#endif//WINCONSOLE_H