/**
* @file    winmodinetfile.h
* @brief   ...
* @author  bbcallen
* @date    2009-08-07 15:10
*/

#ifndef WINMODINETFILE_H
#define WINMODINETFILE_H

#include "winmod\winmodinethandle.h"

NS_WINMOD_BEGIN

class CInetFile: public CInetHandle
{
public:
    CInetFile();

    CInetFile(CInetFile& h);

    explicit CInetFile(HANDLE h);

    virtual ~CInetFile();

    CInetFile& operator=(CInetFile& h);


    HRESULT Read(LPVOID pBuffer, DWORD nBufSize, DWORD& nBytesRead);

    HRESULT Write(LPCVOID pBuffer, DWORD nBufSize, DWORD& nBytesWritten);

    HRESULT Seek(LONGLONG nOffset, DWORD dwFrom = FILE_CURRENT);
};




inline CInetFile::CInetFile():
    CInetHandle()
{
}

inline CInetFile::CInetFile(CInetFile& h):
    CInetHandle(h)
{
}

inline CInetFile::CInetFile(HANDLE h):
    CInetHandle(h)
{
}

inline CInetFile::~CInetFile()
{
}

inline CInetFile& CInetFile::operator=(CInetFile& h)
{
    *(CInetHandle*)this = (CInetHandle&)h;
}



inline HRESULT CInetFile::Read(LPVOID pBuffer, DWORD nBufSize, DWORD& nBytesRead)
{
    assert(m_h);
    BOOL br = ::InternetReadFile(m_h, pBuffer, nBufSize, &nBytesRead);
    if (!br)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    return S_OK;
}

inline HRESULT CInetFile::Write(LPCVOID pBuffer, DWORD nBufSize, DWORD& nBytesWritten)
{
    assert(m_h);
    BOOL br = ::InternetWriteFile(m_h, pBuffer, nBufSize, &nBytesWritten);
    if (!br)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    return S_OK;
}

inline HRESULT CInetFile::Seek(LONGLONG nOffset, DWORD dwFrom)
{
    assert(m_h);
    BOOL br = ::InternetSetFilePointer(m_h, (LONG)nOffset, NULL, dwFrom, 0);
    if (!br)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    return S_OK;
}

NS_WINMOD_END

#endif//WINMODINETFILE_H