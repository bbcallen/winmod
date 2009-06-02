/**
* @file    wintrustmod.h
* @brief   ...
* @author  bbcallen
* @date    2009-02-11  11:19
*/

#ifndef WINTRUSTMOD_H
#define WINTRUSTMOD_H

#include <Wintrust.h>
#include <softpub.h>
#include <Mscat.h>
#include "winmod\winmodule.h"

NS_WINMOD_BEGIN

typedef BOOL (WINAPI *PFN_CryptCATAdminAcquireContext)(
    OUT HCATADMIN *phCatAdmin,
    IN const GUID *pgSubsystem,
    IN DWORD dwFlags);

typedef BOOL (WINAPI *PFN_CryptCATAdminReleaseContext)(
    IN HCATADMIN hCatAdmin,
    IN DWORD dwFlags);

typedef BOOL (WINAPI *PFN_CryptCATAdminReleaseCatalogContext)(
    IN HCATADMIN hCatAdmin,
    IN HCATINFO hCatInfo,
    IN DWORD dwFlags);

typedef BOOL (WINAPI *PFN_CryptCATCatalogInfoFromContext)(
    IN HCATINFO hCatInfo,
    IN OUT CATALOG_INFO *psCatInfo,
    IN DWORD dwFlags);

typedef BOOL (WINAPI *PFN_CryptCATAdminCalcHashFromFileHandle)(
    IN HANDLE hFile,
    IN OUT DWORD *pcbHash,
    OUT OPTIONAL BYTE *pbHash,
    IN DWORD dwFlags);

typedef HCATINFO (WINAPI *PFN_CryptCATAdminEnumCatalogFromHash)(
    IN HCATADMIN hCatAdmin,
    IN BYTE *pbHash,
    IN DWORD cbHash,
    IN DWORD dwFlags,
    IN OUT HCATINFO *phPrevCatInfo);

typedef LONG (WINAPI *PFN_WinVerifyTrust)(
    HWND hwnd,
    GUID *pgActionID,
    LPVOID pWVTData);


class CWinModule_wintrust: public CWinModule
{
public:
    CWinModule_wintrust():
        m_pfnCryptCATAdminAcquireContext(NULL),
        m_pfnCryptCATAdminReleaseContext(NULL),
        m_pfnCryptCATAdminReleaseCatalogContext(NULL),
        m_pfnCryptCATCatalogInfoFromContext(NULL),
        m_pfnCryptCATAdminCalcHashFromFileHandle(NULL),
        m_pfnCryptCATAdminEnumCatalogFromHash(NULL),
        m_pfnWinVerifyTrust(NULL)
    {

    }

    virtual void FreeLib()
    {
        m_pfnCryptCATAdminAcquireContext            = NULL;
        m_pfnCryptCATAdminReleaseContext            = NULL;
        m_pfnCryptCATAdminReleaseCatalogContext     = NULL;
        m_pfnCryptCATCatalogInfoFromContext         = NULL;
        m_pfnCryptCATAdminCalcHashFromFileHandle    = NULL;
        m_pfnCryptCATAdminEnumCatalogFromHash       = NULL;
        m_pfnWinVerifyTrust                         = NULL;
        return CWinModule::FreeLib();
    }

    BOOL WINAPI CryptCATAdminAcquireContext(
        OUT HCATADMIN *phCatAdmin,
        IN const GUID *pgSubsystem,
        IN DWORD dwFlags)
    {
        if (NULL == m_pfnCryptCATAdminAcquireContext)
        {
            m_pfnCryptCATAdminAcquireContext =
                (PFN_CryptCATAdminAcquireContext) GetProcAddr("CryptCATAdminAcquireContext");

            if (!m_pfnCryptCATAdminAcquireContext)
                return FALSE;
        }

        return m_pfnCryptCATAdminAcquireContext(
            phCatAdmin,
            pgSubsystem,
            dwFlags);
    }

    BOOL WINAPI CryptCATAdminReleaseContext(
        IN HCATADMIN hCatAdmin,
        IN DWORD dwFlags)
    {
        if (NULL == m_pfnCryptCATAdminReleaseContext)
        {
            m_pfnCryptCATAdminReleaseContext =
                (PFN_CryptCATAdminReleaseContext) GetProcAddr("CryptCATAdminReleaseContext");

            if (!m_pfnCryptCATAdminReleaseContext)
                return FALSE;
        }

        return m_pfnCryptCATAdminReleaseContext(
            hCatAdmin,
            dwFlags);
    }

    BOOL WINAPI CryptCATAdminReleaseCatalogContext(
        IN HCATADMIN hCatAdmin,
        IN HCATINFO hCatInfo,
        IN DWORD dwFlags)
    {
        if (NULL == m_pfnCryptCATAdminReleaseCatalogContext)
        {
            m_pfnCryptCATAdminReleaseCatalogContext =
                (PFN_CryptCATAdminReleaseCatalogContext) GetProcAddr("CryptCATAdminReleaseCatalogContext");

            if (!m_pfnCryptCATAdminReleaseCatalogContext)
                return FALSE;
        }

        return m_pfnCryptCATAdminReleaseCatalogContext(
            hCatAdmin,
            hCatInfo,
            dwFlags);
    }

    BOOL WINAPI CryptCATCatalogInfoFromContext(
        IN HCATINFO hCatInfo,
        IN OUT CATALOG_INFO *psCatInfo,
        IN DWORD dwFlags)
    {
        if (NULL == m_pfnCryptCATCatalogInfoFromContext)
        {
            m_pfnCryptCATCatalogInfoFromContext =
                (PFN_CryptCATCatalogInfoFromContext) GetProcAddr("CryptCATCatalogInfoFromContext");

            if (!m_pfnCryptCATCatalogInfoFromContext)
                return FALSE;
        }

        return m_pfnCryptCATCatalogInfoFromContext(
            hCatInfo,
            psCatInfo,
            dwFlags);
    }

    BOOL WINAPI CryptCATAdminCalcHashFromFileHandle(
        IN HANDLE hFile,
        IN OUT DWORD *pcbHash,
        OUT OPTIONAL BYTE *pbHash,
        IN DWORD dwFlags)
    {
        if (NULL == m_pfnCryptCATAdminCalcHashFromFileHandle)
        {
            m_pfnCryptCATAdminCalcHashFromFileHandle =
                (PFN_CryptCATAdminCalcHashFromFileHandle) GetProcAddr("CryptCATAdminCalcHashFromFileHandle");

            if (!m_pfnCryptCATAdminCalcHashFromFileHandle)
                return FALSE;
        }

        return m_pfnCryptCATAdminCalcHashFromFileHandle(
            hFile,
            pcbHash,
            pbHash,
            dwFlags);
    }

    HCATINFO WINAPI CryptCATAdminEnumCatalogFromHash(
        IN HCATADMIN hCatAdmin,
        IN BYTE *pbHash,
        IN DWORD cbHash,
        IN DWORD dwFlags,
        IN OUT HCATINFO *phPrevCatInfo)
    {
        if (NULL == m_pfnCryptCATAdminEnumCatalogFromHash)
        {
            m_pfnCryptCATAdminEnumCatalogFromHash =
                (PFN_CryptCATAdminEnumCatalogFromHash) GetProcAddr("CryptCATAdminEnumCatalogFromHash");

            if (!m_pfnCryptCATAdminEnumCatalogFromHash)
                return NULL;
        }

        return m_pfnCryptCATAdminEnumCatalogFromHash(
            hCatAdmin,
            pbHash,
            cbHash,
            dwFlags,
            phPrevCatInfo);
    }

    LONG WINAPI WinVerifyTrust(
        HWND hwnd,
        GUID *pgActionID,
        LPVOID pWVTData)
    {
        if (NULL == m_pfnWinVerifyTrust)
        {
            m_pfnWinVerifyTrust =
                (PFN_WinVerifyTrust) GetProcAddr("WinVerifyTrust");

            if (!m_pfnWinVerifyTrust)
                return GetLastError() ? HRESULT_FROM_WIN32(GetLastError()) : E_FAIL;
        }

        return m_pfnWinVerifyTrust(
            hwnd,
            pgActionID,
            pWVTData);
    }


private:

    PFN_CryptCATAdminAcquireContext         m_pfnCryptCATAdminAcquireContext;
    PFN_CryptCATAdminReleaseContext         m_pfnCryptCATAdminReleaseContext;
    PFN_CryptCATAdminReleaseCatalogContext  m_pfnCryptCATAdminReleaseCatalogContext;
    PFN_CryptCATCatalogInfoFromContext      m_pfnCryptCATCatalogInfoFromContext;
    PFN_CryptCATAdminCalcHashFromFileHandle m_pfnCryptCATAdminCalcHashFromFileHandle;
    PFN_CryptCATAdminEnumCatalogFromHash    m_pfnCryptCATAdminEnumCatalogFromHash;
    PFN_WinVerifyTrust                      m_pfnWinVerifyTrust;
};

NS_WINMOD_END

#endif//WINTRUSTMOD_H