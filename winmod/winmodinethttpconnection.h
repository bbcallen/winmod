/**
* @file    winmodinethttpconnection.h
* @brief   ...
* @author  bbcallen
* @date    2009-08-07 15:13
*/

#ifndef WINMODINETHTTPCONNECTION_H
#define WINMODINETHTTPCONNECTION_H

#include <atlstr.h>
#include "winmod\winmodinetconnection.h"
#include "winmod\winmodinethttpfile.h"

NS_WINMOD_BEGIN

#define WINMOD_HTTP__APPLICATION__X_WWW_FORM_URLENCODED     L"application/x-www-form-urlencoded"
#define WINMOD_HTTP__APPLICATION__X_OCTET_STREAM            L"application/octet-stream"


// 下载回调接口
class IInetHttpDownloadFile
{
public:

    enum {
        UNKNOWN_SIZE = 0, 
    };

    virtual HRESULT STDMETHODCALLTYPE SetSize(DWORD dwSize = UNKNOWN_SIZE) = 0;
    virtual HRESULT STDMETHODCALLTYPE Close() = 0;
    virtual HRESULT STDMETHODCALLTYPE Seek(LONG lOffset, int nOrigin) = 0;
    virtual HRESULT STDMETHODCALLTYPE Write(LPCVOID pvData, DWORD dwSize) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetEndOfFile() = 0;
    virtual HRESULT STDMETHODCALLTYPE Flush() = 0;
};

class IInetHttpDownloadProgress
{
public:
    virtual HRESULT STDMETHODCALLTYPE OnReceiveData(
        DWORD   dwTotalSize,
        DWORD   dwCurrentOffset,
        DWORD   dwCurrentSize,
        LPCVOID pvData) = 0;
};


class CInetHttpConnection: public CInetConnection
{
public:
    CInetHttpConnection();

    CInetHttpConnection(CInetHttpConnection& h);

    explicit CInetHttpConnection(HANDLE h);

    virtual ~CInetHttpConnection();

    CInetHttpConnection& operator=(CInetHttpConnection& h);

    // pstrVerb could be one of:
    // "POST"
    // "GET"
    // "HEAD"
    // "PUT"
    // "LINK"
    // "DELETE"
    // "UNLINK"
    HINTERNET OpenRequest(
        LPCTSTR     pstrVerb,
        LPCTSTR     pstrObjectName,
        LPCTSTR     pstrVersion         = NULL,
        LPCTSTR     pstrReferer         = NULL,
        LPCTSTR*    ppstrAcceptTypes    = NULL,
        DWORD       dwFlags             = INTERNET_FLAG_EXISTING_CONNECT,
        DWORD_PTR   dwContext           = 1);

    // lpszContentType could be one of:
    // WINMOD_HTTP_CONTENT_TYPE__APPLICATION__X_WWW_FORM_URLENCODED
    // WINMDD_HTTP_CONTENT_TYPE__APPLICATION__X_OCTET_STREAM
    // ...
    HRESULT HttpRequest(
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
        /* [in ] */ LPCTSTR                     lpObject,
        /* [in ] */ DWORD                       dwTimeout,
        /* [out] */ DWORD*                      pdwStatusCode       = NULL,
        /* [in ] */ LPCTSTR                     lpszSpecHostName    = NULL);
};




inline CInetHttpConnection::CInetHttpConnection():
    CInetConnection()
{
}

inline CInetHttpConnection::CInetHttpConnection(CInetHttpConnection& h):
    CInetConnection(h)
{
}

inline CInetHttpConnection::CInetHttpConnection(HANDLE h):
    CInetConnection(h)
{
}

inline CInetHttpConnection::~CInetHttpConnection()
{
}

inline CInetHttpConnection& CInetHttpConnection::operator=(CInetHttpConnection& h)
{
    *(CInetConnection*)this = (CInetConnection&)h;
}



inline HINTERNET CInetHttpConnection::OpenRequest(
    LPCTSTR     pstrVerb,
    LPCTSTR     pstrObjectName,
    LPCTSTR     pstrVersion,
    LPCTSTR     pstrReferer,
    LPCTSTR*    ppstrAcceptTypes,
    DWORD       dwFlags,
    DWORD_PTR   dwContext)
{
    assert(m_h);
    HINTERNET hFile = ::HttpOpenRequest(
        m_h,
        pstrVerb,
        pstrObjectName,
        pstrVersion,
        pstrReferer,
        ppstrAcceptTypes,
        dwFlags,
        dwContext);
    if (!hFile)
        return NULL;

    return hFile;
}

NS_WINMOD_END

#endif//WINMODINETHTTPCONNECTION_H