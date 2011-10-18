/**
* @file    wintimerqueue.h
* @brief   ...
* @author  bbcallen
* @date    2011-10-15 17:49
*/

#ifndef WINTIMERQUEUE_H
#define WINTIMERQUEUE_H

#include "winmodsync.h"
#include "winmodcom.h"
#include "winthreadpooldef.h"

NS_WINMOD_BEGIN

class CWinTimerQueue: public CWinModSyncTraits
{
public:
    CWinTimerQueue();
    ~CWinTimerQueue() {Close();}

    HRESULT Create();
    // NULL for non-block,
    // INVALID_HANDLE_VALUE for block
    HRESULT Close(HANDLE hCompletionEvent = INVALID_HANDLE_VALUE);

    // WT_EXECUTEDEFAULT
    // WT_EXECUTEINIOTHREAD
    // WT_EXECUTEINUITHREAD
    // WT_EXECUTEINWAITTHREAD
    // WT_EXECUTEONLYONCE
    // WT_EXECUTEINTIMERTHREAD
    // WT_EXECUTELONGFUNCTION
    // WT_EXECUTEINPERSISTENTIOTHREAD
    // WT_EXECUTEINPERSISTENTTHREAD
    // WT_TRANSFER_IMPERSONATION
    HRESULT AddTimer(
        IWinModTimerCommand* piTimerCommand,
        DWORD dwDueTime,
        DWORD dwPeriod,
        ULONG uFlags = WT_EXECUTEDEFAULT);

    HRESULT AddTimer(
        IWinModCommand* piCommand,
        DWORD dwDueTime,
        DWORD dwPeriod,
        ULONG uFlags = WT_EXECUTEDEFAULT);

private:
    // denied
    CWinTimerQueue(const CWinTimerQueue&);
    CWinTimerQueue& operator=(const CWinTimerQueue&);

protected:
    class CWinTimerQueueTimer;

    HRESULT AddTimer(
        CWinTimerQueueTimer* pTimerCommand,
        DWORD dwDueTime,
        ULONG uFlags = WT_EXECUTEDEFAULT);

    static VOID CALLBACK WaitOrTimerCallback(
        PVOID lpParameter,
        BOOLEAN TimerOrWaitFired);


    class __declspec(uuid("591795E2-8903-4e8b-B6BD-E2B8E7BA1AB7"))
    CWinTimerQueueTimer: public IWinModTimerCommand
    {
    public:
        static CWinTimerQueueTimer* Create()
        {
            return new CWinComObject<CWinTimerQueueTimer>;
        }

        virtual BOOL STDMETHODCALLTYPE TimerNeedContinue()
        {
            if (!m_spiTimerControl)
                return FALSE;

            if (0 == m_dwPeriod ||
                CWinModBits::MatchAll(m_uTimerFlags, WT_EXECUTEONLYONCE))
                return FALSE;

            return m_spiTimerControl->TimerNeedContinue();            
        }

        virtual HRESULT STDMETHODCALLTYPE OnWinCmdExecute()
        {
            if (!m_spiCommand)
                return S_OK;

            return m_spiCommand->OnWinCmdExecute();  
        }

        WINCOM_BEGIN_COM_MAP(CWinTimerQueueTimer)
            WINCOM_INTERFACE_ENTRY(IWinModCommand);
            WINCOM_INTERFACE_ENTRY(IWinModTimerCommand);
            WINCOM_INTERFACE_ENTRY(CWinTimerQueueTimer);
        WINCOM_END_COM_MAP()

    protected:
        CWinTimerQueueTimer()
            : m_pTimerQueue(NULL)
            , m_hTimer(NULL)
            , m_dwPeriod(0)
            , m_uTimerFlags(0)
        {}

    public:
        
        CWinTimerQueue*                 m_pTimerQueue;
        CComPtr<IWinModTimerCommand>    m_spiTimerControl;
        CComPtr<IWinModCommand>         m_spiCommand;
        HANDLE                          m_hTimer;
        
        DWORD                           m_dwPeriod; // in milliseconds
        ULONG                           m_uTimerFlags;
        
    };
    typedef CComPtr<CWinTimerQueueTimer> CWinTimerQueueTimerPtr;


    typedef CAtlMap<CWinTimerQueueTimer*, CWinTimerQueueTimerPtr>  CTimerSet;


    CObjLock    m_ObjLock;
    HANDLE      m_hTimerQueue;
    CTimerSet   m_TimerSet;
};

inline CWinTimerQueue::CWinTimerQueue(): m_hTimerQueue(NULL)
{
}

inline HRESULT CWinTimerQueue::Create()
{
    Close(INVALID_HANDLE_VALUE);

    m_hTimerQueue = ::CreateTimerQueue();
    if (!m_hTimerQueue)
        return MAKE_WIN32_ERROR(GetLastError());

    return S_OK;
}

inline HRESULT CWinTimerQueue::Close(HANDLE hCompletionEvent)
{
    if (!m_hTimerQueue)
        return S_FALSE;

    HANDLE hTimerQueue = m_hTimerQueue;
    m_hTimerQueue = NULL;

    if (!::DeleteTimerQueueEx(hTimerQueue, hCompletionEvent))
        return MAKE_WIN32_ERROR(GetLastError());

    return S_OK;
}

inline HRESULT CWinTimerQueue::AddTimer(
    IWinModTimerCommand* piTimerCommand,
    DWORD dwDueTime,
    DWORD dwPeriod,
    ULONG uFlags)
{
    if (!m_hTimerQueue)
        return E_HANDLE;

    CWinTimerQueueTimerPtr spTimer = CWinTimerQueueTimer::Create();
    if (!spTimer)
        return E_OUTOFMEMORY;

    spTimer->m_pTimerQueue      = this;
    spTimer->m_spiTimerControl  = piTimerCommand;
    spTimer->m_spiCommand       = piTimerCommand,
    spTimer->m_dwPeriod         = dwPeriod;
    spTimer->m_uTimerFlags      = uFlags;

    return AddTimer(spTimer, dwDueTime, uFlags);
}

inline HRESULT CWinTimerQueue::AddTimer(
    IWinModCommand* piCommand,
    DWORD dwDueTime,
    DWORD dwPeriod,
    ULONG uFlags)
{
    if (!m_hTimerQueue)
        return E_HANDLE;

    CWinTimerQueueTimerPtr spTimer = CWinTimerQueueTimer::Create();
    if (!spTimer)
        return E_OUTOFMEMORY;

    spTimer->m_pTimerQueue      = this;
    spTimer->m_spiTimerControl  = NULL;
    spTimer->m_spiCommand       = piCommand;
    spTimer->m_dwPeriod         = dwPeriod;
    spTimer->m_uTimerFlags      = uFlags;

    return AddTimer(spTimer, dwDueTime, uFlags);
}

inline HRESULT CWinTimerQueue::AddTimer(
    CWinTimerQueueTimer* pTimerCommand,
    DWORD dwDueTime,
    ULONG uFlags)
{
    BOOL bRet = ::CreateTimerQueueTimer(
        &pTimerCommand->m_hTimer,
        m_hTimerQueue,
        WaitOrTimerCallback,
        (void*)pTimerCommand,
        dwDueTime,
        pTimerCommand->m_dwPeriod,
        uFlags);
    if (!bRet)
        return MAKE_WIN32_ERROR(GetLastError());

    {
        CObjGuard Guard(m_ObjLock);
        m_TimerSet.SetAt(pTimerCommand, pTimerCommand);
    }

    return S_OK;
}

inline VOID CALLBACK CWinTimerQueue::WaitOrTimerCallback(
    PVOID lpParameter,
    BOOLEAN TimerOrWaitFired)
{
    if (!lpParameter)
        return;

    CComPtr<CWinTimerQueueTimer> spTimer = (CWinTimerQueueTimer*) lpParameter;
    if (!spTimer->m_pTimerQueue || !spTimer->m_spiCommand)
        return;

    CWinTimerQueue* pTimerQueue = spTimer->m_pTimerQueue;

    {
        CObjGuard Guard(pTimerQueue->m_ObjLock);
        if (!pTimerQueue->m_TimerSet.Lookup(spTimer))
        {   // some delayed timer may occur even after we call DeleteTimerQueueTimer
            //assert(!"invalid timer");
            return;
        }
    }


    spTimer->m_spiCommand->OnWinCmdExecute();
    ::Sleep(1000);

    
    if ((spTimer->m_spiTimerControl && !spTimer->m_spiTimerControl->TimerNeedContinue()) ||
        (0 == spTimer->m_dwPeriod) ||
        (CWinModBits::MatchAll(spTimer->m_uTimerFlags, WT_EXECUTEONLYONCE)))
    {
        BOOL bRet = ::DeleteTimerQueueTimer(
            pTimerQueue->m_hTimerQueue, 
            spTimer->m_hTimer,
            NULL);

        if (bRet || ERROR_IO_PENDING == GetLastError())
        {
            CObjGuard Guard(pTimerQueue->m_ObjLock);
            pTimerQueue->m_TimerSet.RemoveKey(spTimer);
        }
    }
}

NS_WINMOD_END

#endif//WINTIMERQUEUE_H