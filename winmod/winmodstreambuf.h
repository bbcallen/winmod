/**
* @file    winmodstreambuf.h
* @brief   ...
* @author  bbcallen
* @date    2011-07-28 21:59
*/

#ifndef WINMODSTREAMBUF_H
#define WINMODSTREAMBUF_H

#include <winmod\winmodsync.h>

NS_WINMOD_BEGIN

class CWinModStreamBufferBlock
{
public:
    CWinModStreamBufferBlock(): m_nOffset(0) {}
    ~CWinModStreamBufferBlock()
    {
        ReleaseData();
    }

    HRESULT AllocData(const void* pBuffer, size_t nLen)
    {
        m_byData.SetCount(nLen);
        if (m_byData.GetCount() != nLen)
            return E_OUTOFMEMORY;

        memcpy(m_byData.GetData(), pBuffer, nLen);
        return S_OK;
    }

    void ReleaseData()
    {
        m_byData.SetCount(0);
    }

    BOOL IsEmpty()
    {
        return (GetLength() == 0);
    }

    size_t GetLength()
    {
        assert(m_byData.GetCount() >= m_nOffset);
        return m_byData.GetCount() - m_nOffset;
    }

    void Read(void* pBuffer, size_t nLen, size_t& nReadLen)
    {
        assert(pBuffer);
        nReadLen = 0;

        size_t nToReadLen = min(nLen, GetLength());
        if (0 == nToReadLen)
            return;

        memcpy(pBuffer, m_byData.GetData() + m_nOffset, nToReadLen);
        m_nOffset += nToReadLen;
        nReadLen   = nToReadLen;
    }

protected:
    CAtlArray<BYTE> m_byData;
    size_t          m_nOffset;
};

template <class SyncTraits = WinMod::CWinModSyncTraits>
class CWinModStreamBufferT
{
public:
    typedef CAtlList<CWinModStreamBufferBlock> CBufferList;
    typedef typename SyncTraits::CObjLock CObjLock;
    typedef typename SyncTraits::CObjGuard CObjGuard;

    CWinModStreamBufferT()
        : m_nBufferLen(0)
        , m_bNoMoreData(FALSE)
    {
    }
    
    
    size_t GetBufferLength()
    {
        return m_nBufferLen;
    }


    BOOL IsEmpty()
    {
        return 0 == GetBufferLength();
    }


    void RemoveAll()
    {
        CObjGuard Guard(m_ObjLock);
        m_BufferList.RemoveAll();
        m_nBufferLen = 0;
        m_bNoMoreData = FALSE;
    }


    HRESULT Read(void* pBuffer, size_t nLen)
    {
        assert(pBuffer);
        CObjGuard Guard(m_ObjLock);

        assert(m_nBufferLen >= 0);
        if (nLen < m_nBufferLen)
            return E_FAIL;

        size_t nReadLen = 0;
        HRESULT hr = Read(pBuffer, nLen, nReadLen);
        if (FAILED(hr))
            return hr;

        assert(nLen == nReadLen);
        return S_OK;   
    }


    HRESULT Read(void* pBuffer, size_t nLen, size_t& nReadLen)
    {
        assert(pBuffer);
        CObjGuard Guard(m_ObjLock);

        assert(m_nBufferLen >= 0);

        nReadLen = 0;
        while (nReadLen < nLen)
        {
            if (m_BufferList.IsEmpty())
                break;


            CWinModStreamBufferBlock& Block = m_BufferList.GetHead();
            if (Block.IsEmpty())
            {
                m_BufferList.RemoveHeadNoReturn();
                continue;
            }

            
            assert(nLen > nReadLen);
            size_t nToReadLen = nLen - nReadLen;
            size_t nBlockReadLen = 0;
            Block.Read((BYTE*)pBuffer + nReadLen, nToReadLen, nBlockReadLen);

            if (Block.IsEmpty())
                m_BufferList.RemoveHeadNoReturn();

            nReadLen     += nBlockReadLen;
            m_nBufferLen -= nBlockReadLen;
        }

        return S_OK;  
    }



    HRESULT Write(const void* pBuffer, size_t nLen)
    {
        assert(pBuffer);
        CObjGuard Guard(m_ObjLock);

        POSITION pos = m_BufferList.AddTail();
        CWinModStreamBufferBlock& Block = m_BufferList.GetAt(pos);
        HRESULT hr = Block.AllocData(pBuffer, nLen);
        if (FAILED(hr))
            return hr;

        assert(m_nBufferLen >= 0);
        m_nBufferLen += nLen;
        return S_OK;
    }


    HRESULT WriteEnd()
    {
        CObjGuard Guard(m_ObjLock);
        
        m_bNoMoreData = TRUE;
        return S_OK;
    }


    BOOL IsEnd()
    {
        CObjGuard Guard(m_ObjLock);

        if (IsEmpty() && NoMoreData())
            return TRUE;

        return FALSE;
    }


    BOOL NoMoreData()
    {
        return m_bNoMoreData;
    }


protected:

    CObjLock            m_ObjLock;
    CBufferList         m_BufferList;
    volatile size_t     m_nBufferLen;
    BOOL                m_bNoMoreData;
};
typedef CWinModStreamBufferT<> CWinModStreamBuffer;

NS_WINMOD_END

#endif//WINMODSTREAMBUF_H