/**
* @file    winthreadpooldef.h
* @brief   ...
* @author  bbcallen
* @date    2011-03-13 16:48
*/

#ifndef WINTHREADPOOLDEF_H
#define WINTHREADPOOLDEF_H

#include "winrunnable.h"

NS_WINMOD_BEGIN

//////////////////////////////////////////////////////////////////////////
class CWinThreadMsg
{
public:
    enum MSG_TYPE
    {
        em_Normal   = 0,
        em_Commit   = 1,  // block following msg, deactive all threads
        em_Exit     = 2,
    };

    CWinThreadMsg(): m_MsgType(em_Normal), m_pvParam(NULL), m_dwPriority(0) {}
    CWinThreadMsg(MSG_TYPE MsgType, void* pvParam, DWORD dwPriority = 0)
        : m_MsgType(MsgType)
        , m_pvParam(pvParam)
        , m_dwPriority(dwPriority)
    {}

    MSG_TYPE    m_MsgType;
    void*       m_pvParam;
    DWORD       m_dwPriority;   // 0 is lowest
};



//////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE IWinWorker
{
public:
    virtual HRESULT STDMETHODCALLTYPE OnWinWorkerExecute(HANDLE hNotifyStop, DWORD dwWorkerID, CWinThreadMsg& ThreadMsg) = 0;
};


//////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE IWinMessageQueue
{
public:
    virtual HRESULT STDMETHODCALLTYPE PostMsg(CWinThreadMsg Msg) = 0;
    virtual HRESULT STDMETHODCALLTYPE ResetMsgQueue() = 0;

    virtual BOOL    STDMETHODCALLTYPE IsEmpty() = 0;
    virtual HRESULT STDMETHODCALLTYPE WaitForMsg(CWinThreadMsg& rMsg, DWORD dwWorkerID = ULONG_MAX) = 0;
    virtual DWORD   STDMETHODCALLTYPE GetWaitingThreadsCount() = 0;
};


//////////////////////////////////////////////////////////////////////////
class __declspec(uuid("E7F5995F-5540-489a-992B-48006E2B5161"))
IWinModCommand: public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE OnWinCmdExecute() = 0;
};


//////////////////////////////////////////////////////////////////////////
class __declspec(uuid("5C39BFBB-86AF-4fe2-B3EA-959943056D60"))
IWinModTimerCommand: public IWinModCommand
{
public:
    // non-repeat timer would not be called
    virtual BOOL STDMETHODCALLTYPE TimerNeedContinue() = 0;
};

NS_WINMOD_END

#endif//WINTHREADPOOLDEF_H