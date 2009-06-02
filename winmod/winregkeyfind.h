/**
* @file    winregfinddepthfirst.h
* @brief   ...
* @author  bbcallen
* @date    2009-04-23  12:22
*/

#ifndef WINREGFINDDEPTHFIRST_H
#define WINREGFINDDEPTHFIRST_H

#include <atlstr.h>
#include <atlbase.h>
#include "winmod\winmodbase.h"
#include "winmod\winregkey.h"

NS_WINMOD_BEGIN

class CWinRegKeyFind
{
public:
    CWinRegKeyFind();
    virtual ~CWinRegKeyFind();

    // Operations
public:

    void    Reset();

    BOOL    FindFirstSubKey(HKEY hKeyParent, LPCWSTR lpszKeyName, REGSAM samDesired = KEY_ENUMERATE_SUB_KEYS);

    BOOL    FindNextSubKey();

    BOOL    IsEndOfFind() const;

    LPCWSTR GetSubKeyName() const;

    CString GetFullKeyName() const;

    const FILETIME& GetSubKeyLastWriteTime() const;

private:
    // Disabled
    CWinRegKeyFind(const CWinRegKeyFind&);
    CWinRegKeyFind& operator=(const CWinRegKeyFind&);

public:
    CWinRegKey  m_hKeyEnum;
    HKEY        m_hKeyParent;

protected:

    BOOL        DoEnumRegKey();

    CString     m_strKeyName;
    FILETIME    m_ftLastWriteTime;
    WCHAR       m_szSubKeyName[MAX_PATH];
    DWORD       m_dwEnumIndex;
};

inline CWinRegKeyFind::CWinRegKeyFind()
{
    Reset();
}

inline CWinRegKeyFind::~CWinRegKeyFind()
{
}

inline void CWinRegKeyFind::Reset()
{
    m_hKeyEnum.Close();

    m_hKeyParent = NULL;
    m_strKeyName.Empty();
    m_dwEnumIndex = 0;
    m_szSubKeyName[0] = L'\0';
    m_ftLastWriteTime.dwHighDateTime = 0;
    m_ftLastWriteTime.dwLowDateTime  = 0;
}

inline BOOL CWinRegKeyFind::IsEndOfFind() const
{
    return NULL == m_hKeyEnum;
}

inline LPCWSTR CWinRegKeyFind::GetSubKeyName() const
{
    return m_szSubKeyName;
}

inline CString CWinRegKeyFind::GetFullKeyName() const
{
    if (m_strKeyName.IsEmpty())
        return m_szSubKeyName;

    if (L'\0' == m_szSubKeyName[0])
        return m_strKeyName;

    CString strFullKeyName = m_strKeyName;
    if (L'\\' != strFullKeyName.GetAt(strFullKeyName.GetLength() - 1))
        strFullKeyName.AppendChar(L'\\');

    strFullKeyName.Append(m_szSubKeyName);
    return strFullKeyName;
}

inline const FILETIME& CWinRegKeyFind::GetSubKeyLastWriteTime() const
{
    return m_ftLastWriteTime;
}

NS_WINMOD_END

#endif//WINREGFINDDEPTHFIRST_H