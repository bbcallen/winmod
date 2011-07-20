/**
* @file    wincmdline.h
* @brief   ...
* @author  bbcallen
* @date    2011-03-05 13:20
*/

#ifndef WINCMDLINE_H
#define WINCMDLINE_H

#include <assert.h>
#include <atlstr.h>
#include <atlcoll.h>
#include "winmodbase.h"

NS_WINMOD_BEGIN

#define WINMOD_CMDL_PAR_ATTR__

/*
http://en.wikipedia.org/wiki/Wikipedia:Manual_of_Style_%28command-line_examples%29
Terminology: 
Option:     An option is a switch (something that modifies the general behavior of the command).

Parameter:  A parameter is a specific value, such as a file or host name.

Argument:   The term argument is used to refer to any of the space-separated strings that follow a command name, including both options and parameters.


//////////////////////////////////////////////////////////////////////////
styles:
== DOS ==

> DIR [options] [pattern ...]
> MOVE.EXE source target

== General Unix ==
$ ls [options] [file ...]
# mkfs [-t fstype] [fs-options] device
$ wget [options] "URI"
//////////////////////////////////////////////////////////////////////////




Example:

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
    WinMod::CWinCmdLine CmdLine;

    CmdLine.SetOption(L"option-1");
    CmdLine.SetOption(L"option-2", L"value");
    CmdLine.SetOption(L"option-3", 1024);
    CmdLine.SetOption(L"option-4", L"s p a c e");
    CmdLine.AddParam(L"param");

    CString strCmdLine = CmdLine.ToCmdLine();
    // equal to
    // -option-1 -option-2=value -option-3=1024 -option-4="s p a c e" param

    // Analyze Command Line
    CmdLine.Analyze(strCmdLine);

    BOOL    bHasOption1     = CmdLine.HasOption(L"option-1");               // TRUE
    CString strOption2      = CmdLine.GetOption(L"option-2").ToString();    // value
    INT     nOption3        = CmdLine.GetOption(L"option-3").ToInt();       // 1024
    CString strOption4      = CmdLine.GetOption(L"option-4").ToString();    // s p a c e
    CString strOption4Raw   = CmdLine.GetOption(L"option-4").ToRaw();       // "s p a c e"

    return 0;
}


*/

// used in SetOption and AddParam
#define WINMOD_CMD_ARG_FLAG__DISABLE_QUOTE      0x00010000      ///< opt value will not be quoted, for NSIS, setup.exe /D=c:\program files\App

class CWinCmdLine
{
public:

    class CWinCmdArg
    {
    public:
        enum ARG_TYPE {
            em_Null             = 0,
            em_Parameter        = 1,
            em_Option           = 2,
            em_OptionWithValue  = 3
        };

        CWinCmdArg()
            : m_Type(em_Null)
            , m_nArgIndex(-1)
            , m_nParamIndex(-1)
        {}

        BOOL    Analyze(LPCWSTR lpszArgument);
        CString ToRaw() const;     ///< raw value:         "C:\Program files\App"
        CString ToString() const;  ///< unquoted value     C:\Program files\App
        int     ToInt() const;
        INT64   ToInt64() const;

        ARG_TYPE GetParamAttr() const;

        BOOL    IsNull() const;
        BOOL    IsParam() const;            // no prefix:       command a.txt
        BOOL    IsOption() const;           // with prefix:     command -i
        BOOL    HasOptionValue() const;     // has opt value    command -i:a.txt

        CString GetKey() const;
        size_t  GetParamIndex() const;

        static  CWinCmdArg MakeParam(LPCWSTR lpszParam, DWORD dwFlag = 0, int nArgIndex = -1, int nParamIndex = -1);
        static  CWinCmdArg MakeOption(LPCWSTR lpszOptName, int nArgIndex = -1);
        static  CWinCmdArg MakeOptionWithValue(LPCWSTR lpszOptName, LPCWSTR lpszValue, DWORD dwFlag = 0, int nArgIndex = -1);
        
    private:
        ARG_TYPE    m_Type;
        CString     m_strKey;
        CString     m_strValue;
        CString     m_strRawValue;

        int         m_nArgIndex;    // zero-based arg index
        int         m_nParamIndex;  // zero-based param index

        friend class CWinCmdLine;
    };

    BOOL    Analyze();  // 解析当前进程的命令行
    BOOL    Analyze(CString strCmdLine);
    CString ToCmdLine(WCHAR chOptHeader = L'-', WCHAR chOptValueHeader = L'=');
    void    Reset();

    const CWinCmdArg GetOption(LPCWSTR lpszOptName);
    BOOL    HasOption(LPCWSTR lpszOptName);
    BOOL    SetOption(LPCWSTR lpszOptName);
    BOOL    SetOption(LPCWSTR lpszOptName, LPCWSTR lpszOptValue, DWORD dwFlag = 0);
    BOOL    SetOption(LPCWSTR lpszOptName, int nValue);
    BOOL    SetOption(LPCWSTR lpszOptName, INT64 nValue);
    void    RemoveOption(LPCWSTR lpszOptName);
    void    RemoveAllOptions();

    const CWinCmdArg GetParam(size_t nParamIndex);
    size_t  GetParamCount();
    BOOL    HasParam(size_t nParamIndex);
    BOOL    AddParam(LPCWSTR lpszParam, DWORD dwFlag = 0);
    BOOL    SetParam(size_t nParamIndex, LPCWSTR lpszParam, DWORD dwFlag = 0);
    BOOL    SetParam(size_t nParamIndex, int nValue, DWORD dwFlag = 0);
    BOOL    SetParam(size_t nParamIndex, INT64 nValue, DWORD dwFlag = 0);
    void    RemoveParam(size_t nParamIndex);
    void    RemoveAllParams();

    static  void QuoteStringIfSpace(CString& strText);
    static  int  UnquoteString(CString& strQuoted);   // return quoted level

private:
    static  LPCWSTR TrimSpace(LPCWSTR lpszText);
    const CWinCmdArg GetArgument(size_t nArgIndex);

    typedef CAtlArray<CWinCmdArg>       CRawArgArray;
    typedef CAtlMap<CString, size_t>    COptionMap;
    typedef CAtlArray<size_t>           CParamMap;

    CRawArgArray    m_RawArgArray;
    COptionMap      m_OptionMap;
    CParamMap       m_ParamMap;
};


inline CString CWinCmdLine::CWinCmdArg::ToRaw() const
{
    return m_strRawValue;
}

inline CString CWinCmdLine::CWinCmdArg::ToString() const
{
    return m_strValue;
}

inline int CWinCmdLine::CWinCmdArg::ToInt() const
{
    return _wtoi(ToString());
}

inline INT64 CWinCmdLine::CWinCmdArg::ToInt64() const
{
    return _wtoi64(ToString());
}

inline CWinCmdLine::CWinCmdArg::ARG_TYPE CWinCmdLine::CWinCmdArg::GetParamAttr() const
{
    return m_Type;
}

inline BOOL CWinCmdLine::CWinCmdArg::IsNull() const
{
    return m_Type == CWinCmdLine::CWinCmdArg::em_Null;
}

inline BOOL CWinCmdLine::CWinCmdArg::IsParam() const
{
    return m_Type == CWinCmdLine::CWinCmdArg::em_Parameter;
}

inline BOOL CWinCmdLine::CWinCmdArg::IsOption() const
{
    return
        m_Type == CWinCmdLine::CWinCmdArg::em_Option ||
        m_Type == CWinCmdLine::CWinCmdArg::em_OptionWithValue;
}

inline BOOL CWinCmdLine::CWinCmdArg::HasOptionValue() const
{
    return m_Type == CWinCmdLine::CWinCmdArg::em_OptionWithValue;
}

inline CString CWinCmdLine::CWinCmdArg::GetKey() const
{
    assert(IsOption());
    return m_strKey;
}

inline size_t CWinCmdLine::CWinCmdArg::GetParamIndex() const
{
    assert(IsParam());
    return m_nParamIndex;
}

NS_WINMOD_END

#endif//WINCMDLINE_H