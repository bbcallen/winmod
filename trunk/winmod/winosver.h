/**
* @file    winosver.h
* @brief   ...
* @author  bbcallen
* @date    2009-04-09  11:53
*/

#ifndef WINOSVER_H
#define WINOSVER_H

#include "winmod\winmodbase.h"

NS_WINMOD_BEGIN

class CWinOSVer
{
public:

    enum {
        em_OS_MajorVer_Vista    = 6,
        em_OS_MajorVer_Win2k3   = 5,
        em_OS_MajorVer_WinXP    = 5,

        em_OS_MinorVer_Win2k3   = 2,
        em_OS_MinorVer_WinXP    = 1,
    };



    static BOOL IsVista()
    {
        return 0 == CompareMajor(em_OS_MajorVer_Vista);
    }

    static BOOL IsVistaOrLater()
    {
        return 0 <= CompareMajor(em_OS_MajorVer_Vista);
    }

    static BOOL IsWinXPOrLater()
    {
        return 0 <= CompareVersion(em_OS_MajorVer_WinXP, em_OS_MinorVer_WinXP);
    }

    static BOOL IsWin2k3()
    {
        return 0 == CompareVersion(em_OS_MajorVer_Win2k3, em_OS_MinorVer_Win2k3);
    }


    /**
    * @retval   +   当前操作系统大于指定版本
    * @retval   -   当前操作系统小于指定版本
    * @retval   0   当前操作系统等于于指定版本
    */
    static int CompareVersion(DWORD dwMajorVer, DWORD dwMinorVer)
    {
        OSVERSIONINFO osInfo;
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);
        ::GetVersionEx(&osInfo);

        if (osInfo.dwMajorVersion > dwMajorVer)
        {
            return 1;
        }
        else if (osInfo.dwMajorVersion < dwMajorVer)
        {
            return -1;
        }

        return osInfo.dwMinorVersion - dwMinorVer;
    }


    static int CompareMajor(DWORD dwMajorVer)
    {
        OSVERSIONINFO osInfo;
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);
        ::GetVersionEx(&osInfo);

        return osInfo.dwMajorVersion - dwMajorVer;
    }
};

NS_WINMOD_END

#endif//WINOSVER_H