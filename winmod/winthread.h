/**
* @file    winthreadbase.h
* @brief   ...
* @author  bbcallen
* @date    2009-03-27  16:06
*/

#ifndef WINTHREADBASE_H
#define WINTHREADBASE_H

#include <assert.h>
#include <atlbase.h>
#include "winmodbase.h"

NS_WINMOD_BEGIN

class IWinRunnable
{
public:
    virtual DWORD STDMETHODCALLTYPE Run() = 0;
};






class CWinModThread: public CHandle
{
public:

    HRESULT Create(IWinRunnable* piRunnable);
    HRESULT CreateNoCRT(IWinRunnable* piRunnable);

    DWORD   WaitExit(DWORD dwMilliseconds);
    BOOL    IsExit();
    //void    Terminate(DWORD dwExitCode = 1);

    DWORD   Suspend();
    DWORD   Resume();


    BOOL    SetPriority(int nPriority);
    int     GetPriority();


    DWORD   GetExitCode(DWORD dwDefaultCode = 0);

    // call in thread
    static void SetThreadName(LPCSTR lpszThreadName, DWORD dwThreadID = -1);

protected:
    CStringA m_strThreadName;

private:

    // for CRT thread API
    static unsigned int __stdcall RunThreadFunc(void* pParam);

    // for Win32 thread API
    static DWORD WINAPI RunThreadFuncNoCRT(LPVOID pParam);
};













#ifdef _ATL_MIN_CRT

inline HRESULT CWinModThread::Create(IWinRunnable* piRunnable)
{
    return CreateNoCRT(piRunnable);
}

#else

inline HRESULT CWinModThread::Create(IWinRunnable* piRunnable)
{
    assert(!m_h);
    assert(piRunnable);
    if (m_h)
        return AtlHresultFromWin32(ERROR_ALREADY_INITIALIZED);

    if (!piRunnable)
        return E_POINTER;

    // use _beginthreadex for initialization of c runtime lib
    m_h = (HANDLE)_beginthreadex(NULL, 0, RunThreadFunc, piRunnable, CREATE_SUSPENDED, NULL);
    if (!m_h)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    Resume();
    return S_OK;
}

#endif//_ATL_MIN_CRT


inline HRESULT CWinModThread::CreateNoCRT(IWinRunnable* piRunnable)
{
    assert(!m_h);
    assert(piRunnable);
    if (m_h)
        return AtlHresultFromWin32(ERROR_ALREADY_INITIALIZED);

    if (!piRunnable)
        return E_POINTER;

    m_h = ::CreateThread(NULL, 0, RunThreadFuncNoCRT, piRunnable, CREATE_SUSPENDED, NULL);
    if (!m_h)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    Resume();
    return S_OK;
}


inline DWORD CWinModThread::WaitExit(DWORD dwMilliseconds)
{
    return ::WaitForSingleObject(m_h, dwMilliseconds);
}

inline BOOL CWinModThread::IsExit()
{
    return WAIT_TIMEOUT != WaitExit(0);
}

//inline void CWinThread::Terminate(DWORD dwExitCode)
//{
//    assert(m_h);
//    ::TerminateThread(m_h, dwExitCode);
//}


inline DWORD CWinModThread::Suspend()
{
    assert(m_h);
    return ::SuspendThread(m_h);
}

inline DWORD CWinModThread::Resume()
{
    return ::ResumeThread(m_h);
}


inline BOOL CWinModThread::SetPriority(int nPriority)
{
    assert(m_h);
    return ::SetThreadPriority(m_h, nPriority);
}

inline int CWinModThread::GetPriority()
{
    assert(m_h);
    return ::GetThreadPriority(m_h);
}


inline DWORD CWinModThread::GetExitCode(DWORD dwDefaultCode)
{
    assert(m_h);
    DWORD dwExitCode;
    if (::GetExitCodeThread(m_h, &dwExitCode))
        return dwExitCode;

    return dwDefaultCode;
}

inline void CWinModThread::SetThreadName(LPCSTR lpszThreadName, DWORD dwThreadID)
{
#ifdef DEBUG
    typedef struct tagTHREADNAME_INFO 
    {
        DWORD dwType;		// must be 0x1000 
        LPCSTR szName;		// pointer to name (in same addr space) 
        DWORD dwThreadID;	// thread ID (-1 caller thread) 
        DWORD dwFlags;		// reserved for future use, must be zero 
    } THREADNAME_INFO;

    // Setting a Thread Name (Unmanaged)
    // http://msdn.microsoft.com/en-us/library/xcb2z8hs%28v=vs.71%29.aspx
    __try
    {
        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = lpszThreadName;
        info.dwThreadID = dwThreadID;
        info.dwFlags = 0;

        RaiseException(
            0x406D1388,
            0,
            sizeof(info)/sizeof(DWORD),
            (DWORD*)&info);
    }
    __except(EXCEPTION_CONTINUE_EXECUTION)
    {
    }

#else
    UNREFERENCED_PARAMETER(dwThreadID);
    UNREFERENCED_PARAMETER(lpszThreadName);
#endif
}


inline unsigned int __stdcall CWinModThread::RunThreadFunc(void* pParam)
{
    DWORD dwRet = RunThreadFuncNoCRT(pParam);

#ifndef _ATL_MIN_CRT
    _endthreadex(dwRet);
#endif//_ATL_MIN_CRT

    return DWORD(dwRet);
}

inline DWORD WINAPI CWinModThread::RunThreadFuncNoCRT(LPVOID pParam)
{
    IWinRunnable* pThis = (IWinRunnable*)pParam;

    if (pThis)
        return pThis->Run();

    return 0;
}













class CWinRefGuard
{
public:
    CWinRefGuard(volatile LONG& rlRef): m_rlRef(rlRef)
    {
        ::InterlockedIncrement(&m_rlRef);
    }

    ~CWinRefGuard()
    {
        ::InterlockedDecrement(&m_rlRef);
    }

protected:
    volatile LONG& m_rlRef;
};








NS_WINMOD_END

#endif//WINTHREADBASE_H