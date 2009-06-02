/**
* @file    winregkeyfind.cpp
* @brief   ...
* @author  bbcallen
* @date    2009-04-23  12:24
*/

#include "stdafx.h"
#include "winregkeyfind.h"

#include <assert.h>

using namespace WinMod;

BOOL CWinRegKeyFind::FindFirstSubKey(HKEY hKeyParent, LPCWSTR lpszKeyName, REGSAM samDesired)
{
    Reset();


    samDesired  |= KEY_ENUMERATE_SUB_KEYS;
    LONG lRet = m_hKeyEnum.Open(hKeyParent, lpszKeyName, samDesired);
    if (ERROR_SUCCESS != lRet)
        return FALSE;


    m_hKeyParent = hKeyParent;
    m_strKeyName = lpszKeyName;


    return DoEnumRegKey();
}

BOOL CWinRegKeyFind::FindNextSubKey()
{
    if (IsEndOfFind())
        return FALSE;


    ++m_dwEnumIndex;


    return DoEnumRegKey();
}


BOOL CWinRegKeyFind::DoEnumRegKey()
{
    if (IsEndOfFind())
        return FALSE;

    m_szSubKeyName[0] = L'\0';
    m_ftLastWriteTime.dwHighDateTime = 0;
    m_ftLastWriteTime.dwLowDateTime  = 0;
    DWORD dwKeyNameLength = _countof(m_szSubKeyName) - 1;
    LONG lRet = m_hKeyEnum.EnumKey(
        m_dwEnumIndex,
        m_szSubKeyName,
        &dwKeyNameLength,
        &m_ftLastWriteTime);
    if (ERROR_SUCCESS != lRet)
    {
        Reset();
        return FALSE;
    }


    m_szSubKeyName[_countof(m_szSubKeyName) - 1] = L'\0';
    return TRUE;
}