/**
* @file    winmodsync.h
* @brief   ...
* @author  bbcallen
* @date    2011-05-10 15:42
*/

#ifndef WINMODSYNC_H
#define WINMODSYNC_H

#include <winbase.h>
#include "winmodbase.h"

NS_WINMOD_BEGIN

class CWinModSyncTraits
{
public:
    typedef CComAutoCriticalSection                     CObjLock;
    typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;
};

class CWinModSyncFakeTraits
{
public:
    typedef CComFakeCriticalSection                     CObjLock;
    typedef CComCritSecLock<CComFakeCriticalSection>    CObjGuard;
};





class CWinCriticalSectionWithTID
{
public:
    CWinCriticalSectionWithTID() throw()
    {
        m_LastOwnThreadId = 0;
        memset(&m_sec, 0, sizeof(CRITICAL_SECTION));
    }
    ~CWinCriticalSectionWithTID()
    {
    }
    HRESULT Lock() throw()
    {
        EnterCriticalSection(&m_sec);
        m_LastOwnThreadId = ::GetCurrentThreadId();
        return S_OK;
    }
    HRESULT Unlock() throw()
    {
        LeaveCriticalSection(&m_sec);
        if (0 == m_sec.RecursionCount)
            m_LastOwnThreadId = 0;
        return S_OK;
    }
    HRESULT Init() throw()
    {
        HRESULT hRes = E_FAIL;
        __try
        {
            InitializeCriticalSection(&m_sec);
            hRes = S_OK;
        }
        // structured exception may be raised in low memory situations
        __except(STATUS_NO_MEMORY == GetExceptionCode())
        {			
            hRes = E_OUTOFMEMORY;		
        }
        return hRes;
    }

    HRESULT Term() throw()
    {
        DeleteCriticalSection(&m_sec);
        return S_OK;
    }

    BOOL OwnByCurrentThread()
    {
        if (NULL == m_sec.OwningThread)
            return FALSE;

        // we don't really need the thread-id if it's not current thread
        // so we don't need to worry about the race-condition
        DWORD dwCurrentID = ::GetCurrentThreadId();
        DWORD dwLockID    = m_LastOwnThreadId;
        return dwCurrentID == dwLockID;
    }

    CRITICAL_SECTION    m_sec;
    DWORD               m_LastOwnThreadId;
};

class CWinAutoCriticalSectionWithTID : public CWinCriticalSectionWithTID
{
public:
    CWinAutoCriticalSectionWithTID()
    {
        HRESULT hr = CWinCriticalSectionWithTID::Init();
        if (FAILED(hr))
            AtlThrow(hr);
    }
    ~CWinAutoCriticalSectionWithTID() throw()
    {
        CWinCriticalSectionWithTID::Term();
    }
private :
    HRESULT Init(); // Not implemented. CWinAutoCriticalSectionWithTID::Init should never be called
    HRESULT Term(); // Not implemented. CWinAutoCriticalSectionWithTID::Term should never be called
};



class CWinModSyncTraitsWithTID
{
public:
    typedef CWinAutoCriticalSectionWithTID              CObjLock;
    typedef CComCritSecLock<CWinCriticalSectionWithTID> CObjGuard;
};

class CWinModSyncTraitsDbg
{
public:
#ifdef _DEBUG
    typedef CWinAutoCriticalSectionWithTID              CObjLock;
    typedef CComCritSecLock<CWinCriticalSectionWithTID> CObjGuard;
#else
    typedef CComAutoCriticalSection                     CObjLock;
    typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;
#endif
};



#ifdef _DEBUG
#define WINMOD_ASSERT_OWN_CS(cs__)      (assert(cs__.OwnByCurrentThread()))
#define WINMOD_ASSERT_NOT_OWN_CS(cs__)  (assert(!cs__.OwnByCurrentThread()))
#else
#define WINMOD_ASSERT_OWN_CS(cs__)
#define WINMOD_ASSERT_NOT_OWN_CS(cs__)
#endif


NS_WINMOD_END

#endif//WINMODSYNC_H