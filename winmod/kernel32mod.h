/**
* @file    kernel32mod.h
* @brief   ...
* @author  bbcallen
* @date    2009-08-04 17:39
*/

#ifndef KERNEL32MOD_H
#define KERNEL32MOD_H

#include <psapi.h>
#include "winmod\winmodule.h"

typedef void (WINAPI *PFN_GetNativeSystemInfo)(__out  LPSYSTEM_INFO lpSystemInfo);
typedef BOOL (WINAPI *PFN_GetProductInfo)(DWORD, DWORD, DWORD, DWORD, PDWORD);

NS_WINMOD_BEGIN

class CWinModule_kernel32: public CWinModule
{
public:
    CWinModule_kernel32():
        m_pfnGetNativeSystemInfo(NULL),
        m_pfnGetProductInfo(NULL)
    {

    }

    virtual void FreeLib()
    {
        m_pfnGetNativeSystemInfo = NULL;
        m_pfnGetProductInfo      = NULL;
        return CWinModule::FreeLib();
    }

    void WINAPI GetNativeSystemInfo(__out  LPSYSTEM_INFO lpSystemInfo)
    {
        if (NULL == m_pfnGetNativeSystemInfo)
        {
            m_pfnGetNativeSystemInfo =
                (PFN_GetNativeSystemInfo) GetProcAddr("GetNativeSystemInfo");

            if (!m_pfnGetNativeSystemInfo)
            {
                ::GetSystemInfo(lpSystemInfo);
                return;
            }
        }

        return m_pfnGetNativeSystemInfo(lpSystemInfo);
    }

    BOOL WINAPI GetProductInfo(
        __in   DWORD dwOSMajorVersion,
        __in   DWORD dwOSMinorVersion,
        __in   DWORD dwSpMajorVersion,
        __in   DWORD dwSpMinorVersion,
        __out  PDWORD pdwReturnedProductType
        )
    {
        if (NULL == m_pfnGetProductInfo)
        {
            m_pfnGetProductInfo = (PFN_GetProductInfo) GetProcAddr("GetProductInfo");

            if (!m_pfnGetProductInfo)
            {
                return FALSE;
            }
        }

        return m_pfnGetProductInfo(
            dwOSMajorVersion,
            dwOSMinorVersion,
            dwSpMajorVersion,
            dwSpMinorVersion,
            pdwReturnedProductType);
    }

private:
    PFN_GetNativeSystemInfo m_pfnGetNativeSystemInfo;
    PFN_GetProductInfo      m_pfnGetProductInfo;
};

NS_WINMOD_END

#endif//KERNEL32MOD_H