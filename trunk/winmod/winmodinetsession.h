/**
* @file    winmodinetsession.h
* @brief   ...
* @author  bbcallen
* @date    2009-08-07 15:13
*/

#ifndef WINMODINETSESSION_H
#define WINMODINETSESSION_H

#include <atlstr.h>
#include "winmod\winmodinethandle.h"
#include "winmod\winmodinethttpconnection.h"

NS_WINMOD_BEGIN

class CInetSession: public CInetHandle
{
public:
    CInetSession();

    virtual ~CInetSession();

    HRESULT Open(
        LPCTSTR lpszAgent       = NULL,
        DWORD   dwAccessType    = INTERNET_OPEN_TYPE_PRECONFIG, 
        LPCTSTR lpszProxy       = NULL, 
        LPCTSTR lpszProxyBypass = NULL, 
        DWORD   dwFlags         = 0);

    // 确保以线程安全的方式重新打开wininet会话
    HRESULT Reopen(
        LPCTSTR lpszAgent       = NULL,
        DWORD   dwAccessType    = INTERNET_OPEN_TYPE_PRECONFIG, 
        LPCTSTR lpszProxy       = NULL, 
        LPCTSTR lpszProxyBypass = NULL, 
        DWORD   dwFlags         = 0);

    HINTERNET HttpConnect(
        LPCTSTR         lpszServerName,
        INTERNET_PORT   nServerPort     = INTERNET_DEFAULT_HTTP_PORT,
        LPCTSTR         lpszUsername    = NULL,
        LPCTSTR         lpszPassword    = NULL,
        DWORD           dwFlags         = 0);

    HRESULT HttpRequest(
        /* [in ] */ LPCTSTR         lpszServerName,
        /* [in ] */ INTERNET_PORT   nServerPort,
        /* [in ] */ LPCTSTR         lpObject,
        /* [in ] */ DWORD           dwTimeout,
        /* [in ] */ LPCTSTR         lpszContentType,
        /* [in ] */ const CStringA& strCommand,
        /* [out] */ CStringA*       pstrResponse        = NULL,
        /* [out] */ DWORD*          pdwStatusCode       = NULL,
        /* [in ] */ LPCTSTR         lpszSpecHostName    = NULL);

    HRESULT HttpDownload(
        /* [in ] */ IInetHttpDownloadFile*      piDownloadFile,
        /* [in ] */ IInetHttpDownloadProgress*  piCallback,
        /* [in ] */ LPCTSTR                     lpszServerName,
        /* [in ] */ INTERNET_PORT               nServerPort,
        /* [in ] */ LPCTSTR                     lpObject,
        /* [in ] */ DWORD                       dwTimeout,
        /* [out] */ DWORD*                      pdwStatusCode       = NULL,
        /* [in ] */ LPCTSTR                     lpszSpecHostName    = NULL);



    HRESULT SetConnectTimeOut(DWORD  dwMilliSeconds);
    HRESULT GetConnectTimeOut(DWORD& dwMilliSeconds);


public:
    typedef CComAutoCriticalSection                     CObjLock;
    typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;

    CObjLock m_objLock;
};



inline CInetSession::CInetSession()
{
}

inline CInetSession::~CInetSession()
{
}

inline HRESULT CInetSession::Open(
    LPCTSTR lpszAgent,
    DWORD   dwAccessType, 
    LPCTSTR lpszProxy, 
    LPCTSTR lpszProxyBypass, 
    DWORD   dwFlags)
{
    Close();
    HINTERNET hSession = ::InternetOpen(lpszAgent, dwAccessType, lpszProxy, lpszProxyBypass, dwFlags);
    if (!hSession)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    Attach(hSession);
    return S_OK;
}

inline HRESULT CInetSession::Reopen(
    LPCTSTR lpszAgent,
    DWORD   dwAccessType, 
    LPCTSTR lpszProxy, 
    LPCTSTR lpszProxyBypass, 
    DWORD   dwFlags)
{
    CObjGuard objGuard(m_objLock);

    HINTERNET hNewSession = ::InternetOpen(lpszAgent, dwAccessType, lpszProxy, lpszProxyBypass, dwFlags);
    if (!hNewSession)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    HINTERNET hOldSession = m_h;
    m_h = hNewSession;

    if (hOldSession)
        ::InternetCloseHandle(hOldSession);

    return S_OK;
}

inline HINTERNET CInetSession::HttpConnect(
    LPCTSTR         lpszServerName,
    INTERNET_PORT   nServerPort,
    LPCTSTR         lpszUsername,
    LPCTSTR         lpszPassword,
    DWORD           dwFlags)
{
    assert(m_h);
    HINTERNET hConn = ::InternetConnect(
        m_h,
        lpszServerName,
        nServerPort,
        lpszUsername,
        lpszPassword,
        INTERNET_SERVICE_HTTP,
        dwFlags,
        0);
    if (!hConn)
        return NULL;

    return hConn;
}

inline HRESULT CInetSession::HttpRequest(
    /* [in ] */ LPCTSTR         lpszServerName,
    /* [in ] */ INTERNET_PORT   nServerPort,
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

    if (!lpszServerName || !lpObject)
        return E_POINTER;

    CInetHttpConnection hConn;
    hConn.Attach(HttpConnect(lpszServerName, nServerPort));
    if (!hConn)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;


    return hConn.HttpRequest(
        lpObject,
        dwTimeout,
        lpszContentType,
        strCommand,
        pstrResponse,
        pdwStatusCode,
        lpszSpecHostName);
}

inline HRESULT CInetSession::HttpDownload(
    /* [in ] */ IInetHttpDownloadFile*      piDownloadFile,
    /* [in ] */ IInetHttpDownloadProgress*  piCallback,
    /* [in ] */ LPCTSTR                     lpszServerName,
    /* [in ] */ INTERNET_PORT               nServerPort,
    /* [in ] */ LPCTSTR                     lpObject,
    /* [in ] */ DWORD                       dwTimeout,
    /* [out] */ DWORD*                      pdwStatusCode,
    /* [in ] */ LPCTSTR                     lpszSpecHostName)
{
    if (!m_h)
        return E_HANDLE;

    if (!lpszServerName || !lpObject)
        return E_POINTER;

    CInetHttpConnection hConn;
    hConn.Attach(HttpConnect(lpszServerName, nServerPort));
    if (!hConn)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;


    return hConn.HttpDownload(
        piDownloadFile,
        piCallback,
        lpObject,
        dwTimeout,
        pdwStatusCode,
        lpszSpecHostName);  
}

inline HRESULT CInetSession::SetConnectTimeOut(DWORD  dwMilliSeconds)
{
    return DoSetOptionDWORD(INTERNET_OPTION_CONNECT_TIMEOUT, dwMilliSeconds);
}

inline HRESULT CInetSession::GetConnectTimeOut(DWORD& dwMilliSeconds)
{
    return DoGetOptionDWORD(INTERNET_OPTION_CONNECT_TIMEOUT, dwMilliSeconds);
}

NS_WINMOD_END

#endif//WINMODINETSESSION_H