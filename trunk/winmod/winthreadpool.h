/**
* @file    winthreadpool.h
* @brief   ...
* @author  bbcallen
* @date    2011-03-01 10:01
*/

#ifndef WINTHREADPOOL_H
#define WINTHREADPOOL_H

#include <assert.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include "winthreadpooldef.h"
#include "winmodsync.h"

NS_WINMOD_BEGIN


/*
 
 Thread State Transition Diagram


               +--------------+
        +----->| wait for msg |<-----------+
        |      +--------------+            |
        |              |                   |
        |              |                   |
        |              |  Pool::PostMsg    |
        |              |                   |
        |              V                   |
        |         +---------+     +-----------------+
        |         | Execute |     | wait for active |
        |         +---------+     +-----------------+
        |          V       V               A
        |   normal |       | commit        |
        |          |       |               |
        +----------+       +---------------+


*/


//////////////////////////////////////////////////////////////////////////
class CWinMessageQueue: public IWinMessageQueue, public CWinModSyncTraits
{
public:
    typedef CAtlList<CWinThreadMsg>             CMsgQueue;
    typedef CRBMultiMap<DWORD, CWinThreadMsg>   CPrioMsgQueue;

    CWinMessageQueue(): m_hNotifyStop(NULL), m_lWaitingThreadCount(0)
    {}

    HRESULT Initialize(HANDLE hNotifyStop);

    // should not be called while running
    BOOL    PopMsg(CWinThreadMsg& rMsg);

    HRESULT STDMETHODCALLTYPE PostMsg(CWinThreadMsg Msg);
    HRESULT STDMETHODCALLTYPE ResetMsgQueue();

    //////////////////////////////////////////////////////////////////////////
    // called by thread
    BOOL    STDMETHODCALLTYPE IsEmpty();
    HRESULT STDMETHODCALLTYPE WaitForMsg(CWinThreadMsg& rMsg, DWORD dwWorkerID = ULONG_MAX);
    DWORD   STDMETHODCALLTYPE GetWaitingThreadsCount() {return m_lWaitingThreadCount;}

public:
    HANDLE  GetEventWakeUp() {return m_hMsgNotEmpty;}

protected:
    CObjLock        m_ObjLock;
    HANDLE          m_hNotifyStop;      // not own this event
    ATL::CEvent     m_hMsgNotEmpty;
    CMsgQueue       m_MsgQueue;         // priority = 0
    CPrioMsgQueue   m_PrioMsgQueue;     // priority > 0

    volatile LONG   m_lWaitingThreadCount;
};





//////////////////////////////////////////////////////////////////////////
class CWinWorkerThread: public AWinRunnable
{
public:
    CWinWorkerThread()
        : m_piWorker(NULL)
        , m_piMessageQueue(NULL)
        , m_hNotifyStop(NULL)
    {}

    HRESULT Initialize(
        HANDLE              hNotifyStop,
        IWinMessageQueue*   piMessageQueue,
        DWORD               dwWorkerID,
        IWinWorker*         piWorker,
        LPCSTR              lpszThreadPoolName);

    HANDLE  GetEventDeactived() {return m_hDeactived;}
    HRESULT Active() {m_hNotifyActive.Set(); return S_OK;}
    BOOL    IsNotifiedStop();

protected:
    virtual DWORD STDMETHODCALLTYPE Run();

    DWORD DoRun();

protected:
    CStringA            m_strThreadPoolName;
    volatile DWORD      m_dwWorkerID;       // zero-based worker id
    IWinWorker*         m_piWorker;
    IWinMessageQueue*   m_piMessageQueue;
    HANDLE              m_hNotifyStop;      // not own this event
    ATL::CEvent         m_hNotifyActive;    // active signal,  set by ThreadPool
    ATL::CEvent         m_hDeactived;       // deactive event, set by Run()

private:
    CWinWorkerThread(const CWinWorkerThread&);
    CWinWorkerThread& operator=(const CWinWorkerThread&);
};


//////////////////////////////////////////////////////////////////////////
class CWinThreadPool: public CWinModSyncTraits
{
public:
    typedef CAtlList<CWinWorkerThread*> CThreadList;

    CWinThreadPool(): m_hNotifyStop(NULL), m_lActivedThreadCount(0)
    {}
    virtual ~CWinThreadPool();

    //////////////////////////////////////////////////////////////////////////
    // called by master
    // single worker for all threads
    HRESULT StartupSingleWorker(
        IWinWorker* piSingleWorker,
        size_t nThreadCount,
        HANDLE hNotifyStop,
        LPCSTR lpszThreadPoolName);

    // one worker per thread
    HRESULT Startup(
        IWinWorker** WorkerPtrArray,
        size_t nWorkerCount,
        HANDLE hNotifyStop,
        LPCSTR lpszThreadPoolName);

    DWORD   WaitForAllExit(DWORD dwMaxWait = INFINITE);

private:
    // INFINITE may cause dead-lock when exit process
    DWORD   WaitForAllDeactived(DWORD dwMaxWait = INFINITE);

public:
    BOOL    IsIdle();
    BOOL    AreAllDeactived();
    HRESULT ActiveAll();


    //////////////////////////////////////////////////////////////////////////
    // called by owner
    HRESULT PostNormalMsg(void* pvParam, DWORD dwPriority);
    HRESULT PostCommitMsg(void* pvParam);
    HRESULT PostExitMsg(void* pvParam);
    HRESULT PostMsg(CWinThreadMsg Msg);
    HRESULT ResetMsgQueue();
    DWORD   GetWaitingThreadsCount();
    DWORD   GetMaxThreadsCount();

protected:
    CObjLock            m_ObjLock;
    HANDLE              m_hNotifyStop;  ///< not own this event
    CWinMessageQueue    m_MessageQueue;
    CThreadList         m_ThreadList;
    CThreadList         m_UnusedThread;
    CStringA            m_strThreadPoolName;
    
    volatile LONG       m_lActivedThreadCount;

private:
    CWinThreadPool(const CWinThreadPool&);
    CWinThreadPool& operator=(const CWinThreadPool&);
};

NS_WINMOD_END

#endif//WINTHREADPOOL_H