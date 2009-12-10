/**
* @file    winfilefinddepthfirst.cpp
* @brief   ...
* @author  bbcallen
* @date    2009-03-06  18:06
*/

#include "stdafx.h"
#include "winfilefinddepthfirst.h"

#include <assert.h>
#include "winpath.h"

using namespace WinMod;

CWinFileFindDepthFirst::CWinFileFindDepthFirst():
    m_hContext(INVALID_HANDLE_VALUE)
{

}

CWinFileFindDepthFirst::~CWinFileFindDepthFirst()
{
    Close();
}

BOOL CWinFileFindDepthFirst::FindFirstFile(
    LPCWSTR pszDirectory,
    LPCWSTR pszFileSpec)
{
    Close();

    assert(pszDirectory);
    if (pszDirectory == NULL)
    {
        ::SetLastError(ERROR_BAD_ARGUMENTS);
        return FALSE;
    }



    // get whole path
    m_pathParent.m_strPath = pszDirectory;
    m_pathParent.ExpandFullPathName();
    m_pathParent.ExpandLongPathName();
    m_pathParent.AddUnicodePrefix();



    if (!m_pathParent.IsDirectory())
    {
        Close();
        return FALSE;
    }


    // use '*'
    if (!pszFileSpec || !*pszFileSpec)
        pszFileSpec = L"*";


    // combine full path
    CWinPath findPath;
    findPath.Combine(m_pathParent.m_strPath, pszFileSpec);


    // begin search
    m_hContext = CWinFileFindApi::FindFirstFileSkipDots(findPath, *this);
    if (INVALID_HANDLE_VALUE == m_hContext)
    {
        Close();
        return FALSE;
    }


    return TRUE;
}

BOOL CWinFileFindDepthFirst::FindNextFile()
{
    assert(INVALID_HANDLE_VALUE != m_hContext);
    if (INVALID_HANDLE_VALUE == m_hContext)
    {
        ::SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    BOOL bFound = this->FindFirstChildFile();
    if (bFound)
        return TRUE;

    bFound = this->FindNextSiblingFile();
    while (!bFound)
    {
        if (!PopNode())
            return FALSE;

        bFound = this->FindNextSiblingFile();
    }

    return bFound;
}

BOOL CWinFileFindDepthFirst::FindNextSiblingFile()
{
    assert(INVALID_HANDLE_VALUE != m_hContext);
    if (INVALID_HANDLE_VALUE == m_hContext)
    {
        ::SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }


    return CWinFileFindApi::FindNextFileSkipDots(m_hContext, *this);
}

BOOL CWinFileFindDepthFirst::FindFirstChildFile(LPCWSTR pszFileSpec)
{
    assert(INVALID_HANDLE_VALUE != m_hContext);
    if (INVALID_HANDLE_VALUE == m_hContext)
    {
        ::SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }


    if (!CWinFileFindData::IsDirectory())
        return FALSE;


    if (CWinFileFindData::IsDots())
        return FALSE;


    PushNode();
    m_hContext = INVALID_HANDLE_VALUE;



    // use '*'
    if (!pszFileSpec || !*pszFileSpec)
        pszFileSpec = L"*";


    // combine full path
    CWinPath findPath;
    findPath.Combine(m_pathParent.m_strPath, pszFileSpec);



    m_hContext = CWinFileFindApi::FindFirstFileSkipDots(findPath, *this);
    if (INVALID_HANDLE_VALUE == m_hContext)
    {
        CloseTop();
        PopNode();
        return FALSE;
    }



    return TRUE;
}

void CWinFileFindDepthFirst::SkipCurrentDirectory()
{
    PopNode();
}

void CWinFileFindDepthFirst::Close()
{
    m_pathParent.m_strPath.Empty();

    while (PopNode())
    {
        NULL;
    }

    CloseTop();
}





CString CWinFileFindDepthFirst::GetFullPath()
{
    return m_pathParent.m_strPath + L"\\" + CWinFileFindData::GetFileName();
}

CString CWinFileFindDepthFirst::GetCurrentDirectory()
{
    return m_pathParent.m_strPath;
}





void CWinFileFindDepthFirst::CloseTop()
{
    ResetFindData();

    if (INVALID_HANDLE_VALUE != m_hContext)
    {
        CWinFileFindApi::FindClose(m_hContext);
        m_hContext = INVALID_HANDLE_VALUE;
    }
}

void CWinFileFindDepthFirst::PushNode()
{
    assert(INVALID_HANDLE_VALUE != m_hContext);

    m_pathParent.AddBackslash();
    m_pathParent.m_strPath.Append(CWinFileFindData::GetFileName());

    POSITION pos = m_findStack.AddTail();
    CWinFileFindNode& node = m_findStack.GetAt(pos);

    node.m_hContext = m_hContext;
    node.m_findData = m_findData;
}

BOOL CWinFileFindDepthFirst::PopNode()
{
    if (m_findStack.IsEmpty())
        return FALSE;

    CloseTop();

    CWinFileFindNode& node = m_findStack.GetTail();
    
    m_hContext = node.m_hContext;
    m_findData = node.m_findData;

    m_findStack.RemoveTailNoReturn();

    m_pathParent.RemoveFileSpec();
    return TRUE;
}