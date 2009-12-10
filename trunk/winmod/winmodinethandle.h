/**
* @file    winmodinethandle.h
* @brief   ...
* @author  bbcallen
* @date    2009-08-07 15:09
*/

#ifndef WINMODINETHANDLE_H
#define WINMODINETHANDLE_H

#include <assert.h>
#include <wininet.h>
#include "winmod\winmodbase.h"

NS_WINMOD_BEGIN

class CInetHandle
{
public:
    CInetHandle();

    CInetHandle(CInetHandle& h);

    explicit CInetHandle(HANDLE h);

    virtual ~CInetHandle();

    CInetHandle& operator=(CInetHandle& h);



    operator        HINTERNET() const;
    void            Attach(HINTERNET h);
    HINTERNET       Detach();
    void            Close();

    HRESULT         DoSetOptionDWORD(DWORD dwOption, DWORD  dwValue);
    HRESULT         DoGetOptionDWORD(DWORD dwOption, DWORD& dwValue);

public:
    HINTERNET m_h;
};


inline CInetHandle::CInetHandle():
    m_h(NULL)
{
}

inline CInetHandle::CInetHandle(CInetHandle& h):
    m_h(NULL)
{
    Attach(h.Detach());
}

inline CInetHandle::CInetHandle(HANDLE h):
    m_h(h)
{
}

inline CInetHandle::~CInetHandle()
{
    if (m_h != NULL)
    {
        Close();
    }
}

inline CInetHandle& CInetHandle::operator=(CInetHandle& h)
{
    if (this != &h)
    {
        if(m_h != NULL)
        {
            Close();
        }
        Attach(h.Detach());
    }

    return(*this);
}

inline CInetHandle::operator HANDLE() const
{
    return(m_h);
}

inline void CInetHandle::Attach(HANDLE h)
{
    assert(m_h == NULL);
    m_h = h;  // Take ownership
}

inline HANDLE CInetHandle::Detach()
{
    HANDLE h;

    h = m_h;  // Release ownership
    m_h = NULL;

    return(h);
}

inline void CInetHandle::Close()
{
    if(m_h != NULL)
    {
        ::InternetCloseHandle(m_h);
        m_h = NULL;
    }
}

inline HRESULT CInetHandle::DoSetOptionDWORD(DWORD dwOption, DWORD  dwValue)
{
    assert(m_h);
    BOOL br = ::InternetSetOption(m_h, dwOption, &dwValue, sizeof(DWORD));
    if (!br)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    return S_OK;
}

inline HRESULT CInetHandle::DoGetOptionDWORD(DWORD dwOption, DWORD& dwValue)
{
    assert(m_h);
    DWORD dwBufLen = sizeof(DWORD);
    BOOL br = ::InternetQueryOption(m_h, dwOption, &dwValue, &dwBufLen);
    if (!br)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    return S_OK;
}

NS_WINMOD_END

#endif//WINMODINETHANDLE_H