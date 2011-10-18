/**
* @file    winhttpmodfile.h
* @brief   ...
* @author  bbcallen
* @date    2010-05-04 16:44
*/

#ifndef WINHTTPMODFILE_H
#define WINHTTPMODFILE_H

#include "winhttpmodhandle.h"

NS_WINMOD_BEGIN

class CWinHttpFile: public CWinHttpHandle
{
public:
    CWinHttpFile();
    virtual ~CWinHttpFile();

    HRESULT Read(LPVOID pBuffer, DWORD nBufSize, DWORD& nBytesRead);

    HRESULT Write(LPCVOID pBuffer, DWORD nBufSize, DWORD& nBytesWritten);

    //HRESULT Seek(LONGLONG nOffset, DWORD dwFrom = FILE_CURRENT);

    HRESULT AddRequestHeaders(
        LPCWSTR pstrHeaders,
        DWORD   dwHeadersLen    = -1,
        DWORD   dwFlags         = WINHTTP_ADDREQ_FLAG_ADD);

    HRESULT SendRequestEx(
        LPCTSTR pstrHeaders     = NULL,
        DWORD   dwHeadersLen    = -1,
        LPVOID  lpOptional      = NULL,
        DWORD   dwOptionalLen   = 0,
        DWORD   dwTotalLength   = 0,
        DWORD_PTR dwContext     = NULL);

    HRESULT SendRequestEx(DWORD dwContentLength);


    HRESULT ReceiveResponse();
    HRESULT EndRequest() {return ReceiveResponse();}


    HRESULT QueryInfoStatusCode(DWORD& dwStatusCode) const;
    HRESULT QueryInfoContentLength(DWORD& dwContentLength) const;
    HRESULT QueryInfo_AsString(DWORD dwInfoLevel, CString& strValue);
    HRESULT QueryInfo_AsString(LPCWSTR lpszCustomHeader, CString& strValue);

private:
    // denied
    CWinHttpFile(CWinHttpFile& h);
    CWinHttpFile(HINTERNET h);
    CWinHttpFile& operator=(CWinHttpFile& h);
};




inline CWinHttpFile::CWinHttpFile()
{
}

inline CWinHttpFile::~CWinHttpFile()
{
}



inline HRESULT CWinHttpFile::Read(LPVOID pBuffer, DWORD nBufSize, DWORD& nBytesRead)
{
    assert(m_h);

    nBytesRead = 0;
    while (nBytesRead < nBufSize)
    {
        //DWORD dwDataAvailable = 0;
        //if (!::WinHttpQueryDataAvailable(m_h, &dwDataAvailable))
        //    dwDataAvailable = 0;


        DWORD dwLeft = nBufSize - nBytesRead;
        DWORD dwToRead = dwLeft;
        //DWORD dwToRead = min(dwLeft, dwDataAvailable);


        DWORD dwRead = 0;
        if (!::WinHttpReadData(m_h, pBuffer, dwToRead, &dwRead))
            return GetLastError() ? AtlHresultFromLastError() : E_FAIL;


        nBytesRead += dwRead;
        if (0 == dwRead)
            break;
    }

    return S_OK;
}

inline HRESULT CWinHttpFile::Write(LPCVOID pBuffer, DWORD nBufSize, DWORD& nBytesWritten)
{
    assert(m_h);
    BOOL br = ::WinHttpWriteData(m_h, pBuffer, nBufSize, &nBytesWritten);
    if (!br)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    return S_OK;
}





inline HRESULT CWinHttpFile::AddRequestHeaders(
    LPCWSTR pstrHeaders, 
    DWORD   dwHeadersLen,
    DWORD   dwFlags)
{
    assert(m_h);
    BOOL br = ::WinHttpAddRequestHeaders(m_h, pstrHeaders, dwHeadersLen, dwFlags);
    if (!br)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    return S_OK;
}

inline HRESULT CWinHttpFile::SendRequestEx(
    LPCTSTR pstrHeaders,
    DWORD   dwHeadersLen,
    LPVOID  lpOptional,
    DWORD   dwOptionalLen,
    DWORD   dwTotalLength,
    DWORD_PTR dwContext)
{
    assert(m_h);
    BOOL br = ::WinHttpSendRequest(m_h, pstrHeaders, dwHeadersLen, lpOptional, dwOptionalLen, dwTotalLength, dwContext);
    if (!br)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    return S_OK;
}

inline HRESULT CWinHttpFile::SendRequestEx(DWORD dwContentLength)
{
    assert(m_h);
    BOOL br = ::WinHttpSendRequest(m_h, NULL, 0, NULL, 0, dwContentLength, 0);
    if (!br)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    return S_OK;
}

inline HRESULT CWinHttpFile::ReceiveResponse()
{
    assert(m_h);
    BOOL br = ::WinHttpReceiveResponse(m_h, NULL);
    if (!br)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    return S_OK;
}

inline HRESULT CWinHttpFile::QueryInfoStatusCode(DWORD& dwStatusCode) const
{
    assert(m_h);
    DWORD dwLen = sizeof(DWORD);

    BOOL br = ::WinHttpQueryHeaders(
        m_h,
        WINHTTP_QUERY_STATUS_CODE  | WINHTTP_QUERY_FLAG_NUMBER ,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &dwStatusCode,
        &dwLen,
        WINHTTP_NO_HEADER_INDEX);
    if (!br)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    return S_OK;
}

inline HRESULT CWinHttpFile::QueryInfoContentLength(DWORD& dwContentLength) const
{
    assert(m_h);
    DWORD dwLen = sizeof(DWORD);

    BOOL br = ::WinHttpQueryHeaders(
        m_h,
        WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, 
        WINHTTP_HEADER_NAME_BY_INDEX,
        &dwContentLength,
        &dwLen,
        WINHTTP_NO_HEADER_INDEX);
    if (!br)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    return S_OK; 
}

inline HRESULT CWinHttpFile::QueryInfo_AsString(DWORD dwInfoLevel, CString& strValue)
{
    assert(m_h);
    strValue.Empty();

    DWORD dwBytes = 0;

    // First, use WinHttpQueryHeaders to obtain the size of the buffer.
    ::WinHttpQueryHeaders(
        m_h,
        dwInfoLevel,
        WINHTTP_HEADER_NAME_BY_INDEX,
        WINHTTP_NO_OUTPUT_BUFFER,
        &dwBytes, WINHTTP_NO_HEADER_INDEX);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return AtlHresultFromLastError();

    
    DWORD dwCharLen = (dwBytes + 1) / sizeof(WCHAR);
    LPCWSTR lpBuffer = strValue.GetBuffer(dwCharLen);
    if (!lpBuffer)
        return E_OUTOFMEMORY;


    // Now, use WinHttpQueryHeaders to retrieve the header.
    DWORD dwReadBytes = dwCharLen * sizeof(WCHAR);
    BOOL bRet = ::WinHttpQueryHeaders(
        m_h,
        dwInfoLevel,
        WINHTTP_HEADER_NAME_BY_INDEX, 
        (LPVOID)lpBuffer,
        &dwReadBytes,
        WINHTTP_NO_HEADER_INDEX);
    if (!bRet)
    {
        strValue.ReleaseBuffer(0);
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;
    }
    

    strValue.ReleaseBuffer(min(dwReadBytes, dwBytes / sizeof(WCHAR)));
    return S_OK;
}

inline HRESULT CWinHttpFile::QueryInfo_AsString(LPCWSTR lpszCustomHeader, CString& strValue)
{
    assert(m_h);
    strValue.Empty();

    DWORD dwBytes = 0;

    // First, use WinHttpQueryHeaders to obtain the size of the buffer.
    ::WinHttpQueryHeaders(
        m_h,
        WINHTTP_QUERY_CUSTOM,
        lpszCustomHeader,
        WINHTTP_NO_OUTPUT_BUFFER,
        &dwBytes, WINHTTP_NO_HEADER_INDEX);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return AtlHresultFromLastError();


    DWORD dwCharLen = (dwBytes + 1) / sizeof(WCHAR);
    LPCWSTR lpBuffer = strValue.GetBuffer(dwCharLen);
    if (!lpBuffer)
        return E_OUTOFMEMORY;


    // Now, use WinHttpQueryHeaders to retrieve the header.
    DWORD dwReadBytes = dwCharLen * sizeof(WCHAR);
    BOOL bRet = ::WinHttpQueryHeaders(
        m_h,
        WINHTTP_QUERY_CUSTOM,
        lpszCustomHeader,
        (LPVOID)lpBuffer,
        &dwReadBytes,
        WINHTTP_NO_HEADER_INDEX);
    if (!bRet)
    {
        strValue.ReleaseBuffer(0);
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;
    }


    strValue.ReleaseBuffer(min(dwReadBytes, dwBytes) / sizeof(WCHAR));
    return S_OK;
}

NS_WINMOD_END

#endif//WINHTTPMODFILE_H