/**
* @file    winthreadgroup.h
* @brief   ...
* @author  bbcallen
* @date    2011-05-22 20:33
*/

#ifndef WINTHREADGROUP_H
#define WINTHREADGROUP_H

#include <assert.h>
#include <atlbase.h>
#include "winobjpool.h"
#include "winthreadpool.h"

NS_WINMOD_BEGIN

class __declspec(uuid("E7F5995F-5540-489a-992B-48006E2B5161"))
IWinModCommand: public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE OnWinCmdExecute() = 0;
};



class CWinThreadGroup
    : public IWinWorker
    , public CWinModSyncTraits
{
public:
    CWinThreadGroup(): m_hNotifyStop(NULL)
    {}
    virtual ~CWinThreadGroup() {}


    HRESULT Startup(size_t nWorkerThreadCount, HANDLE hNotifyStop, DWORD dwCmdTargetID)
    {
        m_hNotifyStop = hNotifyStop;
        return m_ThreadPool.StartupSingleWorker(this, nWorkerThreadCount, hNotifyStop, dwCmdTargetID);
    }

    HRESULT WaitForAllExit(DWORD dwMaxWait = INFINITE)
    {
        return m_ThreadPool.WaitForAllExit(dwMaxWait);
    }

    HRESULT PostCommand(IWinModCommand* piCommand, DWORD dwPriority = 0)
    {
        if (IsNotifiedStop())
            return E_ABORT;

        POSITION pos = PushCmd(piCommand);
        return m_ThreadPool.PostNormalMsg(pos, dwPriority);
    }

    BOOL IsNotifiedStop()
    {
        DWORD dwWaitRet = ::WaitForSingleObject(m_hNotifyStop, 0);
        return dwWaitRet != WAIT_TIMEOUT;
    }

protected:
    // IWinWorker::OnWinWorkerExecute
    HRESULT STDMETHODCALLTYPE OnWinWorkerExecute(HANDLE hNotifyStop, DWORD dwWorkerID, CWinThreadMsg& ThreadMsg)
    {
        if (ThreadMsg.m_MsgType != CWinThreadMsg::em_Normal)
            return S_OK;

        CComPtr<IWinModCommand> spiCommand;
        PopCmd((POSITION)ThreadMsg.m_pvParam, &spiCommand);
        assert(spiCommand);
        if (!spiCommand)
            return S_OK;

        return InternalExecute(hNotifyStop, spiCommand);
    }


protected:
    HRESULT InternalExecute(HANDLE hNotifyStop, IWinModCommand* piCommand)
    {
        assert(piCommand);
        return piCommand->OnWinCmdExecute();
    }

    POSITION PushCmd(IWinModCommand* pMsg)
    {
        CObjGuard Guard(m_CmdListLock);

        return m_CmdList.AddTail(pMsg);
    }

    void PopCmd(POSITION pos, IWinModCommand** ppiCmd)
    {
        assert(ppiCmd);
        assert(!*ppiCmd);
        CObjGuard Guard(m_CmdListLock);

        m_CmdList.GetAt(pos).CopyTo(ppiCmd);
        m_CmdList.RemoveAt(pos);
    }


public:
    CWinThreadPool  m_ThreadPool;

protected:
    typedef CAtlList<CComPtr<IWinModCommand> > CCmdList;

    CObjLock        m_CmdListLock;
    CCmdList        m_CmdList;

    HANDLE          m_hNotifyStop;      ///< not own this event
};















/*
    CWinThreadAsyncCallQueue is some kind like Async I/O


      ( every caller call one or more cmd )
      ( both parameters and "yield" information should be placed in cmd )


    +----------+ -> cmd1 -> +=======================================+
    |   ch-1   |            |              Async-Call               |
    +----------+ <- rsp1 <- +=============+=========================+
                            |             |       ThreadPool        |
    +----------+ -> cmd2 -> |      +->>>  |                         |
    |   ch-2   |            |      |      |                         |
    +----------+ <- rsp2 <- |  +-------+  | -> cmd1 -> +----------+ |
                            |  | cmd3  |  |            | worker-1 | |
    +----------+ -> cmd3 -> |  +-------+  | <- rsp1 <- +----------+ |
    |   ch-3   | -> cmd4 -> |  | cmd4  |  |                         |
    |          |            |  +-------+  | -> cmd2 -> +----------+ |
    |          | <- rsp3 <- |      |      |            | worker-2 | |
    +----------+ <- rsp4 <- |  >>>-+      | <- rsp2 <- +----------+ |
                            |             |                         |
                            +---------------------------------------+


Example:

{
    CWinAsyncCallQueue acqDoSomehing;


    // alloc channel
    CWinAsyncCall AsyncCaller;
    HRESULT hr = AsyncCaller.AllocChannel(&acqDoSomehing, ::GetCurrentThreadId());
    if (FAILED(hr))
        return hr;


    AsyncCaller.AsyncCall(cmd1);        // post async call
    AsyncCaller.AsyncCall(cmd2);
    AsyncCaller.AsyncCall(cmd3);
    AsyncCaller.AsyncCall(cmd4);
    AsyncCaller.AsyncCall(cmd5);

    AsyncCaller.WaitChannelRet(rsp1);   // wait response of async call
    AsyncCaller.WaitChannelRet(rsp2);

    AsyncCaller.AsyncCall(cmd5);        // continue post
    AsyncCaller.AsyncCall(cmd6);


    AsyncCaller.WaitChannelRet(rsp3);

        ...

    AsyncCaller.WaitChannelRet(rsp5);
    AsyncCaller.WaitChannelRet(rsp6);   // wait last response


    // auto free channel
}
*/

class CWinAsyncCallQueue
    : public IWinWorker
    , public CWinModSyncTraits
{
public:

    class CCmdItem
    {
    public:
        CCmdItem(): m_dwChannelID(0), m_bWatchRsp(TRUE) {}

        CComPtr<IWinModCommand> m_spiCmd;
        DWORD                   m_dwChannelID;
        BOOL                    m_bWatchRsp;
    };




    class CCallChannel
    {
    public:
        typedef CAtlMap<POSITION, DWORD>    CCmdMap;

        CCallChannel(): m_lAsyncCallCount(0) {}
        HRESULT Initialize(HANDLE m_hNotifyStop)
        {
            assert(m_hNotifyStop);
            return m_RspMsgQueue.Initialize(m_hNotifyStop);
        }

        HRESULT PushCmd(POSITION pos)
        {
            // record cmd of this channel
            m_CmdMap.SetAt(pos, 0);
            return S_OK;
        }

        HRESULT PostRsp(POSITION pos)
        {
            // check if rsp match any cmd of this channel
            CCmdMap::CPair* pPair = m_CmdMap.Lookup(pos);
            if (!pPair)
                return S_FALSE;

            m_CmdMap.RemoveAtPos(pPair);

            return m_RspMsgQueue.PostMsg(
                CWinThreadMsg(CWinThreadMsg::em_Normal, pos, 0));
        }

        CCmdMap             m_CmdMap;
        CWinMessageQueue    m_RspMsgQueue;      // worker -> caller
        LONG                m_lAsyncCallCount;
    };




    CWinAsyncCallQueue()
        : m_hNotifyStop(NULL)
        , m_lLastChannel(0)
    {}
    virtual ~CWinAsyncCallQueue() {}


    HRESULT Startup(
        size_t nWorkerThreadCount,
        HANDLE hNotifyStop,
        DWORD  dwCmdTargetID)
    {
        assert(hNotifyStop);

        m_hNotifyStop = hNotifyStop;
        return m_ThreadPool.StartupSingleWorker(this, nWorkerThreadCount, hNotifyStop, dwCmdTargetID);
    }

    HRESULT WaitForAllExit(DWORD dwMaxWait = INFINITE)
    {
        return m_ThreadPool.WaitForAllExit(dwMaxWait);
    }

    HRESULT AllocChannel(DWORD dwChannelID)
    {
        CObjGuard Guard(m_CmdListLock);

        if (m_ChannelMap.Lookup(dwChannelID))
            return S_FALSE;


        CCallChannel* pChannel = new CCallChannel;
        if (!pChannel)
            return E_OUTOFMEMORY;


        HRESULT hr = pChannel->Initialize(m_hNotifyStop);
        if (FAILED(hr))
        {
            delete pChannel;
            return hr;
        }


        m_ChannelMap.SetAt(dwChannelID, pChannel);
        return S_OK;
    }

    void FreeChannel(DWORD dwChannelID)
    {
        CObjGuard Guard(m_CmdListLock);

        CChannelMap::CPair* pPair = m_ChannelMap.Lookup(dwChannelID);
        if (!pPair)
            return;

        CCallChannel* pChannel = pPair->m_value;
        m_ChannelMap.RemoveAt(pPair);
        assert(pChannel);
        assert(0 == pChannel->m_RspMsgQueue.GetWaitingThreadsCount());

        // remove unprocessed response
        CWinThreadMsg Msg;
        while (pChannel->m_RspMsgQueue.PopMsg(Msg))
        {
            if (Msg.m_MsgType != CWinThreadMsg::em_Normal)
                continue;

            POSITION pos = (POSITION)Msg.m_pvParam;
            assert(pos);
            PopCmdNoReturn(pos);
        }


        delete pChannel;
    }

    // dwChannelID defined by caller
    // it's recommended to pass caller's thread-id as channel-id
    HRESULT AsyncCall(DWORD dwChannelID, IWinModCommand* piCommand, DWORD dwPriority = 0)
    {
        assert(piCommand);
        if (IsNotifiedStop())
            return E_ABORT;

        POSITION pos = PushCmd(dwChannelID, piCommand);
        if (!pos)
            return E_ABORT;

        return m_ThreadPool.PostNormalMsg(pos, dwPriority);
    }

    // just like CWinThreadGroup::PostCommand
    HRESULT AsyncCallNoReturn(IWinModCommand* piCommand, DWORD dwPriority = 0)
    {
        assert(piCommand);
        if (IsNotifiedStop())
            return E_ABORT;

        POSITION pos = PushCmdNoReturn(piCommand);
        if (!pos)
            return E_ABORT;

        return m_ThreadPool.PostNormalMsg(pos, dwPriority);
    }

    DWORD GetAsyncCallCount(DWORD dwChannelID)
    {
        CObjGuard Guard(m_CmdListLock);

        CCallChannel* pChannel = NULL;
        if (!m_ChannelMap.Lookup(dwChannelID, pChannel))
        {   // can not find channel
            assert("unexpected channel id");
            return NULL;
        }

        assert(pChannel);
        return pChannel->m_lAsyncCallCount;
    }

    HRESULT WaitChannelRet(DWORD dwChannelID, IWinModCommand** ppiCommand)
    {
        assert(ppiCommand);
        assert(!*ppiCommand);
        CCallChannel* pChannel = NULL;


        {
            CObjGuard Guard(m_CmdListLock);

            CChannelMap::CPair* pPair = m_ChannelMap.Lookup(dwChannelID);
            if (!pPair)
                return E_ABORT;

            pChannel = pPair->m_value;
            assert(pChannel);
        }


        CWinThreadMsg Msg;
        HRESULT hr = pChannel->m_RspMsgQueue.WaitForMsg(Msg);
        if (FAILED(hr))
            return hr;


        if (Msg.m_MsgType != CWinThreadMsg::em_Normal)
            return E_ABORT;


        InterlockedDecrement(&pChannel->m_lAsyncCallCount);

        CCmdItem Item;
        POSITION pos = (POSITION)Msg.m_pvParam;
        PopCmd((POSITION)Msg.m_pvParam, Item);
        assert(Item.m_spiCmd);
        return Item.m_spiCmd.CopyTo(ppiCommand);
    }

    DWORD GetMaxThreadsCount()
    {
        return m_ThreadPool.GetMaxThreadsCount();
    }

    DWORD GetWaitingThreadsCount()
    {
        return m_ThreadPool.GetWaitingThreadsCount();
    }

    BOOL IsNotifiedStop()
    {
        DWORD dwWaitRet = ::WaitForSingleObject(m_hNotifyStop, 0);
        return dwWaitRet != WAIT_TIMEOUT;
    }

protected:
    // IWinWorker::OnWinWorkerExecute
    HRESULT STDMETHODCALLTYPE OnWinWorkerExecute(HANDLE hNotifyStop, DWORD dwWorkerID, CWinThreadMsg& ThreadMsg)
    {
        if (ThreadMsg.m_MsgType != CWinThreadMsg::em_Normal)
            return S_OK;

        CCmdItem Item;
        PopCmd((POSITION)ThreadMsg.m_pvParam, Item);
        assert(Item.m_spiCmd);
        if (!Item.m_spiCmd)
            return S_OK;

        HRESULT hrCall = InternalExecute(hNotifyStop, Item.m_spiCmd);

        if (Item.m_bWatchRsp)
            PushRsp(Item);

        return hrCall;
    }

protected:
    HRESULT InternalExecute(HANDLE hNotifyStop, IWinModCommand* piCommand)
    {
        assert(piCommand);
        return piCommand->OnWinCmdExecute();
    }

    POSITION PushCmd(DWORD dwChannelID, IWinModCommand* pMsg)
    {
        CObjGuard Guard(m_CmdListLock);

        CCallChannel* pChannel = NULL;
        if (!m_ChannelMap.Lookup(dwChannelID, pChannel))
        {   // can not find channel
            assert("unexpected channel id");
            return NULL;
        }
        
        assert(pChannel);
        InterlockedIncrement(&pChannel->m_lAsyncCallCount);

        CCmdItem Item;
        Item.m_dwChannelID  = dwChannelID;
        Item.m_spiCmd       = pMsg;
        Item.m_bWatchRsp    = TRUE;
        POSITION pos = m_CmdList.AddTail(Item);
        if (!pos)
            return NULL;

        pChannel->PushCmd(pos);
        return pos;
    }

    POSITION PushCmdNoReturn(IWinModCommand* pMsg)
    {
        CObjGuard Guard(m_CmdListLock);

        CCmdItem Item;
        Item.m_dwChannelID  = 0;
        Item.m_spiCmd       = pMsg;
        Item.m_bWatchRsp    = FALSE;
        return m_CmdList.AddTail(Item);
    }

    void PopCmd(POSITION pos, CCmdItem& Item)
    {
        assert(pos);
        CObjGuard Guard(m_CmdListLock);

        Item = m_CmdList.GetAt(pos);
        m_CmdList.RemoveAt(pos);
    }

    void PopCmdNoReturn(POSITION pos)
    {
        assert(pos);
        CObjGuard Guard(m_CmdListLock);

        m_CmdList.RemoveAt(pos);
    }

    void PushRsp(CCmdItem& Item)
    {
        assert(Item.m_spiCmd);
        CObjGuard Guard(m_CmdListLock);  

        CCallChannel* pChannel = NULL;
        if (!m_ChannelMap.Lookup(Item.m_dwChannelID, pChannel))
        {
            // no channel to receive rsp;
            return;
        }

        assert(pChannel);
        POSITION pos = m_CmdList.AddTail(Item);
        if (!pos)
            return;

        pChannel->PostRsp(pos);
    }

public:
    CWinThreadPool  m_ThreadPool;

protected:
    typedef CAtlList<CCmdItem>              CCmdList;
    typedef CRBMap<DWORD, CCallChannel*>    CChannelMap;

    CObjLock        m_CmdListLock;
    CCmdList        m_CmdList;

    HANDLE          m_hNotifyStop;      ///< not own this event

    DWORD           m_dwCmdLimit;
    ATL::CEvent     m_hBelowLimit;

    CChannelMap     m_ChannelMap;       ///< receive complete cmd for each channel
    volatile LONG   m_lLastChannel;
};


class CWinAsyncCall
{
public:
    CWinAsyncCall(): m_dwChannelID(0), m_pAsyncCallQueue(NULL) {}
    ~CWinAsyncCall() {FreeChannel();}

    HRESULT AllocChannel(CWinAsyncCallQueue* pAsyncCallQueue, DWORD dwChannelID)
    {
        FreeChannel();

        if (!pAsyncCallQueue)
            return S_FALSE;

        HRESULT hr = pAsyncCallQueue->AllocChannel(dwChannelID);
        if (FAILED(hr))
            return hr;

        m_dwChannelID       = dwChannelID;
        m_pAsyncCallQueue   = pAsyncCallQueue;
        return S_OK;
    }

    void FreeChannel()
    {
        if (!m_pAsyncCallQueue)
            return;

        m_pAsyncCallQueue->FreeChannel(m_dwChannelID);
        m_dwChannelID       = 0;
        m_pAsyncCallQueue   = NULL;
        return;
    }

    DWORD GetChannelID()
    {
        return m_dwChannelID;
    }

    HRESULT AsyncCall(IWinModCommand* piCommand, DWORD dwPriority = 0)
    {
        assert(m_pAsyncCallQueue);
        assert(piCommand);

        return m_pAsyncCallQueue->AsyncCall(m_dwChannelID, piCommand, dwPriority);
    }

    DWORD GetAsyncCallCount()
    {
        assert(m_pAsyncCallQueue);

        return m_pAsyncCallQueue->GetAsyncCallCount(m_dwChannelID);
    }

    HRESULT WaitChannelRet(IWinModCommand** ppiCommand)
    {
        assert(m_pAsyncCallQueue);
        assert(ppiCommand);
        assert(!*ppiCommand);

        return m_pAsyncCallQueue->WaitChannelRet(m_dwChannelID, ppiCommand);
    }

    template <class T>
    HRESULT WaitChannelRet(T** ppiCommand)
    {
        assert(m_pAsyncCallQueue);
        assert(ppiCommand);
        assert(!*ppiCommand);

        CComPtr<IWinModCommand> spiCmd;
        HRESULT hr = m_pAsyncCallQueue->WaitChannelRet(m_dwChannelID, &spiCmd);
        if (FAILED(hr))
            return hr;

        return spiCmd.QueryInterface(ppiCommand);
    }

    DWORD GetMaxThreadsCount()
    {
        assert(m_pAsyncCallQueue);
        return m_pAsyncCallQueue->GetMaxThreadsCount();
    }

    BOOL NoMoreFreeThread()
    {
        DWORD dwAsyncCallCount  = GetAsyncCallCount();
        DWORD dwMaxThreadsCount = GetMaxThreadsCount();
        return dwAsyncCallCount > dwMaxThreadsCount;
    }

protected:

    DWORD               m_dwChannelID;
    CWinAsyncCallQueue* m_pAsyncCallQueue;
};


NS_WINMOD_END

#endif//WINTHREADGROUP_H