/**
* @file    winthreadpool.cpp
* @brief   ...
* @author  bbcallen
* @date    2011-03-24 20:58
*/

#include "stdafx.h"
#include "winthreadpool.h"

//////////////////////////////////////////////////////////////////////////
HRESULT CWinMessageQueue::Initialize(HANDLE hNotifyStop)
{
    assert(hNotifyStop);
    if (!hNotifyStop)
        return E_HANDLE;

    m_hNotifyStop = hNotifyStop;
    m_hMsgNotEmpty.Create(NULL, FALSE, FALSE, NULL); // auto reset, init false
    
    return S_OK;
}

BOOL CWinMessageQueue::PopMsg(CWinThreadMsg& Msg)
{
    assert(0 == GetWaitingThreadsCount());
    CObjGuard Guard(m_ObjLock);
    
    if (!m_PrioMsgQueue.IsEmpty())
    {
        POSITION pos = m_PrioMsgQueue.GetTailPosition();
        Msg = m_PrioMsgQueue.GetValueAt(pos);
        m_PrioMsgQueue.RemoveAt(pos);
        return TRUE;
    }

    if (!m_MsgQueue.IsEmpty())
    {
        Msg = m_MsgQueue.RemoveHead();
        return TRUE;
    }

    m_hMsgNotEmpty.Reset();
    return FALSE;
}

HRESULT STDMETHODCALLTYPE CWinMessageQueue::PostMsg(CWinThreadMsg Msg)
{
    CObjGuard Guard(m_ObjLock);

    if (Msg.m_dwPriority)
    {
        m_PrioMsgQueue.Insert(Msg.m_dwPriority, Msg);
    }
    else
    {
        m_MsgQueue.AddTail(Msg);
    }

    m_hMsgNotEmpty.Set();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CWinMessageQueue::ResetMsgQueue()
{
    assert(0 == m_lWaitingThreadCount && "reset msg queue only if no threads are waiting");

    CObjGuard Guard(m_ObjLock);

    m_MsgQueue.RemoveAll();
    m_hMsgNotEmpty.Reset();

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// called by thread
BOOL STDMETHODCALLTYPE CWinMessageQueue::IsEmpty()
{
    assert(m_hNotifyStop);
    assert(m_hMsgNotEmpty);

    CObjGuard Guard(m_ObjLock);

    if (!m_PrioMsgQueue.IsEmpty())
        return FALSE;

    if (!m_MsgQueue.IsEmpty())
        return FALSE;

    return TRUE;
}

HRESULT STDMETHODCALLTYPE CWinMessageQueue::WaitForMsg(CWinThreadMsg& rMsg, DWORD dwWorkerID)
{
    assert(m_hNotifyStop);
    assert(m_hMsgNotEmpty);

    CWinRefGuard RefGuard(m_lWaitingThreadCount);

    HANDLE HandleArray[] = {m_hNotifyStop, m_hMsgNotEmpty};
    while (WAIT_OBJECT_0 + 1 == ::WaitForMultipleObjects(_countof(HandleArray), HandleArray, FALSE, INFINITE))
    {
        CObjGuard Guard(m_ObjLock);

        if (!m_PrioMsgQueue.IsEmpty())
        {
            // CRBMultiMap insert new node at most left
            // so we treat GetTailPosition() as head, to keep sorting stable
            POSITION pos = m_PrioMsgQueue.GetTailPosition();
            assert(pos);
            rMsg = m_PrioMsgQueue.GetValueAt(pos);
            if (CWinThreadMsg::em_Commit == rMsg.m_MsgType)
            {
                m_hMsgNotEmpty.Set();   // prepare for being actived again
                return S_OK;            // do not remove COMMIT msg, so it can block following msg
            }

            m_PrioMsgQueue.RemoveAt(pos);
        }
        else if (!m_MsgQueue.IsEmpty())
        {
            rMsg = m_MsgQueue.GetHead();
            if (CWinThreadMsg::em_Commit == rMsg.m_MsgType)
            {
                m_hMsgNotEmpty.Set();   // prepare for being actived again
                return S_OK;            // do not remove COMMIT msg, so it can block following msg
            }

            m_MsgQueue.RemoveHeadNoReturn();
        }
        else
        {
            m_hMsgNotEmpty.Reset(); // set empty, and wait for any msg
            continue;
        }


        if (!m_PrioMsgQueue.IsEmpty() || !m_MsgQueue.IsEmpty())
            m_hMsgNotEmpty.Set();


        return S_OK;
    }

    return E_ABORT;
}







//////////////////////////////////////////////////////////////////////////
HRESULT CWinWorkerThread::Initialize(
    HANDLE              hNotifyStop,
    IWinMessageQueue*   piMessageQueue,
    DWORD               dwWorkerID,
    IWinWorker*         piWorker,
    DWORD               dwThreadPoolID)
{
    assert(!m_hNotifyStop);
    assert(!m_piMessageQueue);
    assert(!m_piWorker);
    assert(hNotifyStop);
    assert(piMessageQueue);
    assert(piWorker);
    m_hNotifyStop       = hNotifyStop;
    m_piMessageQueue    = piMessageQueue;
    m_dwThreadPoolID    = dwThreadPoolID;
    m_dwWorkerID        = dwWorkerID;
    m_piWorker          = piWorker;

    m_hNotifyActive.Create(NULL, FALSE, FALSE, NULL);   // auto reset, init false
    m_hDeactived.Create(NULL, TRUE, TRUE, NULL);        // manual reset, init true

    return S_OK;
}

BOOL CWinWorkerThread::IsNotifiedStop()
{
    assert(m_hNotifyStop);
    if (m_hNotifyStop)
        return TRUE;

    if (WAIT_TIMEOUT != ::WaitForSingleObject(m_hNotifyStop, 0))
        return TRUE;

    return FALSE;
}


DWORD STDMETHODCALLTYPE CWinWorkerThread::Run()
{
    DWORD dwRet = DoRun();

    if (m_hDeactived)
        m_hDeactived.Close();

    return dwRet;
}

DWORD CWinWorkerThread::DoRun()
{
    assert(m_piMessageQueue);
    assert(m_piWorker);

    DWORD dwThreadPoolID = m_dwThreadPoolID;

    m_hDeactived.Reset();
    while (IsNotifiedStop())
    {
        while (IsNotifiedStop())
        {
            CWinThreadMsg Msg;
            assert(m_piMessageQueue);
            HRESULT hr = m_piMessageQueue->WaitForMsg(Msg, m_dwWorkerID);
            if (FAILED(hr))
            {
                m_hDeactived.Set();
                return hr;
            }


            if (CWinThreadMsg::em_Commit == Msg.m_MsgType)
                break;
            if (CWinThreadMsg::em_Exit == Msg.m_MsgType)
                return E_ABORT;


            assert(m_piWorker);
            m_piWorker->OnWinWorkerExecute(m_hNotifyStop, m_dwWorkerID, Msg);
        }

        // wait until being actived
        m_hDeactived.Set();
       
        HANDLE HandleArray[] = {m_hNotifyStop, m_hNotifyActive};
        if (WAIT_OBJECT_0 + 1 != ::WaitForMultipleObjects(_countof(HandleArray), HandleArray, FALSE, INFINITE))
            return E_ABORT;

        m_hDeactived.Reset();
    }

    return E_ABORT;
}














//////////////////////////////////////////////////////////////////////////
CWinThreadPool::~CWinThreadPool()
{
    CObjGuard Guard(m_ObjLock);

    while (!m_ThreadList.IsEmpty())
    {
        CWinWorkerThread* pThread = m_ThreadList.RemoveHead();
        assert(pThread);
        delete pThread;
    }
}



HRESULT CWinThreadPool::StartupSingleWorker(
    IWinWorker* piSingleWorker,
    size_t nThreadCount,
    HANDLE hNotifyStop,
    DWORD  dwThreadPoolID)
{
    assert(piSingleWorker);
    assert(nThreadCount);
    assert(hNotifyStop);
    assert(nThreadCount <= MAXIMUM_WAIT_OBJECTS);

    if (!nThreadCount || !hNotifyStop || nThreadCount > MAXIMUM_WAIT_OBJECTS)
        return E_INVALIDARG;

    CObjGuard Guard(m_ObjLock);

    CAtlArray<IWinWorker*> WorkerPtrArray;
    for (size_t i = 0; i < nThreadCount; ++i)
    {
        WorkerPtrArray.Add(piSingleWorker);
    }
    return Startup(WorkerPtrArray.GetData(), WorkerPtrArray.GetCount(), hNotifyStop, dwThreadPoolID);
}



HRESULT CWinThreadPool::Startup(
    IWinWorker** WorkerPtrArray,
    size_t nWorkerCount,
    HANDLE hNotifyStop,
    DWORD  dwThreadPoolID)
{
    assert(WorkerPtrArray);
    assert(nWorkerCount);
    assert(hNotifyStop);
    assert(nWorkerCount <= MAXIMUM_WAIT_OBJECTS);

    if (!nWorkerCount || !hNotifyStop || nWorkerCount > MAXIMUM_WAIT_OBJECTS)
        return E_INVALIDARG;

    assert(!m_hNotifyStop);
    if (m_hNotifyStop)
        return S_FALSE;

    CObjGuard Guard(m_ObjLock);

    m_dwThreadPoolID = dwThreadPoolID;
    m_hNotifyStop    = hNotifyStop;
    HRESULT hr = m_MessageQueue.Initialize(m_hNotifyStop);
    if (FAILED(hr))
        return hr;

    for (size_t i = 0; i < nWorkerCount; ++i)
    {
        assert(WorkerPtrArray[i]);
        CWinWorkerThread* pNewThread = new CWinWorkerThread;
        m_ThreadList.AddTail(pNewThread);
        m_UnusedThread.AddTail(pNewThread);
        pNewThread->Initialize(hNotifyStop, &m_MessageQueue, DWORD(i), WorkerPtrArray[i], m_dwThreadPoolID);
    }

    hr = ActiveAll();
    if (FAILED(hr))
        return hr;

    return S_OK;
}

DWORD CWinThreadPool::WaitForAllExit(DWORD dwMaxWait)
{
    CAtlArray<HANDLE> HandleArray;

    // fetch handle of threads 
    {
        CObjGuard Guard(m_ObjLock);

        POSITION pos = m_ThreadList.GetHeadPosition();
        for (NULL; pos; m_ThreadList.GetNext(pos))
        {
            CWinWorkerThread* pThread = m_ThreadList.GetAt(pos);
            assert(pThread);
            if (pThread->m_hThread)
                HandleArray.Add(pThread->m_hThread);
        }

        if (WAIT_TIMEOUT != ::WaitForSingleObject(m_hNotifyStop, 0))
            m_UnusedThread.RemoveAll();
    }


    assert(m_lActivedThreadCount == HandleArray.GetCount());


    DWORD dwWaitRet = 0;
    if (HandleArray.IsEmpty())
        dwWaitRet = WAIT_OBJECT_0;
    else if (1 == HandleArray.GetCount())
        dwWaitRet = ::WaitForSingleObject(HandleArray[0], dwMaxWait);
    else
        dwWaitRet = ::WaitForMultipleObjects((DWORD)HandleArray.GetCount(), HandleArray.GetData(), TRUE, dwMaxWait);


    assert(WAIT_FAILED != dwWaitRet);
    return dwWaitRet;
}


DWORD CWinThreadPool::WaitForAllDeactived(DWORD dwMaxWait)
{
    while (TRUE)
    {
        CAtlArray<HANDLE> HandleArray;
        size_t nUnusedThreads = 0;

        // fetch handle of threads 
        {
            CObjGuard Guard(m_ObjLock);

            POSITION pos = m_ThreadList.GetHeadPosition();
            for (NULL; pos; m_ThreadList.GetNext(pos))
            {
                CWinWorkerThread* pThread = m_ThreadList.GetAt(pos);
                assert(pThread);
                HandleArray.Add(pThread->GetEventDeactived());
            }

            nUnusedThreads = m_UnusedThread.GetCount();
        }


        DWORD dwWaitRet = 0;
        if (HandleArray.IsEmpty())
            dwWaitRet = WAIT_OBJECT_0;
        else if (1 == HandleArray.GetCount())
            dwWaitRet = ::WaitForSingleObject(HandleArray[0], dwMaxWait);
        else
            dwWaitRet = ::WaitForMultipleObjects((DWORD)HandleArray.GetCount(), HandleArray.GetData(), TRUE, dwMaxWait);


        // in case new thread is created after fetching handle
        {
            CObjGuard Guard(m_ObjLock);

            if (nUnusedThreads == m_UnusedThread.GetCount())
                return dwWaitRet;
        }
    }
}


BOOL CWinThreadPool::AreAllDeactived()
{
    return WAIT_TIMEOUT != WaitForAllDeactived(0);
}


HRESULT CWinThreadPool::ActiveAll()
{
    CObjGuard Guard(m_ObjLock);

    if (!AreAllDeactived())  /// if any thread is actived
        return E_PENDING;

    POSITION pos = m_ThreadList.GetHeadPosition();
    for (NULL; pos; m_ThreadList.GetNext(pos))
    {
        CWinWorkerThread* pThread = m_ThreadList.GetAt(pos);
        assert(pThread);
        pThread->Active();
    }

    return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// called by owner
HRESULT CWinThreadPool::PostNormalMsg(void* pvParam, DWORD dwPriority)
{
    return PostMsg(CWinThreadMsg(CWinThreadMsg::em_Normal, pvParam, dwPriority));
}

HRESULT CWinThreadPool::PostCommitMsg(void* pvParam)
{
    return PostMsg(CWinThreadMsg(CWinThreadMsg::em_Commit, pvParam));
}

HRESULT CWinThreadPool::PostExitMsg(void* pvParam)
{
    return PostMsg(CWinThreadMsg(CWinThreadMsg::em_Exit, pvParam));
}

HRESULT CWinThreadPool::PostMsg(CWinThreadMsg Msg)
{
    if (WAIT_TIMEOUT != ::WaitForSingleObject(m_hNotifyStop, 0))
        return E_ABORT;

    HRESULT hr = m_MessageQueue.PostMsg(Msg);
    if (FAILED(hr))
        return hr;

    {   // start new thread, only if no more idle thread
        CObjGuard Guard(m_ObjLock);

        if (WAIT_TIMEOUT != ::WaitForSingleObject(m_hNotifyStop, 0))
            return E_ABORT;

        if (!m_UnusedThread.IsEmpty() && !m_MessageQueue.GetWaitingThreadsCount())
        {
            CWinWorkerThread* pWorkThread = m_UnusedThread.RemoveHead();
            pWorkThread->StartRunning();

            ::InterlockedIncrement(&m_lActivedThreadCount);
        }
    }

    return S_OK;
}

HRESULT CWinThreadPool::ResetMsgQueue()
{
    return m_MessageQueue.ResetMsgQueue();
}

DWORD CWinThreadPool::GetWaitingThreadsCount()
{
    return m_MessageQueue.GetWaitingThreadsCount();
}

DWORD CWinThreadPool::GetMaxThreadsCount()
{
    return (DWORD)m_ThreadList.GetCount();
}