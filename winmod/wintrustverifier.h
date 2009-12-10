/**
* @file    wintrustverifier.h
* @brief   ...
* @author  bbcallen
* @date    2009-02-11  17:22
*/

#ifndef WINTRUSTVERIFIER_H
#define WINTRUSTVERIFIER_H

#include "winmod\winmodbase.h"
#include "winmod\wintrustmod.h"

NS_WINMOD_BEGIN

/// MSDN says WinVerifyTrust may be altered or unavailable 
/// in subsequent versions, so we call LoadLibaray to obtain WinVerifyTrust
class CWinTrustVerifier
{
public:

    DWORD   VerifyFile(LPCWSTR pwszFileFullPath);

    DWORD   VerifyFile(LPCWSTR pwszFileFullPath, HANDLE hFile);

    DWORD   VerifyEmbeddedWinTrustFile(LPCWSTR pwszFileFullPath, HANDLE hFile);

    DWORD   VerifyEmbeddedSignature(LPCWSTR pwszFileFullPath, HANDLE hFile, GUID& policyGUID);

    DWORD   VerifyCatalogSignature(LPCWSTR pwszFileFullPath, HANDLE hFile);

    BOOL    HasEmbeddedSignature(HANDLE hFile);

    static BOOL IsPEFile(LPCWSTR pwszFileFullPath);

    static BOOL IsPEFile(HANDLE hFile);

    static BOOL IsWinTrustRetCode(DWORD dwRetCode);



    HRESULT TryLoadDll();



    

    CWinModule_wintrust m_modWinTrust;
};

NS_WINMOD_END

#endif//WINTRUSTVERIFIER_H