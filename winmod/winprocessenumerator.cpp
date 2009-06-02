/**
* @file    winprocessenumerator.cpp
* @brief   ...
* @author  bbcallen
* @date    2009-02-25  11:27
*/

#include "stdafx.h"
#include "winprocessenumerator.h"

#include <atlpath.h>
#include "winmod\winpath.h"
#include "winmod\wintokenstack.h"

#pragma comment(lib, "psapi.lib")

#define UNICODE_MAX_PATH 32768

using namespace WinMod;

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
        // 数组不够大,需要重新分配新的大小
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
    // 临时获取DebugPrevilege
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
