/**
* @file    winhttpmodconnection.cpp
* @brief   ...
* @author  bbcallen
* @date    2010-05-04 15:49
*/

#include "stdafx.h"
#include "winhttpmodconnection.h"

#include "winhttpmod.h"

using namespace WinMod;

HRESULT CWinHttpConnection::HttpRequest(
    /* [in ] */ LPCTSTR         lpObject,
    /* [in ] */ DWORD           dwTimeout,
    /* [in ] */ LPCTSTR         lpszContentType,
    /* [in ] */ const CStringA& strCommand,
    /* [out] */ CStringA*       pstrResponse,
    /* [out] */ DWORD*          pdwStatusCode,
    /* [in ] */ LPCTSTR         lpszSpecHostName)
{
    if (!m_h)
        return E_HANDLE;

    if (!lpObject || !lpszContentType)
        return E_POINTER;

    SetConnectTimeOut(dwTimeout);
    SetSendTimeOut(dwTimeout);
    SetReceiveTimeOut(dwTimeout);


    m_hHttpFile.Close();
    m_hHttpFile.Attach(OpenRequest(L"POST", lpObject));
    if (!m_hHttpFile)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;


    HRESULT hr = E_FAIL;
    if (lpszSpecHostName)
    {
        CString strHost;
        strHost.Format(L"Host: %s", lpszSpecHostName);
        hr = m_hHttpFile.AddRequestHeaders(strHost,  strHost.GetLength(), HTTP_ADDREQ_FLAG_REPLACE);
    }


    CString strContentLength;
    strContentLength.Format(L"Content-Length: %d", strCommand.GetLength());
    CString strContentType = L"Content-Type: ";
    strContentType.Append(lpszContentType);
    CString strAccept = L"Accept: */*";

    hr = m_hHttpFile.AddRequestHeaders(strContentLength,    strContentLength.GetLength());
    hr = m_hHttpFile.AddRequestHeaders(strContentType,      strContentType.GetLength());
    hr = m_hHttpFile.AddRequestHeaders(strAccept,           strAccept.GetLength());



    INTERNET_BUFFERS inetBuf;
    ::ZeroMemory(&inetBuf, sizeof(inetBuf));
    hr = m_hHttpFile.SendRequest(NULL, 0, (LPVOID)(LPCSTR)strCommand, (DWORD)strCommand.GetLength());
    if (FAILED(hr))
        return hr;


    DWORD dwStatusCode = HTTP_STATUS_SERVER_ERROR;
    hr = m_hHttpFile.QueryInfoStatusCode(dwStatusCode);
    if (FAILED(hr))
        return hr;

    
    if (pdwStatusCode)
        *pdwStatusCode = dwStatusCode;


    if (HTTP_STATUS_OK != dwStatusCode)
        return E_FAIL;



    // �������Ҫ����response,��ֱ�ӷ���
    if (!pstrResponse)
        return S_OK;


    DWORD dwContentLength = 400;
    hr = m_hHttpFile.QueryInfoContentLength(dwContentLength);
    if (FAILED(hr))
        return hr;

    // �򵥵ĳ�������
    if (dwContentLength > 0x100000)
        return E_FAIL;


    DWORD dwBytesRead = 0;
    hr = m_hHttpFile.Read(pstrResponse->GetBuffer(dwContentLength), dwContentLength, dwBytesRead);
    pstrResponse->ReleaseBuffer(dwBytesRead);
    if (FAILED(hr))
        return hr;


    return S_OK;
}



HRESULT CWinHttpConnection::HttpDownload(
    /* [in ] */ IWinHttpDownloadFile*       piDownloadFile,
    /* [in ] */ IWinHttpDownloadProgress*   piCallback,
    /* [in ] */ LPCTSTR                     lpObject,
    /* [in ] */ DWORD                       dwTimeout,
    /* [out] */ DWORD*                      pdwStatusCode,
    /* [in ] */ LPCTSTR                     lpszSpecHostName)
{
    if (!m_h)
        return E_HANDLE;

    if (!lpObject || !piDownloadFile)
        return E_POINTER;

    if (pdwStatusCode)
        *pdwStatusCode = 0;


    SetConnectTimeOut(dwTimeout);
    SetSendTimeOut(dwTimeout);
    SetReceiveTimeOut(dwTimeout);


    m_hHttpFile.Close();
    m_hHttpFile.Attach(OpenRequest(L"GET", lpObject));
    if (!m_hHttpFile)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;


    HRESULT hr = E_FAIL;
    if (lpszSpecHostName)
    {
        CString strHost;
        strHost.Format(L"Host: %s", lpszSpecHostName);
        hr = m_hHttpFile.AddRequestHeaders(strHost,  strHost.GetLength(), HTTP_ADDREQ_FLAG_REPLACE);
    }



    INTERNET_BUFFERS inetBuf;
    ::ZeroMemory(&inetBuf, sizeof(inetBuf));
    hr = m_hHttpFile.SendRequest();
    if (FAILED(hr))
        return hr;



    // ��ѯhttp״̬��
    DWORD dwHttpStatusCode = 0;
    hr = m_hHttpFile.QueryInfoStatusCode(dwHttpStatusCode);
    if (FAILED(hr))
        return hr;


    if (pdwStatusCode)
        *pdwStatusCode = dwHttpStatusCode;


    // ���http״̬��
    if (HTTP_STATUS_OK != dwHttpStatusCode)
        return MAKE_WINMOD_HTTP_ERROR(dwHttpStatusCode);


    // ��ѯ���ݳ���
    DWORD dwContentLength = 0;
    hr = m_hHttpFile.QueryInfoContentLength(dwContentLength);
    if (FAILED(hr))
        return hr;




    hr = piDownloadFile->SetSize(dwContentLength);
    if (FAILED(hr))
        return hr;


    // ��ʼ��������
    DWORD dwTotalSize       = dwContentLength;
    DWORD dwTransferedSize  = 0;


    hr = piDownloadFile->Seek(0, FILE_BEGIN);
    if (FAILED(hr))
        return hr;


    while (dwTransferedSize < dwTotalSize)
    {
        BYTE  byBuffer[4096];
        DWORD dwBytesRead = 0;
        DWORD dwBytesLeft = dwTotalSize - dwTransferedSize;
        DWORD dwToRead = min(sizeof(byBuffer), dwBytesLeft); 


        hr = m_hHttpFile.Read(byBuffer, dwToRead, dwBytesRead);
        if (FAILED(hr))
            return hr;


        if (0 == dwBytesRead)
            break;


        // �ص�����
        if (piCallback)
        {
            hr = piCallback->OnReceiveData(
                dwTotalSize,
                dwTotalSize - dwBytesLeft,
                dwBytesRead,
                byBuffer);
            if (FAILED(hr))
                return hr;
        }


        hr = piDownloadFile->Write(byBuffer, dwBytesRead);
        if (FAILED(hr))
            return hr;


        dwTransferedSize += dwBytesRead;
    }


    // �����ļ��Ĵ���
    hr = piDownloadFile->Flush();
    if (FAILED(hr))
        return hr;


    hr = piDownloadFile->SetEndOfFile();
    if (FAILED(hr))
        return hr;


    return S_OK;
}