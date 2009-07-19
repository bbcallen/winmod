/**
* @file    wintokenstack.cpp
* @brief   ...
* @author  bbcallen
* @date    2009-02-25  11:27
*/

#include "stdafx.h"
#include "wintokenstack.h"

#include <assert.h>
#include <atlsecurity.h>

using namespace WinMod;

HRESULT CWinTokenHelper::EnableDebugPrivilege()
{
    CAccessToken token;
    BOOL br = token.GetEffectiveToken(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY);
    if (!br)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    br = token.EnablePrivilege(L"SeDebugPrivilege");
    if (!br)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    return S_OK;
}

HRESULT CWinTokenHelper::DisableDebugPrivilege()
{
    CAccessToken token;
    BOOL br = token.GetEffectiveToken(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY);
    if (!br)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    br = token.DisablePrivilege(L"SeDebugPrivilege");
    if (!br)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    return S_OK;
}










CWinTokenStack::CWinTokenStack()
{

}

CWinTokenStack::~CWinTokenStack()
{
    PopAll();
}

HRESULT CWinTokenStack::Push()
{
    CAccessToken token;
    BOOL br = token.GetEffectiveToken(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY);
    if (!br)
    {
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;
    }

    m_tokenStack.AddTail(token.Detach());
    return S_OK;
}

HRESULT CWinTokenStack::Pop()
{
    assert(!m_tokenStack.IsEmpty());
    if (m_tokenStack.IsEmpty())
        return E_FAIL;

    CAccessToken token;
    token.Attach(m_tokenStack.RemoveTail());
    


    if (token.GetHandle())
    {
        bool br = token.Impersonate();
        if (!br)
            return GetLastError() ? AtlHresultFromLastError() : E_FAIL;
    }
    else
    {
        bool br = token.Revert();
        if (!br)
            return GetLastError() ? AtlHresultFromLastError() : E_FAIL;
    }



    return S_OK;
}

void CWinTokenStack::PopAll()
{
    while (m_tokenStack.GetCount() > 1)
    {
        HANDLE hToken = m_tokenStack.RemoveTail();
        if (hToken)
            ::CloseHandle(hToken);
    }

    if (!m_tokenStack.IsEmpty())
        Pop();
}
