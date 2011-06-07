/**
* @file    winmodperf.h
* @brief   ...
* @author  bbcallen
* @date    2011-04-02 11:46
*/

#ifndef WINMODPERF_H
#define WINMODPERF_H

#include <utility>
#include <atltime.h>
#include "winmodbase.h"

NS_WINMOD_BEGIN


/*

Example:

CWinModPerfManager g_PerfMgr;

void DoSomething();

void Foo()
{
    {
        CWinModPerfGuard PerfGuard(g_PerfMgr)

        DoSomething();
    }

    ...
    
    WINMOD_PERF_DATA PerfData;
    g_PerfMgr.GetPerfData(PerfData);

    return 0;
}

*/


struct WINMOD_PERF_DATA
{
    UINT32  m_uPerfCount;       ///< 测试次数
    DOUBLE  m_dPerfTotal;       ///< 测试总时间
    UINT64  m_uTopPerf[10];     ///< 耗时最高的10次测试的时间
};

class CWinModPerf: public WINMOD_PERF_DATA
{
public:
    CWinModPerf()
    {
        ZeroMemory((WINMOD_PERF_DATA*)this, sizeof(WINMOD_PERF_DATA));
    }

    void AddPerf(UINT64 uTickDiff)
    {
        m_uPerfCount++;
        m_dPerfTotal += uTickDiff;

        // 简单插入排序
        size_t i = 0;
        for (NULL; i < _countof(m_uTopPerf); ++i)
        {
            if (uTickDiff > m_uTopPerf[i])
                break;
        }

        for (NULL; i < _countof(m_uTopPerf); ++i)
        {
            std::swap(uTickDiff, m_uTopPerf[i]);
        }
    }

};










class CWinModPerfCounter: public CWinModSyncTraits
{
public:
    typedef CAtlMap<UINT32, CWinModPerf> CPerfMap;

    void ResetPerf()
    {
        CObjGuard Guard(m_ObjLock);

        m_PerfMap.RemoveAll();
    }

    void AddPerf(UINT32 dwPerfID, UINT64 uTickDiff)
    {
        CObjGuard Guard(m_ObjLock);

        m_PerfMap[dwPerfID].AddPerf(uTickDiff);
    }

    void GetPerfData(UINT32 dwPerfID, WINMOD_PERF_DATA& PerfData)
    {
        CObjGuard Guard(m_ObjLock);

        PerfData = m_PerfMap[dwPerfID];
    }

protected:
    CObjLock    m_ObjLock;
    CPerfMap    m_PerfMap;
};









template <class T_TIMER>
class CWinModPerfGuardT
{
public:
    CWinModPerfGuardT(DWORD dwPerfID, CWinModPerfCounter* pPerfCounter)
        : m_uPerfTickBegin(0)
        , m_dwPerfID(dwPerfID)
        , m_pPerfCounter(pPerfCounter)
    {
        if (!m_pPerfCounter)
            return;

        m_uPerfTickBegin = T_TIMER::GetTime();
    }

    CWinModPerfGuardT(DWORD dwPerfID, CWinModPerfCounter& pPerfCounter)
        : m_uPerfTickBegin(0)
        , m_dwPerfID(dwPerfID)
        , m_pPerfCounter(&pPerfCounter)
    {
        m_uPerfTickBegin = T_TIMER::GetTime();
    }

    ~CWinModPerfGuardT()
    {
        CountAndRelease();
    }

    void CountAndRelease()
    {
        if (m_pPerfCounter)
        {
            ULONGLONG uTickEnd  = T_TIMER::GetTime();
            ULONGLONG uTickDiff = uTickEnd - m_uPerfTickBegin;
            m_pPerfCounter->AddPerf(m_dwPerfID, uTickDiff);
        }
    }

    ULONGLONG           m_uPerfTickBegin;
    DWORD               m_dwPerfID;
    CWinModPerfCounter* m_pPerfCounter;
};






class CWinModPerfTimer_Performance
{
public:
    static UINT64 GetTime()
    {
        LARGE_INTEGER liPerformceCount;
        ::QueryPerformanceCounter(&liPerformceCount);
        return PerformceCountToSystemTime(liPerformceCount.QuadPart);
    }

    static UINT64 PerformceCountToSystemTime(UINT64 uTick)
    {
        LARGE_INTEGER liFrequency;
        if (!::QueryPerformanceFrequency(&liFrequency))
            return 0;

        if (!liFrequency.QuadPart)
            return 0;

        UINT64 uTime = UINT64(DOUBLE(uTick) * CFileTime::Second / liFrequency.QuadPart);
        return uTime;
    }
};
typedef CWinModPerfGuardT<CWinModPerfTimer_Performance> CWinModPerfGuard_Performance;
typedef CWinModPerfGuardT<CWinModPerfTimer_Performance> CWinModPerfGuard;










class CWinModPerfTimer_ThreadCpu
{
public:
    static UINT64 GetTime()
    {
        CFileTime ftCreateTime;
        CFileTime ftExitTime;
        CFileTime ftKernelTime;
        CFileTime ftUserTime;

        ::GetThreadTimes(
            ::GetCurrentThread(),
            &ftCreateTime,
            &ftExitTime,
            &ftKernelTime,
            &ftUserTime);

        ULONGLONG uTime = ftKernelTime.GetTime() + ftUserTime.GetTime();
        return uTime;
    }
};
typedef CWinModPerfGuardT<CWinModPerfTimer_ThreadCpu> CWinModPerfGuard_ThreadCpu;



NS_WINMOD_END

#endif//WINMODPERF_H