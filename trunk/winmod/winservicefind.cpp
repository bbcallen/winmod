/**
* @file    winservicefind.cpp
* @brief   ...
* @author  bbcallen
* @date    2009-04-25  14:49
*/

#include "stdafx.h"
#include "winservicefind.h"

#include <assert.h>
#include "winmod\winmodule.h"

using namespace WinMod;


#define WINMOD_REGSUBKEY_PARAMETERS             L"Parameters"   ///< Services\\..\\Parameters

#define WINMOD_REGVALUE_SERVICES_IMAGEPATH      L"ImagePath"    ///< Services\\..\\ImagePath
#define WINMOD_REGVALUE_SERVICES_DISPLAYNAME    L"DisplayName"  ///< Services\\..\\DisplayName
#define WINMOD_REGVALUE_SERVICES_DESCRIPTION    L"Description"  ///< Services\\..\\Description
#define WINMOD_REGVALUE_SERVICES_TYPE           L"Type"         ///< Services\\..\\Type
#define WINMOD_REGVALUE_SERVICES_START          L"Start"        ///< Services\\..\\Start

#define WINMOD_REGVALUE_SERVICES_Service        L"ServiceDll"   ///< Services\\..\\Parameters\\ServiceDll


BOOL CWinServiceFind::FindFirstService(HKEY hRootKey, LPCWSTR lpszControlSet, REGSAM samDesired)
{
    assert(hRootKey);
    assert(lpszControlSet);
    Reset();

    samDesired |= KEY_ENUMERATE_SUB_KEYS;
    BOOL bFind = m_hRegKeyFind.FindFirstSubKey(hRootKey, lpszControlSet, samDesired);
    if (!bFind)
    {
        Reset();
        return FALSE;
    }

    DoReadStandardValues();
    return TRUE;
}

BOOL CWinServiceFind::FindNextService()
{
    if (IsEndOfFind())
        return FALSE;

    BOOL bFind = m_hRegKeyFind.FindNextSubKey();
    if (!bFind)
    {
        Reset();
        return FALSE;
    }

    DoReadStandardValues();
    return TRUE;
}


void CWinServiceFind::DoReadStandardValues()
{
    assert(!IsEndOfFind());
    if (IsEndOfFind())
        return;


    m_imagePath         = L"";
    m_serviceDll        = L"";
    m_strDisplayName    = L"";
    m_strDescription    = L"";
    m_dwServiceType     = ULONG_MAX;
    m_dwServiceStart    = ULONG_MAX;


    CString    strKeyName = m_hRegKeyFind.GetFullKeyName();
    CWinRegKey hRegKeyService;
    LONG lRet = hRegKeyService.Open(m_hRegKeyFind.m_hKeyParent, strKeyName, KEY_QUERY_VALUE);
    if (ERROR_SUCCESS == lRet)
    {   // ..\\Service\\[ImagePath]
        HRESULT hr = hRegKeyService.ExQueryStringValue(WINMOD_REGVALUE_SERVICES_IMAGEPATH, m_imagePath);
        if (FAILED(hr))
            m_imagePath = L"";              // no return

        // ..\\Service\\[DisplayName]
        hr = hRegKeyService.ExQueryStringValue(WINMOD_REGVALUE_SERVICES_DISPLAYNAME, m_strDisplayName);
        if (FAILED(hr))
            m_strDisplayName = L"";         // no return

        // ..\\Service\\[Description]
        hr = hRegKeyService.ExQueryStringValue(WINMOD_REGVALUE_SERVICES_DESCRIPTION, m_strDescription);
        if (FAILED(hr))
            m_strDescription = L"";         // no return

        // ..\\Service\\[Type]
        lRet = hRegKeyService.QueryDWORDValue(WINMOD_REGVALUE_SERVICES_TYPE, m_dwServiceType);
        if (ERROR_SUCCESS != lRet)
            m_dwServiceType = ULONG_MAX;    // no return

        // ..\\Service\\[Start]
        lRet = hRegKeyService.QueryDWORDValue(WINMOD_REGVALUE_SERVICES_START, m_dwServiceStart);
        if (ERROR_SUCCESS != lRet)
            m_dwServiceStart = ULONG_MAX;   // no return
    }

    
    if (SERVICE_WIN32_SHARE_PROCESS == m_dwServiceType)
    {   // 对于 svchost.exe 这一类的服务,还要检查ServiceDll
        assert(!strKeyName.IsEmpty() && L'\\' != strKeyName[strKeyName.GetLength() - 1]);
        strKeyName.AppendChar(L'\\');
        strKeyName.Append(WINMOD_REGSUBKEY_PARAMETERS);
        CWinRegKey hRegKeyParameters;
        lRet = hRegKeyParameters.Open(m_hRegKeyFind.m_hKeyParent, strKeyName, KEY_QUERY_VALUE);
        if (ERROR_SUCCESS == lRet)
        {   // ..\\Service\\Parameters\\[ServiceDll]
            HRESULT hr = hRegKeyParameters.ExQueryStringValue(WINMOD_REGVALUE_SERVICES_Service, m_serviceDll);
            if (FAILED(hr))
                m_serviceDll = L"";              // no return
        }
    }

}



LPCWSTR CWinServiceFind::GetServiceTypeAsString() const
{
    switch (m_dwServiceType)
    {
    case ULONG_MAX:                     return L"N/A";
    case SERVICE_ADAPTER:               return L"Adapter";
    case SERVICE_FILE_SYSTEM_DRIVER:    return L"File System Driver";
    case SERVICE_KERNEL_DRIVER:         return L"Kernel Driver";
    case SERVICE_RECOGNIZER_DRIVER:     return L"Recognizer Driver";
    case SERVICE_WIN32_OWN_PROCESS:     return L"Win32 Own Process";
    case SERVICE_WIN32_SHARE_PROCESS:   return L"Win32 Share Process";
    default:
        return L"Unknown Type";
    }
}

LPCWSTR CWinServiceFind::GetServiceStartAsString() const
{
    switch (m_dwServiceStart)
    {
    case ULONG_MAX:                 return L"N/A";
    case SERVICE_AUTO_START:        return L"Auto Start";
    case SERVICE_BOOT_START:        return L"Boot Start";
    case SERVICE_DEMAND_START:      return L"Demand Start";
    case SERVICE_DISABLED:          return L"Disabled";
    case SERVICE_SYSTEM_START:      return L"System Start";
    default:
        return L"Unknown Start";
    }
}

CString CWinServiceFind::GetConvertedDisplayName() const
{
    return DoConvertString(m_strDisplayName);
}

CString CWinServiceFind::GetConvertedDescription() const
{
    return DoConvertString(m_strDescription);
}

CString CWinServiceFind::DoConvertString(const CString& strMessage) const
{
    if (strMessage.IsEmpty())
        return L"";


    // 不需要加载资源dll,直接返回显示名称
    if (L'@' != strMessage[0])
        return strMessage;


    int nPos = strMessage.ReverseFind(L'-');
    if (-1 == nPos)
        return L"";


    // 寻找资源id
    LPCWSTR lpResourceID = (LPCWSTR)strMessage + nPos + 1;
    DWORD   dwResourceID = StrToInt(lpResourceID);


    // 寻找分隔资源id的逗号
    CString strResourceFile = (LPCWSTR)strMessage + 1;
    --nPos;
    while (nPos > 0 && strResourceFile[nPos] != L',')
    {
        --nPos;
    }

    // 逗号前的部分截取为资源文件路径
    strResourceFile.Truncate(nPos);
    CWinPathApi::ExpandEnvironmentStrings(strResourceFile);
    CWinPathApi::ExpandAsAccessiblePath(strResourceFile);


    CWinModule hModule;
    HRESULT hr = hModule.LoadLibEx(strResourceFile, LOAD_LIBRARY_AS_DATAFILE);
    if (FAILED(hr))
        return L"";


    WCHAR szBuf[BUFSIZ];
    szBuf[_countof(szBuf) - 1] = L'\0';
    int nRet = LoadString(hModule, dwResourceID, szBuf, BUFSIZ - 1);
    if (0 == nRet)
        strMessage;


    return CString(szBuf);
}