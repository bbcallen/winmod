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


    // �� dwCchMaxLen ������ 1024 �� ULONG_MAX ֮��
    dwCchMaxLen = max(MAX_PATH, dwCchMaxLen);
    dwCchMaxLen = min(LONG_MAX, dwCchMaxLen);
    if (dwCchMaxLen <= 0)
        return AtlHresultFromWin32(ERROR_MORE_DATA);


    // ��ȡ����,ע��,����ĵ�λ���ֽ���
    ULONG uCchValueLen = 0;
    LONG lRet = CRegKey::QueryStringValue(lpszValueName, NULL, &uCchValueLen);
    if (ERROR_SUCCESS != lRet)
        return lRet ? AtlHresultFromWin32(lRet) : E_FAIL;


    // �� dwCchMaxLen ������ MAX_PATH ����, ����鳤������
    uCchValueLen = max(uCchValueLen, MAX_PATH);
    if (uCchValueLen > dwCchMaxLen)
        return AtlHresultFromWin32(ERROR_MORE_DATA);


    // ���ٳ���Ϊ1024
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