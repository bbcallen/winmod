/**
* @file    winrunnable.h
* @brief   ...
* @author  bbcallen
* @date    2009-03-27  15:54
*/

#ifndef WINRUNNABLE_H
#define WINRUNNABLE_H

#include <atlsync.h>
#include <atlbase.h>
#include "winmod\winthread.h"

NS_WINMOD_BEGIN

/**
* @brief abstract class for thread
*/
class AWinRunnable: public IWinRunnable
{
public:
    AWinRunnable();
    virtual ~AWinRunnable();


    void    CloseThread();
    HRESULT StartRunning();


    void    NotifyStopRunning();
    DWORD   WaitUntilNotifiedStop(DWORD dwMilliseconds = INFINITE);
    BOOL    IsNotifiedStopRunning();
    void    ForceStopRunning(DWORD dwMilliseconds, DWORD dwTerminateCode = 1);

protected:

    // override this function to implement thread routine
    virtual DWORD STDMETHODCALLTYPE Run() = 0;

protected:
    CWinThread  m_hThread;  
    ATL::CEvent m_hNotifyStop;  ///< indicate whether need stop thread
};



inline AWinRunnable::AWinRunnable()
{

}


inline void AWinRunnable::CloseThread()
{
    m_hThread.Close();
}


inline HRESULT AWinRunnable::StartRunning()
{
    if (m_hThread && !m_hThread.IsExit())
    {
        return AtlHresultFromWin32(ERROR_ALREADY_INITIALIZED);
    }
        
    m_hThread.Close();
    m_hNotifyStop.Close();
    m_hNotifyStop.Create(NULL, TRUE, FALSE, NULL);

    return m_hThread.Create(this);
}


inline void AWinRunnable::NotifyStopRunning()
{
    if (m_hNotifyStop)
        m_hNotifyStop.Set();
};


inline DWORD AWinRunnable::WaitUntilNotifiedStop(DWORD dwMilliseconds)
{
    return ::WaitForSingleObject(m_hNotifyStop, dwMilliseconds);
};


inline BOOL AWinRunnable::IsNotifiedStopRunning()
{
    return WAIT_TIMEOUT != WaitUntilNotifiedStop(0);
}


inline void AWinRunnable::ForceStopRunning(DWORD dwMilliseconds, DWORD dwTerminateCode)
{
    NotifyStopRunning();
    DWORD dwRet = m_hThread.WaitExit(dwMilliseconds);
    if (WAIT_TIMEOUT == dwRet)
        m_hThread.Terminate(dwTerminateCode);
}





NS_WINMOD_END

#endif//WINRUNNABLE_H