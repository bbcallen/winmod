/**
* @file    winmodcom.h
* @brief   ...
* @author  bbcallen
* @date    2011-10-15 20:30
*/

#ifndef WINMODCOM_H
#define WINMODCOM_H

#include "winmodbase.h"

NS_WINMOD_BEGIN

template <class T>
class CWinComObject: public T
{
public:
    CWinComObject(): m_lRefCount(0) {;}
    virtual ~CWinComObject() {;}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
    {
        HRESULT hr = _InternalQueryInterface(riid, ppvObject);
        if (SUCCEEDED(hr))
            AddRef();

        return hr;
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        ::InterlockedIncrement(&m_lRefCount);
        return m_lRefCount;
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        LONG lRet = ::InterlockedDecrement(&m_lRefCount);
        if (0 == lRet)
            delete this;

        return lRet;
    }

private:
    volatile LONG m_lRefCount;
};



template <class T>
class CWinComSingleton: public T
{
public:
    static CWinComSingleton<T>* GetInstancePtr()
    {
        static CWinComSingleton<T> s_Instance;
        return &s_Instance;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
    {
        HRESULT hr = _InternalQueryInterface(riid, ppvObject);
        if (SUCCEEDED(hr))
            AddRef();

        return hr;
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return 1;
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        return 1;
    }

private:
    CWinComSingleton() {;}
    virtual ~CWinComSingleton() {;}
};



#define WINCOM_BEGIN_COM_MAP( CComImpl ) HRESULT _InternalQueryInterface( REFIID riid, void **ppvObject ) { \
    HRESULT hr = E_FAIL; \
    if( ppvObject == NULL ) {return E_INVALIDARG;

#define WINCOM_INTERFACE_ENTRY( I ) 	 } else if( riid == __uuidof( I ) || riid == IID_IUnknown ) { \
    *ppvObject = static_cast<I*>(this); \
    hr = S_OK;

#define WINCOM_INTERFACE_ENTRY2( I1, I2 )     } else if( riid == __uuidof( I2 ) || riid == IID_IUnknown ) { \
    *ppvObject = static_cast<I2*>(static_cast<I1*>(this)); \
    hr = S_OK;

#define WINCOM_INTERFACE_ENTRY_IID( IID__, I )  } else if( riid == IID__ || riid == IID_IUnknown ) { \
    *ppvObject = static_cast<I*>(this); \
    hr = S_OK;

#define WINCOM_INTERFACE_INHERIT( BASE )  } else if( SUCCEEDED(hr = BASE::_InternalQueryInterface(riid, ppvObject) ) ) { \
    ;

#define WINCOM_END_COM_MAP()  } else { \
    *ppvObject = NULL; \
    hr = E_NOINTERFACE; } \
    return hr; }

NS_WINMOD_END

#endif//WINMODCOM_H