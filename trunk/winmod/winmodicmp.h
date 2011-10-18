/**
* @file    winmodicmp.h
* @brief   ...
* @author  bbcallen
* @date    2011-10-17 15:14
*/

#ifndef WINMODICMP_H
#define WINMODICMP_H

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Iphlpapi.h>
#include <Icmpapi.h>
#pragma comment(lib, "Iphlpapi.lib")
#include "winmodbase.h"

//////////////////////////////////////////////////////////////////////////
// NOTICE!
/*
    Stop Debugging before IcmpSendEcho return, may cause 
    BLUE SCREEN:  PROCESS_HAS_LOCKED_PAGES

    See:
    http://social.msdn.microsoft.com/Forums/en-us/vclanguage/thread/c0d616a2-a1ea-43fb-831c-b1f51a957639
*/

NS_WINMOD_BEGIN

class CWinModIcmpFile
{
public:
    CWinModIcmpFile();
    ~CWinModIcmpFile() {Close();}

    HRESULT Create();
    void    Close();
    DWORD   SendEcho(
        IPAddr DestinationAddress,
        LPVOID RequestData,
        WORD RequestSize,
        PIP_OPTION_INFORMATION RequestOptions,
        LPVOID ReplyBuffer,
        DWORD ReplySize,
        DWORD Timeout);
    HANDLE GetHandle() const {return m_hIcmpFile;}

protected:
    HANDLE m_hIcmpFile;
};

inline CWinModIcmpFile::CWinModIcmpFile(): m_hIcmpFile(INVALID_HANDLE_VALUE)
{
}

inline HRESULT CWinModIcmpFile::Create()
{
    Close();

    m_hIcmpFile = ::IcmpCreateFile();
    if (INVALID_HANDLE_VALUE == m_hIcmpFile)
        return MAKE_WIN32_ERROR(GetLastError());

    return S_OK;
}

inline void CWinModIcmpFile::Close()
{
    if (INVALID_HANDLE_VALUE == m_hIcmpFile)
        return;

    ::IcmpCloseHandle(m_hIcmpFile);
    m_hIcmpFile = INVALID_HANDLE_VALUE;
}

inline DWORD CWinModIcmpFile::SendEcho(
        IPAddr DestinationAddress,
        LPVOID RequestData,
        WORD RequestSize,
        PIP_OPTION_INFORMATION RequestOptions,
        LPVOID ReplyBuffer,
        DWORD ReplySize,
        DWORD Timeout)
{
    assert(INVALID_HANDLE_VALUE != m_hIcmpFile);
    if (INVALID_HANDLE_VALUE == m_hIcmpFile)
        return 0;

    return ::IcmpSendEcho(
        m_hIcmpFile,
        DestinationAddress,
        RequestData,
        RequestSize,
        RequestOptions,
        ReplyBuffer,
        ReplySize,
        Timeout);
}








class CWinModIcmpTracert
{
public:
    enum {
        WINMOD_ICMP_TIMEOUT_DEFAULT = 4000,
        WINMOD_ICMP_MAX_HOP_DEFAULT = 30
    };

    CWinModIcmpTracert();
    ~CWinModIcmpTracert() {Close();}

    BOOL    FindFirstHop(
        IPAddr DestinationAddress,
        DWORD dwMaxHop = WINMOD_ICMP_MAX_HOP_DEFAULT,
        DWORD dwTimeout = WINMOD_ICMP_TIMEOUT_DEFAULT);
    BOOL    FindNextHop();
    BOOL    FindCurrentHop();
    void    Close();

    BOOL    IsEndOfEcho();
    DWORD   GetCurrentTtl() const {return m_IpOptInfo.Ttl;}
    DWORD   GetDestHops()   const {return m_dwDestHops;}

public:
    ICMP_ECHO_REPLY         m_EchoReply;

protected:
    DWORD                   m_dwTimeout;
    IPAddr                  m_DestAddress;
    IP_OPTION_INFORMATION   m_IpOptInfo;
    DWORD                   m_dwDestHops;
    DWORD                   m_dwMaxHops;

protected:
    CWinModIcmpFile m_hIcmpFile;
};

inline CWinModIcmpTracert::CWinModIcmpTracert()
    : m_dwTimeout(WINMOD_ICMP_TIMEOUT_DEFAULT)
    , m_dwDestHops(ULONG_MAX)
    , m_dwMaxHops(WINMOD_ICMP_MAX_HOP_DEFAULT)
{
    ZeroMemory(&m_DestAddress, sizeof(m_DestAddress));
    ZeroMemory(&m_IpOptInfo, sizeof(m_IpOptInfo));
    ZeroMemory(&m_EchoReply, sizeof(m_EchoReply));
}

inline BOOL CWinModIcmpTracert::FindFirstHop(IPAddr DestinationAddress, DWORD dwMaxHops, DWORD dwTimeout)
{
    Close();

    HRESULT hr = m_hIcmpFile.Create();
    if (FAILED(hr))
        return FALSE;

    m_DestAddress           = DestinationAddress;
    m_IpOptInfo.Ttl         = UCHAR_MAX;
    m_EchoReply.Options.Ttl = 1;

    DWORD dwEchoRet = m_hIcmpFile.SendEcho(
        m_DestAddress,
        NULL, 0,
        NULL,
        &m_EchoReply,
        sizeof(m_EchoReply),
        m_dwTimeout);
    if (0 == dwEchoRet)
        return FALSE;

    m_IpOptInfo.Ttl = 0;
    m_dwDestHops = m_EchoReply.Options.Ttl;
    m_dwMaxHops  = dwMaxHops;
    return FindCurrentHop();
}

inline BOOL CWinModIcmpTracert::FindNextHop()
{
    if (IsEndOfEcho())
        return FALSE;

    m_IpOptInfo.Ttl++;
    if (m_EchoReply.Address == m_DestAddress)
    {
        m_hIcmpFile.Close();
        return FALSE;
    }

    if (!FindCurrentHop())
        return FALSE;

    return TRUE;
}

inline BOOL CWinModIcmpTracert::FindCurrentHop()
{
    if (IsEndOfEcho())
        return FALSE;

    DWORD dwEchoRet = m_hIcmpFile.SendEcho(
        m_DestAddress,
        NULL, 0,
        &m_IpOptInfo,
        &m_EchoReply,
        sizeof(m_EchoReply),
        m_dwTimeout);
    if (0 == dwEchoRet)
        return TRUE;

    return TRUE;
}

inline void CWinModIcmpTracert::Close()
{
    m_hIcmpFile.Close();
    ZeroMemory(&m_DestAddress, sizeof(m_DestAddress));
    ZeroMemory(&m_IpOptInfo, sizeof(m_IpOptInfo));
    ZeroMemory(&m_EchoReply, sizeof(m_EchoReply));
    m_dwTimeout  = WINMOD_ICMP_TIMEOUT_DEFAULT;
    m_dwMaxHops  = WINMOD_ICMP_MAX_HOP_DEFAULT;
    m_dwDestHops = ULONG_MAX;
}

inline BOOL CWinModIcmpTracert::IsEndOfEcho()
{
    if (INVALID_HANDLE_VALUE == m_hIcmpFile.GetHandle())
        return TRUE;

    if (m_IpOptInfo.Ttl > m_dwMaxHops)
        return TRUE;

    if (m_IpOptInfo.Ttl > m_dwDestHops)
        return TRUE;

    return FALSE;
}

NS_WINMOD_END

#endif//WINMODICMP_H