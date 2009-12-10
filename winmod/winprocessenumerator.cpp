/**
* @file    winprocessenumerator.cpp
* @brief   ...
* @author  bbcallen
* @date    2009-02-25  11:27
*/

#include "stdafx.h"
#include "winprocessenumerator.h"

#include <sddl.h>
#include <atlpath.h>
#include "winmod\winosver.h"
#include "winmod\winpath.h"

#pragma comment(lib, "psapi.lib")

using namespace WinMod;

HRESULT CWinProcessApi::ObtainImpersonateToken(DWORD dwProcessId, HANDLE& hTokenObtain)
{
    CHandle hProcess;
    hProcess.Attach(::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId));
    if (!hProcess)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;



    CHandle hTokenDuplicate;
    HANDLE  hT = NULL;
    BOOL bRet = ::OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hT);
    if (!bRet)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;
    hTokenDuplicate.Attach(hT);
    hT = NULL;



    if (CWinOSVer::IsVistaOrLater())
    {
        TOKEN_LINKED_TOKEN linkedToken = {0};

        DWORD dwSize = sizeof linkedToken;

        bRet = ::GetTokenInformation(hTokenDuplicate, TokenLinkedToken, &linkedToken, dwSize, &dwSize);
        if (!bRet)
            return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

        hTokenDuplicate.Close();
        hTokenDuplicate.Attach(linkedToken.LinkedToken);
        linkedToken.LinkedToken = NULL;
    }


    bRet = ::DuplicateTokenEx(
        hTokenDuplicate, 
        TOKEN_ALL_ACCESS, 
        0, 
        SecurityImpersonation, 
        TokenPrimary, 
        &hT);
    if (!bRet)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;
    hTokenDuplicate.Close();
    hTokenDuplicate.Attach(hT);
    hT = NULL;


    if (CWinOSVer::IsVistaOrLater())
    {
        {
            PWSTR szIntegritySid = L"S-1-16-12288"; //high
            PSID  pIntegritySid  = NULL;
            TOKEN_MANDATORY_LABEL til = {0};

            bRet = ::ConvertStringSidToSid(szIntegritySid, &pIntegritySid);
            if (!bRet)
                return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

            til.Label.Attributes = SE_GROUP_INTEGRITY;
            til.Label.Sid = pIntegritySid;

            bRet = ::SetTokenInformation(hTokenDuplicate, TokenIntegrityLevel,  &til, sizeof(TOKEN_MANDATORY_LABEL));
            if (!bRet)
                return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

            if (pIntegritySid)
            {
                ::LocalFree((HLOCAL)pIntegritySid);
                pIntegritySid = NULL;
            }
        }
    }

    hTokenObtain = hTokenDuplicate.Detach();
    return S_OK;
}




HRESULT CWinProcessApi::ExecuteAsDesktopUser(
    LPCWSTR                 lpszExePath,
    LPCWSTR                 lpszCommandLine,
    DWORD                   dwDesktopProcessId,
    PROCESS_INFORMATION*    pProcessInformation)
{
    HRESULT hr = E_FAIL;
    CHandle hToken;
    HANDLE  hT = NULL;
    if (dwDesktopProcessId)
    {   // 获取指定进程的token
        hr = CWinProcessApi::ObtainImpersonateToken(dwDesktopProcessId, hT);
        if (SUCCEEDED(hr))
        {
            hToken.Attach(hT);
            hT = NULL;
        }
    }



    if (!hToken)
    {  // 获取explorer的token
        hr = ObtainExplorerToken(hT);
        if (FAILED(hr))
            return hr;

        hToken.Attach(hT);
        hT = NULL;
    }





    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));


    si.cb        = sizeof(STARTUPINFO);
    si.lpDesktop = L"winsta0\\default";


    // 以指定token创建进程
    CString strCommandLine = lpszCommandLine;
    BOOL bRet = ::CreateProcessAsUser(        
        hToken,
        lpszExePath,
        (LPWSTR)(LPCWSTR)strCommandLine,
        NULL,
        NULL,
        FALSE,
        NORMAL_PRIORITY_CLASS,
        NULL,
        NULL,
        &si,
        &pi);
    if (!bRet)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;



    if (pProcessInformation)
    {
        *pProcessInformation = pi;
    }
    else
    {
        ::CloseHandle(pi.hThread);
        ::CloseHandle(pi.hProcess);
    }


    return S_OK;
}



HRESULT CWinProcessApi::ObtainExplorerToken(HANDLE& hTokenObtain)
{
    CWinProcessEnumerator hProcEnum;
    HRESULT hr = hProcEnum.EnumAllProcesses();
    if (FAILED(hr))
        return hr;


    WCHAR szPath[MAX_PATH + 1];
    ::GetWindowsDirectory(szPath, MAX_PATH);
    ::PathAppend(szPath, L"explorer.exe");


    BOOL bFind = hProcEnum.FindFirstProcess();
    for (NULL; bFind; bFind = hProcEnum.FindNextProcess())
    {
        CString strProcPath;
        hr = hProcEnum.GetProcessPath(strProcPath);
        if (FAILED(hr))
            continue;

        if (0 != strProcPath.CompareNoCase(szPath))
            continue;

        hr = CWinProcessApi::ObtainImpersonateToken(hProcEnum.GetProcessID(), hTokenObtain);
        if (FAILED(hr))
            continue;

        return S_OK;
    }


    return E_FAIL;
}










CWinProcessEnumerator::CWinProcessEnumerator()
{
    Reset();
}

HRESULT CWinProcessEnumerator::EnumAllProcesses()
{
    Reset();

    // obtain DebugPrevilege
    CWinTokenStack localPrivilege;
    localPrivilege.Push();
    localPrivilege.EnableDebugPrivilege();

    DWORD dwReturnCount     = 200;
    DWORD dwExpectedCount   = 0;

    while (dwExpectedCount < dwReturnCount)
    {
        // need reallocate
        dwExpectedCount = dwReturnCount + 20;   // 20 more in case that process list changes

        BOOL br = m_procList.SetCount(dwExpectedCount);
        if (!br)
        {
            m_procList.RemoveAll();
            return E_OUTOFMEMORY;
        }

        DWORD dwReturnByte = 0;
        br = ::EnumProcesses(
            m_procList.GetData(),
            DWORD(sizeof(DWORD) * m_procList.GetCount()),
            &dwReturnByte);
        if (!br)
        {
            m_procList.RemoveAll();
            return GetLastError() ? HRESULT_FROM_WIN32(GetLastError()) : E_FAIL;
        }


        dwReturnCount = dwReturnByte / sizeof(DWORD);
    }

    m_procList.SetCount(dwReturnCount);
    return S_OK;
}

BOOL CWinProcessEnumerator::FindFirstProcess()
{
    m_procIndex = 0;
    return !EndOfProcesses();
}

BOOL CWinProcessEnumerator::FindNextProcess()
{
    ++m_procIndex;
    return !EndOfProcesses();
}

BOOL CWinProcessEnumerator::EndOfProcesses()
{
    return m_procIndex >= m_procList.GetCount();
}

DWORD CWinProcessEnumerator::GetProcessID()
{
    if (EndOfProcesses())
        return 0;

    return m_procList[m_procIndex];
}

HRESULT CWinProcessEnumerator::GetProcessPath(CString& strProcessPath)
{
    // obtain DebugPrevilege
    CWinTokenStack localPrivilege;
    localPrivilege.Push();
    localPrivilege.EnableDebugPrivilege();

    CHandle hProcess;
    hProcess.Attach(::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetProcessID()));
    if (!hProcess)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    DWORD dwRet = ::GetModuleFileNameEx(
        hProcess,
        NULL,
        strProcessPath.GetBuffer(UNICODE_MAX_PATH),
        UNICODE_MAX_PATH);
    strProcessPath.ReleaseBuffer(dwRet);
    if (!dwRet)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    CWinPathApi::ExpandAsAccessiblePath(strProcessPath);
    return S_OK;
}

HRESULT CWinProcessEnumerator::GetProcessImageName(CString& strProcessImageName)
{
    // obtain DebugPrevilege
    CWinTokenStack localPrivilege;
    localPrivilege.Push();
    localPrivilege.EnableDebugPrivilege();


    CHandle hProcess;
    hProcess.Attach(::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetProcessID()));
    if (!hProcess)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    HRESULT hr = E_FAIL;
    if (!dll_psapi)
    {
        hr = dll_psapi.LoadLib(L"psapi.dll");
        if (FAILED(hr))
            return hr;
    }

    DWORD dwRet = dll_psapi.GetProcessImageFileNameW(
        hProcess,
        strProcessImageName.GetBuffer(UNICODE_MAX_PATH),
        UNICODE_MAX_PATH);
    strProcessImageName.ReleaseBuffer(dwRet);
    if (!dwRet)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    return S_OK;
}

HRESULT CWinProcessEnumerator::GetProcessName(CString& strProcessName)
{
    // obtain DebugPrevilege
    CWinTokenStack localPrivilege;
    localPrivilege.Push();
    localPrivilege.EnableDebugPrivilege();

    CWinPath strPath;

    // try ::GetModuleFileNameEx()
    HRESULT hr = GetProcessPath(strPath.m_strPath);
    if (FAILED(hr))
    {
        // then try ::GetProcessImageFileName()
        hr = GetProcessImageName(strPath.m_strPath);
        if (FAILED(hr))
        {
            // try Toolhelp32 函数 at last
            CWinProcessToolHelpEnumerator thEnum;
            BOOL br = thEnum.FindProcessID(GetProcessID());
            if (!br)
                return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

            hr = thEnum.GetProcessName(strPath.m_strPath);
            if (FAILED(hr))
                return hr;
        }
    }

    strPath.StripPath();
    strProcessName = strPath.m_strPath;

    return S_OK;
}

void CWinProcessEnumerator::Reset()
{
    m_procIndex = 0;
    m_procList.RemoveAll();
}












HRESULT CWinModuleEnumerator::EnumAllModules(DWORD dwModuleID)
{
    Reset();

    // 临时获取DebugPrevilege
    CWinTokenStack localPrivilege;
    localPrivilege.Push();
    localPrivilege.EnableDebugPrivilege();

    m_hProc.Attach(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwModuleID));
    if (!m_hProc)
        return GetLastError() ? HRESULT_FROM_WIN32(GetLastError()) : E_FAIL;



    DWORD dwReturnByte = 0;
    BOOL br = ::EnumProcessModules(m_hProc, NULL, 0, &dwReturnByte);
    if (!br)
        return GetLastError() ? HRESULT_FROM_WIN32(GetLastError()) : E_FAIL;

    
    DWORD dwReturnCount   = dwReturnByte / sizeof(HMODULE);
    DWORD dwExpectedCount = 0;
    while (dwExpectedCount < dwReturnCount)
    {
        dwExpectedCount = dwReturnCount;

        br = m_moduleList.SetCount(dwExpectedCount);
        if (!br)
            return E_OUTOFMEMORY;

        DWORD dwReturnByte = 0;
        br = ::EnumProcessModules(
            m_hProc,
            m_moduleList.GetData(),
            DWORD(sizeof(HMODULE) * m_moduleList.GetCount()),
            &dwReturnByte);
        if (!br)
            return GetLastError() ? HRESULT_FROM_WIN32(GetLastError()) : E_FAIL;

        dwReturnCount = dwReturnByte / sizeof(HMODULE);
    }

    return S_OK;
}

BOOL CWinModuleEnumerator::FindFirstModule()
{
    m_moduleIndex = 0;
    return !EndOfModules();
}


BOOL CWinModuleEnumerator::FindNextModule()
{
    ++m_moduleIndex;
    return !EndOfModules();
}





BOOL CWinModuleEnumerator::EndOfModules()
{
    return m_moduleIndex >= m_moduleList.GetCount();
}

HMODULE CWinModuleEnumerator::GetModule()
{
    if (EndOfModules())
        return NULL;

    return m_moduleList[m_moduleIndex];
}

HRESULT CWinModuleEnumerator::GetModulePath(CString& strPath)
{
    if (EndOfModules())
        return E_FAIL;

    DWORD dwRet = ::GetModuleFileNameEx(
        m_hProc,
        GetModule(),
        strPath.GetBuffer(UNICODE_MAX_PATH),
        UNICODE_MAX_PATH);
    strPath.ReleaseBuffer(dwRet);
    if (0 == dwRet)
        return GetLastError() ? HRESULT_FROM_WIN32(GetLastError()) : E_FAIL;

    CWinPathApi::ExpandAsAccessiblePath(strPath);
    return S_OK;
}


HRESULT CWinProcessEnumerator::ObtainImpersonateToken(HANDLE& hTokenObtain)
{
    return CWinProcessApi::ObtainImpersonateToken(GetProcessID(), hTokenObtain);
}








void CWinModuleEnumerator::Reset()
{
    m_hProc.Close();

    m_moduleIndex = 0;
    m_moduleList.RemoveAll();
}














CWinProcessToolHelpEnumerator::CWinProcessToolHelpEnumerator():
    m_bValieProcEntry(NULL)
{
}

BOOL CWinProcessToolHelpEnumerator::FindProcessID(DWORD dwProcessID)
{
    HRESULT hr = EnumAllProcesses();
    if (FAILED(hr))
        return FALSE;

    for (FindFirstProcess(); !EndOfProcesses(); FindNextProcess())
    {
        if (dwProcessID == GetProcessID())
            return TRUE;
    }

    return FALSE;
}


HRESULT CWinProcessToolHelpEnumerator::EnumAllProcesses()
{
    // obtain DebugPrevilege
    CWinTokenStack localPrivilege;
    localPrivilege.Push();
    localPrivilege.EnableDebugPrivilege();

    m_hProcessSnap.Attach(::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    if (INVALID_HANDLE_VALUE == m_hProcessSnap)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    PROCESSENTRY32 pe32 = {0};
    pe32.dwSize = sizeof(PROCESSENTRY32);

    m_bValieProcEntry = TRUE;
    return S_OK;
}

BOOL CWinProcessToolHelpEnumerator::FindFirstProcess()
{
    ZeroMemory(&m_pe32, sizeof(m_pe32));
    m_pe32.dwSize = sizeof(m_pe32);
    m_bValieProcEntry = ::Process32FirstW(m_hProcessSnap, &m_pe32);
    return !EndOfProcesses();
}

BOOL CWinProcessToolHelpEnumerator::FindNextProcess()
{
    ZeroMemory(&m_pe32, sizeof(m_pe32));
    m_pe32.dwSize = sizeof(m_pe32);
    m_bValieProcEntry = ::Process32NextW(m_hProcessSnap, &m_pe32);
    return !EndOfProcesses();
}

BOOL CWinProcessToolHelpEnumerator::EndOfProcesses()
{
    return !m_bValieProcEntry;
}

DWORD CWinProcessToolHelpEnumerator::GetProcessID()
{
    if (EndOfProcesses())
        return 0;

    return m_pe32.th32ProcessID;
}

HRESULT CWinProcessToolHelpEnumerator::GetProcessName(CString& strProcessName)
{
    if (EndOfProcesses())
        return E_FAIL;

    strProcessName = m_pe32.szExeFile;
    return S_OK;
}
