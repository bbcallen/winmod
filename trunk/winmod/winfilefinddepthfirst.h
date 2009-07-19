/**
* @file    winfilefinddepthfirst.h
* @brief   ...
* @author  bbcallen
* @date    2009-03-06  18:06
*/

#ifndef WINFILEFINDDEPTHFIRST_H
#define WINFILEFINDDEPTHFIRST_H

#include <atlstr.h>
#include <atlcoll.h>
#include "winmod\winmodbase.h"
#include "winmod\winpath.h"
#include "winmod\winfilefinddata.h"

NS_WINMOD_BEGIN

// skip . and ..
class CWinFileFindDepthFirst: public CWinFileFindData
{
public:
    CWinFileFindDepthFirst();
    ~CWinFileFindDepthFirst();
    
// Operations
public:
    BOOL    FindFirstFile(
        LPCWSTR pszDirectory,
        LPCWSTR pszFileSpec = NULL);

    BOOL    FindNextFile();

    BOOL    FindNextSiblingFile();

    BOOL    FindFirstChildFile(LPCWSTR pszFileSpec = NULL);

    void    SkipCurrentDirectory();

    void    Close();

// Attributes
public:
    CString GetFullPath();

    CString GetCurrentDirectory();


protected:

    void    CloseTop();

    void    PushNode();

    BOOL    PopNode();



    // inner context
    class CWinFileFindNode: public CWinFileFindData
    {
    public:
        CWinFileFindNode(): m_hContext(INVALID_HANDLE_VALUE) {}

        HANDLE          m_hContext;
        WIN32_FIND_DATA m_findData;
    };

    CWinPath                    m_pathParent;

    HANDLE                      m_hContext;
    CAtlList<CWinFileFindNode>  m_findStack;
};

NS_WINMOD_END

#endif//WINFILEFINDDEPTHFIRST_H