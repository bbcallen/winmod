/**
* @file    winpath.h
* @brief   ...
* @author  bbcallen
* @date    2009-03-07  15:51
*/

#ifndef WINPATH_H
#define WINPATH_H

#include <shobjidl.h>
#include <atlstr.h>
#include "winmod\winmodbase.h"

NS_WINMOD_BEGIN

// 未完全实现,不建议使用
class CWinPathApi
{
public:
    enum
    {
        WIN_PATH_MAX_UNICODE_PATH           = 32767 + 4,
        WIN_PATH_UNICODE_PATH_PREFIX        = 4,
        WIN_PATH_UNICODE_UNC_PATH_PREFIX    = 7,
    };

    static HRESULT  ExpandFullPathName(CString& strFullPathName);

    static HRESULT  ExpandLongPathName(CString& strFullPathName);

    // long path, lower case
    static void     NormalizePathName(CString& strFullPathName);


    // replace 'lpPattern' at begging of 'strPath' with 'csidl'
    static BOOL     ExpandSpecialFolderPathAtBeginning(CString& strPath, LPCWSTR lpPattern, int csidl);

    // replace 'lpPattern' at begging of 'strPath' with 'lpExpandAs'
    static BOOL     ExpandPatternAtBeginning(CString& strPath, LPCWSTR lpPattern, LPCWSTR lpExpandAs);

    static BOOL     ExpandEnvironmentStrings(CString& strPath);


    // replace unaccessible path, such as '\??\' '\\SystemRoot'
    static BOOL     ExpandAsAccessiblePath(CString& strPath);



    static BOOL     HasUnicodePrefix(LPCWSTR pszPath);

    // for non-unicode path, return after "\\\\"
    // for unicode path, return after "\\\\?\\UNC\\"
    // for else, return whole string
    static LPCWSTR  FindAfterAnyPrefix(LPCWSTR pszPath);

    static LPCWSTR  FindAfterUnicodePrefix(LPCWSTR pszPath);

    static LPCWSTR  FindFileName(LPCWSTR pszPath);


    static LPCWSTR  FindExtension(LPCWSTR pszPath);

    static HRESULT  CreateLnkFile(LPCWSTR pszPath, LPCWSTR pszDesc, LPCWSTR pszLnkFilePath);
    static HRESULT  ResolveLnkFile(LPCWSTR pszLnkFile, CString& strTargetPath, DWORD dwFlag = SLR_NOUPDATE | SLR_NOTRACK | SLR_NOSEARCH | SLR_NO_UI);

    static BOOL     IsDots(LPCWSTR pszPath);

    static BOOL     IsDirectory(LPCWSTR pszPath);
    static BOOL     IsRelative(LPCWSTR pszPath);
    static BOOL     IsRoot(LPCWSTR pszPath);
    static BOOL     IsUNC(LPCWSTR pszPath);
    static BOOL     IsUNCServer(LPCWSTR pszPath);
    static BOOL     IsUNCServerShare(LPCWSTR pszPath);
    static BOOL     IsFileExisting(LPCWSTR pszPath);

    static BOOL     IsLnkFile(LPCWSTR pszPath);
};



// 未完全实现,不建议使用
class CWinPath
{
// Constructors
public:

    CWinPath();
	CWinPath(const CWinPath& path);
	CWinPath(LPCWSTR pszPath);


// Operators
public:
    operator const CString& () const;
    operator CString& ();
    operator LPCWSTR() const;
    CWinPath& operator+=(LPCWSTR pszMore);

// Operations
public:
    void        AddBackslash();
    BOOL        AddExtension(LPCWSTR pszExtension);
    BOOL        Append(LPCWSTR pszMore);
    void        BuildRoot(int iDrive);
    void        Canonicalize();
    void        Combine(LPCWSTR pszDir, LPCWSTR pszFile);
    //CWinPath    CommonPrefix(LPCWSTR pszOther );
    //BOOL        CompactPath(HDC hDC, UINT nWidth);
    BOOL        CompactPathEx(UINT nMaxChars, DWORD dwFlags = 0);
    //BOOL        FileExists() const;
    int         FindExtension() const;
    int         FindFileName() const;
    //int         GetDriveNumber() const;
    CString     GetExtension() const;
    BOOL        IsDirectory() const;
    //BOOL        IsFileSpec() const;
    //BOOL        IsPrefix(LPCWSTR pszPrefix ) const;
    BOOL        IsRelative() const;
    BOOL        IsRoot() const;
    //BOOL        IsSameRoot(LPCWSTR pszOther) const;
    BOOL        IsUNC() const;
    BOOL        IsUNCServer() const;
    BOOL        IsUNCServerShare() const;
    //BOOL        MakePretty();
    //BOOL        MatchSpec(LPCWSTR pszSpec) const;
    //void        QuoteSpaces();
    //BOOL        RelativePathTo(LPCWSTR pszFrom, DWORD dwAttrFrom, LPCWSTR pszTo, DWORD dwAttrTo );
    void        RemoveArgs();
    void        RemoveBackslash();
    //void        RemoveBlanks();
    void        RemoveExtension();
    BOOL        RemoveFileSpec();
    //BOOL        RenameExtension(LPCWSTR pszExtension);
    //int         SkipRoot() const;
    void        StripPath();
    BOOL        StripToRoot();
    void        UnquoteSpaces();


// Extra Operation
public:

    BOOL        IsExisting() const;
    void        RemoveSingleArg();

    BOOL        HasUnicodePrefix() const;
    void        RemoveUnicodePrefix();
    void        AddUnicodePrefix();
    CWinPath    GetPathWithoutUnicodePrefix() const;

    HRESULT     ExpandFullPathName();
    HRESULT     ExpandLongPathName();
    void        ExpandNormalizedPathName();

    CString m_strPath;
};




inline CWinPath::CWinPath() throw()
{
}

inline CWinPath::CWinPath(const CWinPath& path):
    m_strPath(path.m_strPath)
{
}

inline CWinPath::CWinPath(LPCWSTR pszPath):
    m_strPath(pszPath )
{
}

inline CWinPath::operator const CString& () const throw()
{
    return m_strPath;
}

inline CWinPath::operator CString& () throw()
{
    return m_strPath;
}

inline CWinPath::operator LPCWSTR() const throw()
{
    return m_strPath;
}

inline CWinPath& CWinPath::operator+=(LPCWSTR pszMore)
{
    Append( pszMore );

    return *this;
}


NS_WINMOD_END

#endif//WINPATH_H