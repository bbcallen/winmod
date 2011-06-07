/**
* @file    winmodinethttpconnection.h
* @brief   ...
* @author  bbcallen
* @date    2009-08-07 15:13
*/

#ifndef WINMODINETHTTPCONNECTION_H
#define WINMODINETHTTPCONNECTION_H

#include <atlstr.h>
#include "winmodinetconnection.h"
#include "winmodinethttpfile.h"

NS_WINMOD_BEGIN

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

class CInetSession;
class CInetHttpConnection: public CInetConnection
{
public:
    CInetHttpConnection();
    virtual ~CInetHttpConnection();
	
    virtual void Close() {Interrupt(); CInetConnection::Close();}

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
        DWORD       dwFlags             = INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_NO_CACHE_WRITE,
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

    void    Interrupt();

private:
    // denied
    CInetHttpConnection(CInetHttpConnection& h);
    explicit CInetHttpConnection(HANDLE h);
    CInetHttpConnection& operator=(CInetHttpConnection& h);

private:
    // used by HttpRequest and HttpDownload
    CInetHttpFile m_hHttpFile;
};




inline CInetHttpConnection::CInetHttpConnection()
{
}

inline CInetHttpConnection::~CInetHttpConnection()
{
}


inline void CInetHttpConnection::Interrupt()
{
    m_hHttpFile.Close();
}


NS_WINMOD_END

#endif//WINMODINETHTTPCONNECTION_H