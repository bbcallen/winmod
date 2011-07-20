/**
* @file    winconsoleprocess.h
* @brief   ...
* @author  bbcallen
* @date    2011-06-23 15:02
*/

#ifndef WINCONSOLEPROCESS_H
#define WINCONSOLEPROCESS_H

#include <atlfile.h>
#include "winmodbase.h"

NS_WINMOD_BEGIN

class CWinConsoleProcess
{
public:
    CWinConsoleProcess();
    virtual ~CWinConsoleProcess() {Close();}

    HRESULT Create(LPCWSTR lpszExePath)
    {
        if (m_hSubProcess && WAIT_TIMEOUT == ::WaitForSingleObject(m_hSubProcess, 0))
            return E_FAIL;

        Close();

        HANDLE hPipeWrite = NULL;
        HANDLE hPipeRead = NULL;
        if (!::CreatePipe(&hPipeRead, &hPipeWrite, NULL, 0))
            return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

        ::SetHandleInformation(hPipeRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);


        STARTUPINFOW         si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags      = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW | STARTF_USEPOSITION;
        si.lpTitle      = m_strTitle.IsEmpty() ? NULL : ((LPWSTR)(LPCWSTR)m_strTitle);
        si.hStdInput    = hPipeRead;
        si.hStdOutput   = ::GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdError    = ::GetStdHandle(STD_ERROR_HANDLE);
        si.wShowWindow  = SW_SHOWNORMAL;
        si.dwX          = 200;


        // create process with token
        CWinPath PathExe = lpszExePath;
        if (!m_strParam.IsEmpty())
        {
            PathExe.m_strPath.Append(L" ");
            PathExe.m_strPath.Append(m_strParam);
        }


        DWORD dwCreationFlag = NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT;
        if (m_bCreateNewConsole)
            dwCreationFlag |= CREATE_NEW_CONSOLE;


        BOOL bRet = ::CreateProcess(        
            NULL,
            (LPWSTR)(LPCWSTR)PathExe.m_strPath,
            NULL,
            NULL,
            TRUE,
            dwCreationFlag,
            NULL,
            NULL,
            &si,
            &pi);
        if (!bRet)
            return GetLastError() ? AtlHresultFromLastError() : E_FAIL;


        ::WaitForInputIdle(pi.hProcess, 2000);
        ::CloseHandle(pi.hThread);


        // bring window to top
        for (int i = 0; i < 200; ++i)
        {
            if (::GetConsoleWindow() != ::GetForegroundWindow())
                break;

            ::Sleep(10);
        }
        ::BringWindowToTop(::GetConsoleWindow());
        ::SetForegroundWindow(::GetConsoleWindow());


#ifndef _ATL_MIN_CRT
        int nHandle = _open_osfhandle((INT_PTR)(HANDLE)hPipeWrite, _O_TEXT | _O_APPEND);
        if (nHandle == -1)
            return E_FAIL;

        m_pStreamToConsole = _wfdopen(nHandle, L"wt");
        if (!m_pStreamToConsole)
        {
            _close(nHandle);
            return E_FAIL;
        }
#endif


        m_hPipeRead.Attach(hPipeRead);
        m_hPipeWrite.Attach(hPipeWrite);
        m_hSubProcess.Attach(pi.hProcess);
        return S_OK;
    }

    void Close()
    {
        if (m_hSubProcess)
            ::TerminateProcess(m_hSubProcess, E_ABORT);

        m_hSubProcess.Close();
        m_hPipeRead.Close();

        if (m_pStreamToConsole)
        {
            fclose(m_pStreamToConsole);
            m_hPipeWrite.Detach();
        }
        else
        {
            m_hPipeWrite.Close();
        }
    }

    FILE* ToStream()
    {
        return m_pStreamToConsole;
    }

#ifndef _ATL_MIN_CRT
    void PrintF(LPCWSTR lpszFormat, ...)
    {
        if (!m_pStreamToConsole)
            return;

        va_list argList;
        va_start(argList, lpszFormat);

        vfwprintf(m_pStreamToConsole, lpszFormat, argList);
        fflush(m_pStreamToConsole);

        va_end(argList);
    }

    void PrintFA(LPCSTR lpszFormat, ...)
    {
        if (!m_pStreamToConsole)
            return;

        va_list argList;
        va_start(argList, lpszFormat);

        vfprintf(m_pStreamToConsole, lpszFormat, argList);
        fflush(m_pStreamToConsole);

        va_end(argList);
    }
#endif


public:
    CHandle     m_hSubProcess;
    CString     m_strTitle;
    CString     m_strParam;
    BOOL        m_bCreateNewConsole;

protected:
    CAtlFile    m_hPipeRead;
    CAtlFile    m_hPipeWrite;
    FILE*       m_pStreamToConsole;
};

inline CWinConsoleProcess::CWinConsoleProcess()
    : m_pStreamToConsole(NULL)
    , m_bCreateNewConsole(TRUE)
{
}

NS_WINMOD_END

#endif//WINCONSOLEPROCESS_H