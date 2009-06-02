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

    // ��֤�ļ�������ǩ��
    DWORD   VerifyFile(LPCWSTR pwszFileFullPath);

    DWORD   VerifyFile(LPCWSTR pwszFileFullPath, HANDLE hFile);

    // �Ż������̵���֤�ļ�������ǩ��(��ʱδȷ���Ƿ�׼ȷ)
    DWORD   VerifyFileFast(LPCWSTR pwszFileFullPath);

    DWORD   VerifyFileFast(LPCWSTR pwszFileFullPath, HANDLE hFile);

    // ��֤wintrust������ǩ��
    DWORD   VerifyWinTrustFile(LPCWSTR pwszFileFullPath, HANDLE hFile);

    // ��֤WHQL����ǩ��
    //DWORD   VerifyDriverFile(LPCWSTR pwszFileFullPath, HANDLE hFile);

    // ��ָ֤����Ƕ������ǩ��
    DWORD   VerifyEmbeddedSignature(LPCWSTR pwszFileFullPath, HANDLE hFile, GUID& policyGUID);

    // ��֤����cat�ļ�������ǩ��
    DWORD   VerifyCatalogSignature(LPCWSTR pwszFileFullPath, HANDLE hFile);

    // �ж�PE�ļ����Ƿ����Ƕ�������ǩ��(��ʱδȷ���Ƿ�׼ȷ)
    BOOL    HasEmbeddedSignature(HANDLE hFile);

    // �ж��Ƿ�ΪPE�ļ�
    static BOOL IsPEFile(LPCWSTR pwszFileFullPath);

    // �ж��Ƿ�ΪPE�ļ�
    static BOOL IsPEFile(HANDLE hFile);

    // �ж��Ƿ�ΪWinTrust�Ĵ�����
    static BOOL IsWinTrustRetCode(DWORD dwRetCode);



    HRESULT TryLoadDll();



    

    CWinModule_wintrust m_modWinTrust;
};

NS_WINMOD_END

#endif//WINTRUSTVERIFIER_H