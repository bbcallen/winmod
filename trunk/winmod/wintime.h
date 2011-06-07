/**
* @file    wintime.h
* @brief   ...
* @author  bbcallen
* @date    2011-03-16 15:28
*/

#ifndef WINTIME_H
#define WINTIME_H

#include <time.h>
#include <atltime.h>
#include "winmodbase.h"

NS_WINMOD_BEGIN

#define WINMOD_EPOCH_BIAS 116444736000000000i64

class CWinFileTime: public ATL::CFileTime
{
public:
    CWinFileTime() throw();
    CWinFileTime(const FILETIME& ft) throw();
    CWinFileTime(ULONGLONG nTime) throw();

    __time64_t GetCRTTime() const throw();
    void SetCRTTime(__time64_t nCRTTime) throw();

    SYSTEMTIME GetLocalSystemTime() const throw();
    SYSTEMTIME GetUTCSystemTime() const throw();

    SYSTEMTIME ToSystemTime() const throw();

    SYSTEMTIME UTCToLocalSystem() const throw();
    SYSTEMTIME LocalToUTCSystem() const throw();
};



inline CWinFileTime::CWinFileTime() throw()
{
}

inline CWinFileTime::CWinFileTime(const FILETIME& ft) throw(): ATL::CFileTime(ft)
{
}

inline CWinFileTime::CWinFileTime(ULONGLONG nTime) throw(): ATL::CFileTime(nTime)
{
}



inline __time64_t CWinFileTime::GetCRTTime() const throw()
{
    ULONGLONG nWinTime = ATL::CFileTime::GetTime();
    __time64_t nCRTTime = ((nWinTime - WINMOD_EPOCH_BIAS) / 10000000i64);
    return nCRTTime;
}

inline void CWinFileTime::SetCRTTime(__time64_t nCRTTime) throw()
{
    ULONGLONG nWinTime = nCRTTime * 10000000i64 + WINMOD_EPOCH_BIAS;
    return SetTime(nWinTime);
}



inline SYSTEMTIME CWinFileTime::GetLocalSystemTime() const throw()
{
    CWinFileTime wftUTC = ATL::CFileTime::GetCurrentTime();
    return wftUTC.UTCToLocalSystem();
}

inline SYSTEMTIME CWinFileTime::GetUTCSystemTime() const throw()
{
    CWinFileTime wftUTC = ATL::CFileTime::GetCurrentTime();
    return wftUTC.ToSystemTime();
}

inline SYSTEMTIME CWinFileTime::ToSystemTime() const throw()
{
    SYSTEMTIME st;
    ::FileTimeToSystemTime(this, &st);
    return st;
}

inline SYSTEMTIME CWinFileTime::UTCToLocalSystem() const throw()
{
    CWinFileTime wftLocal = ATL::CFileTime::UTCToLocal();
    return wftLocal.ToSystemTime();
}

inline SYSTEMTIME CWinFileTime::LocalToUTCSystem() const throw()
{
    CWinFileTime wftUTC = ATL::CFileTime::LocalToUTC();
    return wftUTC.ToSystemTime();
}

NS_WINMOD_END

#endif//WINTIME_H