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

class CWinTrustVerifier
{
public:

    // 验证文件的数字签名
    DWORD   VerifyFile(LPCWSTR pwszFileFullPath);

    DWORD   VerifyFile(LPCWSTR pwszFileFullPath, HANDLE hFile);

    // 优化了流程的验证文件的数字签名(暂时未确认是否准确)
    DWORD   VerifyFileFast(LPCWSTR pwszFileFullPath);

    DWORD   VerifyFileFast(LPCWSTR pwszFileFullPath, HANDLE hFile);

    // 验证wintrust的数字签名
    DWORD   VerifyWinTrustFile(LPCWSTR pwszFileFullPath, HANDLE hFile);

    // 验证WHQL数字签名
    //DWORD   VerifyDriverFile(LPCWSTR pwszFileFullPath, HANDLE hFile);

    // 验证指定的嵌入数字签名
    DWORD   VerifyEmbeddedSignature(LPCWSTR pwszFileFullPath, HANDLE hFile, GUID& policyGUID);

    // 验证包含cat文件的数字签名
    DWORD   VerifyCatalogSignature(LPCWSTR pwszFileFullPath, HANDLE hFile);

    // 判断PE文件中是否包含嵌入的数字签名(暂时未确认是否准确)
    BOOL    HasEmbeddedSignature(HANDLE hFile);

    // 判断是否为PE文件
    static BOOL IsPEFile(LPCWSTR pwszFileFullPath);

    // 判断是否为PE文件
    static BOOL IsPEFile(HANDLE hFile);

    // 判断是否为WinTrust的错误码
    static BOOL IsWinTrustRetCode(DWORD dwRetCode);



    HRESULT TryLoadDll();



    

    CWinModule_wintrust m_modWinTrust;
};

NS_WINMOD_END

#endif//WINTRUSTVERIFIER_H