/**
* @file    winregkey.cpp
* @brief   ...
* @author  bbcallen
* @date    2009-04-25  15:07
*/

#include "stdafx.h"
#include "winregkey.h"

#include <assert.h>
#include "winmod\winpath.h"

using namespace WinMod;

HRESULT CWinRegKey::ExQueryStringValue(LPCWSTR lpszValueName, CString& strValue, DWORD dwCchMaxLen)
{
    assert(m_hKey);
    if (!m_hKey)
        return E_HANDLE;


    // 将 dwCchMaxLen 调整到 1024 和 ULONG_MAX 之间
    dwCchMaxLen = max(MAX_PATH, dwCchMaxLen);
    dwCchMaxLen = min(LONG_MAX, dwCchMaxLen);
    if (dwCchMaxLen <= 0)
        return AtlHresultFromWin32(ERROR_MORE_DATA);


    // 获取长度,注意,这里的单位是字节数
    ULONG uCchValueLen = 0;
    LONG lRet = CRegKey::QueryStringValue(lpszValueName, NULL, &uCchValueLen);
    if (ERROR_SUCCESS != lRet)
        return lRet ? AtlHresultFromWin32(lRet) : E_FAIL;


    // 将 dwCchMaxLen 调整到 MAX_PATH 以上, 并检查长度限制
    uCchValueLen = max(uCchValueLen, MAX_PATH);
    if (uCchValueLen > dwCchMaxLen)
        return AtlHresultFromWin32(ERROR_MORE_DATA);


    // 最少长度为1024
    lRet = CRegKey::QueryStringValue(lpszValueName, strValue.GetBuffer(uCchValueLen + 1), &uCchValueLen);
    if (ERROR_SUCCESS != lRet)
    {
        strValue.ReleaseBuffer(0);
        return lRet ? AtlHresultFromWin32(lRet) : E_FAIL;
    }
    strValue.ReleaseBuffer(-1);


    return S_OK;
}


HRESULT CWinRegKey::ExQueryExpendedStringValue(LPCWSTR lpszValueName, CString& strValue, DWORD dwCchMaxLen)
{
    HRESULT hr = ExQueryStringValue(lpszValueName, strValue, dwCchMaxLen);
    if (FAILED(hr))
        return hr;

    CWinPathApi::ExpandAsAccessiblePath(strValue);
    CWinPathApi::ExpandEnvironmentStrings(strValue);

    return S_OK;
}