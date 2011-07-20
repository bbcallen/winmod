/**
* @file    winmod_ntdll.h
* @brief   ...
* @author  bbcallen
* @date    2011-06-23 14:41
*/

#ifndef WINMOD_NTDLL_H
#define WINMOD_NTDLL_H

#include <atlcomcli.h>
#include "winmodbase.h"

NS_WINMOD_BEGIN

class CWinMod_ntdll
{
public:
    static CWinMod_ntdll& GetInstance()
    {
        static CWinMod_ntdll s_Instance;
        return s_Instance;
    }

    typedef NTSTATUS (WINAPI *PFN_NtQueryInformationProcess)(
        __in       HANDLE ProcessHandle,
        __in       PROCESSINFOCLASS ProcessInformationClass,
        __out      PVOID ProcessInformation,
        __in       ULONG ProcessInformationLength,
        __out_opt  PULONG ReturnLength
        );

    NTSTATUS NtQueryInformationProcess(
        HANDLE ProcessHandle,
        PROCESSINFOCLASS ProcessInformationClass,
        PVOID ProcessInformation,
        ULONG ProcessInformationLength,
        PULONG ReturnLength)
    {
        if (!m_pfnNtQueryInformationProcess)
        {
            HMODULE hMod = ::GetModuleHandle(L"ntdll.dll");
            if (!hMod)
                return ::GetLastError() ? AtlHresultFromLastError() : E_FAIL;

            m_pfnNtQueryInformationProcess = (PFN_NtQueryInformationProcess)::GetProcAddress(hMod, "NtQueryInformationProcess");
            if (!m_pfnNtQueryInformationProcess)
                return E_POINTER;
        }

        return m_pfnNtQueryInformationProcess(
            ProcessHandle,
            ProcessInformationClass,
            ProcessInformation,
            ProcessInformationLength,
            ReturnLength);
    }

protected:
    CWinMod_ntdll(): m_pfnNtQueryInformationProcess(NULL)
    {
    };

    PFN_NtQueryInformationProcess m_pfnNtQueryInformationProcess;
};


class CWinModHelper_ntdll: public CWinMod_ntdll
{
public:
    static CWinModHelper_ntdll GetInstance()
    {
        static CWinModHelper_ntdll s_Instance;
        return s_Instance;
    }

    DWORD GetParentProcessId()
    {
        HMODULE hMod = GetModuleHandle(L"ntdll.dll");
        if (!hMod)
            return 0;


        PFN_NtQueryInformationProcess pfnNtQueryInformationProcess = (PFN_NtQueryInformationProcess)::GetProcAddress(hMod, "NtQueryInformationProcess");
        if (!pfnNtQueryInformationProcess)
            return 0;


        ULONG uRetLeng = 0;
        PROCESS_BASIC_INFORMATION ProcInfo;
        LONG lRet = CWinMod_ntdll::NtQueryInformationProcess(
            ::GetCurrentProcess(),
            ProcessBasicInformation,
            &ProcInfo,
            sizeof(ProcInfo),
            &uRetLeng);
        if (ERROR_SUCCESS != lRet)
            return 0;


        return (DWORD)(DWORD_PTR)ProcInfo.Reserved3;
    }
};


NS_WINMOD_END

#endif//WINMOD_NTDLL_H