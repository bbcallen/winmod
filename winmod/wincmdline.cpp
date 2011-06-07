/**
* @file    wincmdline.cpp
* @brief   ...
* @author  bbcallen
* @date    2011-03-05 13:20
*/

#include "stdafx.h"
#include "wincmdline.h"

#pragma warning(disable: 4018)  // 'expression' : signed/unsigned mismatch
#pragma warning(disable: 4244)  // 'argument' : conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable: 4267)  // 'var' : conversion from 'size_t' to 'type', possible loss of data

NS_WINMOD_USING

BOOL CWinCmdLine::CWinCmdArg::Analyze(LPCWSTR lpszArgument)
{
    if (!lpszArgument || !*lpszArgument)
        return FALSE;


    m_strKey.Empty();
    m_strRawValue = lpszArgument;

    m_strValue = lpszArgument;
    if (CWinCmdLine::UnquoteString(m_strValue))
    {   // command "argument"
        m_Type = em_Parameter;
        return TRUE;
    }

    

    assert(!m_strValue.IsEmpty());
    if (L'/' != m_strValue[0] && L'-' != m_strValue[0])
    {   // command argument
        m_Type = em_Parameter;
        return TRUE;
    }


    //////////////////////////////////////////////////////////////////////////
    // not 'parameter', try 'option' analyse
    int nPos = 1;
    m_strKey = m_strValue.Tokenize(L"=:", nPos);
    if (m_strKey.IsEmpty())
        return FALSE;


    m_strRawValue = (nPos == -1) ? L"" : m_strValue.Mid(nPos);
    m_strValue    = m_strRawValue;
    CWinCmdLine::UnquoteString(m_strValue);
    m_Type = m_strRawValue.IsEmpty() ? CWinCmdLine::CWinCmdArg::em_Option : CWinCmdLine::CWinCmdArg::em_OptionWithValue;
    return TRUE;
}

CWinCmdLine::CWinCmdArg CWinCmdLine::CWinCmdArg::MakeParam(LPCWSTR lpszParam, DWORD dwFlag, int nArgIndex, int nParamIndex)
{
    assert(lpszParam && *lpszParam);

    CWinCmdArg NewArg;
    NewArg.m_Type        = CWinCmdArg::em_Parameter;
    NewArg.m_strRawValue = lpszParam;
    NewArg.m_strValue    = lpszParam;
    NewArg.m_nArgIndex   = nArgIndex;
    NewArg.m_nParamIndex = nParamIndex;

    if (!(dwFlag & WINMOD_CMD_ARG_FLAG__DISABLE_QUOTE))
    {
        QuoteStringIfSpace(NewArg.m_strRawValue);
    }

    return NewArg;
}

CWinCmdLine::CWinCmdArg CWinCmdLine::CWinCmdArg::MakeOption(LPCWSTR lpszOptName, int nArgIndex)
{
    assert(lpszOptName && *lpszOptName);

    CWinCmdArg NewArg;
    NewArg.m_Type       = CWinCmdArg::em_Option;
    NewArg.m_strKey     = lpszOptName;
    NewArg.m_nArgIndex  = nArgIndex;

    return NewArg;
}

CWinCmdLine::CWinCmdArg CWinCmdLine::CWinCmdArg::MakeOptionWithValue(LPCWSTR lpszOptName, LPCWSTR lpszValue, DWORD dwFlag, int nArgIndex)
{
    assert(lpszOptName && *lpszOptName);

    CWinCmdArg NewArg;
    NewArg.m_Type        = CWinCmdArg::em_OptionWithValue;
    NewArg.m_strKey      = lpszOptName;
    NewArg.m_strRawValue = lpszValue;
    NewArg.m_strValue    = lpszValue;
    NewArg.m_nArgIndex   = nArgIndex;

    if (!(dwFlag & WINMOD_CMD_ARG_FLAG__DISABLE_QUOTE))
    {
        QuoteStringIfSpace(NewArg.m_strRawValue);
    }

    return NewArg;
}






//////////////////////////////////////////////////////////////////////////
BOOL CWinCmdLine::Analyze()
{
    CString strCmdLine = GetCommandLine();
    return Analyze(strCmdLine);
}

BOOL CWinCmdLine::Analyze(CString strCmdLine)
{
    Reset();
    strCmdLine.Trim();

    size_t  nArgIndex   = 0;
    size_t  nParamIndex = 0;
    LPCWSTR lpszAnalyze = strCmdLine;
    while (*lpszAnalyze)
    {
        lpszAnalyze = TrimSpace(lpszAnalyze);


        LPCWSTR lpszNext = ::StrPBrk(lpszAnalyze, L" \"");
        if (!lpszNext)
            lpszNext = lpszAnalyze + wcslen(lpszAnalyze);

        if (L'"' == *lpszNext)
        {
            LPCWSTR lpszQuoteRight = ::StrChr(lpszNext + 1, L'"');
            if (!lpszQuoteRight || *lpszQuoteRight != L'"')
                return FALSE;   // quote is not closed

            lpszNext = lpszQuoteRight + 1;
        }

        CString strRawArg;
        strRawArg.SetString(lpszAnalyze, lpszNext - lpszAnalyze);


        // ignore text after quoted substring
        lpszAnalyze = lpszNext;
        lpszNext    = ::StrChr(lpszNext, L' ');
        lpszAnalyze = lpszNext ? lpszNext : lpszAnalyze + wcslen(lpszAnalyze);



        CWinCmdArg NewArg;
        if (!NewArg.Analyze(strRawArg) || NewArg.IsNull())
            return FALSE;


        // save argument
        NewArg.m_nArgIndex = m_RawArgArray.Add(NewArg);
        if (NewArg.IsParam())
        {
            NewArg.m_nParamIndex = m_ParamMap.Add(NewArg.m_nArgIndex);
        }
        else if (NewArg.IsOption())
        {
            m_OptionMap.SetAt(NewArg.GetKey(), NewArg.m_nArgIndex);
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}

CString CWinCmdLine::ToCmdLine(WCHAR chOptHeader, WCHAR chOptValueHeader)
{
    CString strCmdLine;
    for (size_t i = 0; i < m_RawArgArray.GetCount(); ++i)
    {
        CWinCmdArg& rArg = m_RawArgArray[i];
        if (rArg.IsNull())
        {
            continue;
        }
        else if (rArg.IsParam())
        {
            strCmdLine.Append(rArg.ToRaw());
            strCmdLine.AppendChar(' ');
        }
        else if (rArg.IsOption())
        {
            strCmdLine.AppendChar(chOptHeader);
            strCmdLine.Append(rArg.GetKey());
            if (rArg.HasOptionValue())
            {
                strCmdLine.AppendChar(chOptValueHeader);
                strCmdLine.Append(rArg.ToRaw());
            }
            strCmdLine.AppendChar(' ');
        }
    }

    strCmdLine.TrimRight();
    return strCmdLine;
}

void CWinCmdLine::Reset()
{
    m_RawArgArray.RemoveAll();
    m_OptionMap.RemoveAll();
    m_ParamMap.RemoveAll();
}

const CWinCmdLine::CWinCmdArg CWinCmdLine::GetOption(LPCWSTR lpszOptName)
{
    assert(lpszOptName && *lpszOptName);
    if (!lpszOptName || !*lpszOptName)
        return CWinCmdArg();

    size_t nArgIndex = 0;
    if (!m_OptionMap.Lookup(lpszOptName, nArgIndex))
        return CWinCmdArg();

    return GetArgument(nArgIndex);
}

BOOL CWinCmdLine::HasOption(LPCWSTR lpszOptName)
{
    assert(lpszOptName && *lpszOptName);
    if (!lpszOptName || !*lpszOptName)
        return FALSE;

    CWinCmdArg Arg = GetOption(lpszOptName);
    return !Arg.IsNull();
}

BOOL CWinCmdLine::SetOption(LPCWSTR lpszOptName)
{
    assert(lpszOptName && *lpszOptName);
    if (!lpszOptName || !*lpszOptName)
        return FALSE;

    size_t nArgIndex = 0;
    if (m_OptionMap.Lookup(lpszOptName, nArgIndex))
    {
        assert(nArgIndex >= 0);
        assert(nArgIndex < m_RawArgArray.GetCount());
        m_RawArgArray[nArgIndex] = CWinCmdArg::MakeOption(lpszOptName, nArgIndex);
        return TRUE;
    }

    nArgIndex = m_RawArgArray.Add(CWinCmdArg::MakeOption(lpszOptName));
    m_OptionMap.SetAt(lpszOptName, nArgIndex);
    return TRUE;
}

BOOL CWinCmdLine::SetOption(LPCWSTR lpszOptName, int nValue)
{
    CString strValue;
    strValue.Format(L"%d", nValue);

    return SetOption(lpszOptName, strValue);
}

BOOL CWinCmdLine::SetOption(LPCWSTR lpszOptName, INT64 nValue)
{
    CString strValue;
    strValue.Format(L"%I64d", nValue);

    return SetOption(lpszOptName, strValue);
}

BOOL CWinCmdLine::SetOption(LPCWSTR lpszOptName, LPCWSTR lpszOptValue, DWORD dwFlag)
{
    assert(lpszOptName && *lpszOptName);
    if (!lpszOptName || !*lpszOptName)
        return FALSE;

    size_t nArgIndex = 0;
    if (m_OptionMap.Lookup(lpszOptName, nArgIndex))
    {
        assert(nArgIndex >= 0);
        assert(nArgIndex < m_RawArgArray.GetCount());
        m_RawArgArray[nArgIndex] = CWinCmdArg::MakeOptionWithValue(lpszOptName, lpszOptValue, dwFlag, nArgIndex);
        return TRUE;
    }

    nArgIndex = m_RawArgArray.Add(CWinCmdArg::MakeOptionWithValue(lpszOptName, lpszOptValue, dwFlag, nArgIndex));
    m_OptionMap.SetAt(lpszOptName, nArgIndex);
    return TRUE;
}

void CWinCmdLine::RemoveOption(LPCWSTR lpszOptName)
{
    assert(lpszOptName && *lpszOptName);
    if (!lpszOptName || !*lpszOptName)
        return;

    size_t nArgIndex = 0;
    if (m_OptionMap.Lookup(lpszOptName, nArgIndex))
    {
        m_OptionMap.RemoveKey(lpszOptName);

        assert(nArgIndex >= 0);
        assert(nArgIndex < m_RawArgArray.GetCount());
        m_RawArgArray[nArgIndex].m_Type = CWinCmdArg::em_Null;
    }
}

void CWinCmdLine::RemoveAllOptions()
{
    m_OptionMap.RemoveAll();

    for (size_t i = 0; i < m_RawArgArray.GetCount(); ++i)
    {
        if (m_RawArgArray[i].IsOption())
            m_RawArgArray[i].m_Type = CWinCmdArg::em_Null;
    }
}










//////////////////////////////////////////////////////////////////////////
const CWinCmdLine::CWinCmdArg CWinCmdLine::GetParam(size_t nParamIndex)
{
    if (nParamIndex < 0 || nParamIndex >= m_ParamMap.GetCount())
        return CWinCmdArg();

    size_t nArgIndex = m_ParamMap[nParamIndex];
    return GetArgument(nArgIndex);
}

size_t CWinCmdLine::GetParamCount()
{
    return m_ParamMap.GetCount();
}

BOOL CWinCmdLine::HasParam(size_t nParamIndex)
{
    if (nParamIndex < 0 || nParamIndex >= m_ParamMap.GetCount())
        return FALSE;

    return TRUE;
}

BOOL CWinCmdLine::AddParam(LPCWSTR lpszParam, DWORD dwFlag)
{
    size_t nArgIndex   = m_RawArgArray.Add(CWinCmdArg::MakeParam(lpszParam, dwFlag));
    size_t nParamIndex = m_ParamMap.Add(nArgIndex);
    
    m_RawArgArray[nArgIndex].m_nArgIndex    = nArgIndex;
    m_RawArgArray[nArgIndex].m_nParamIndex  = nParamIndex;
    return TRUE;
}

BOOL CWinCmdLine::SetParam(size_t nParamIndex, LPCWSTR lpszParam, DWORD dwFlag)
{
    if (nParamIndex < 0)
        return FALSE;

    if (nParamIndex == m_ParamMap.GetCount())
        return AddParam(lpszParam, dwFlag);

    if (nParamIndex > m_ParamMap.GetCount())
        return FALSE;

    size_t nArgIndex = m_ParamMap[nParamIndex];
    assert(nArgIndex >= 0);
    assert(nArgIndex < m_RawArgArray.GetCount());
    m_RawArgArray[nArgIndex] = CWinCmdArg::MakeParam(lpszParam, dwFlag, nArgIndex, nParamIndex);
    return TRUE;
}

BOOL CWinCmdLine::SetParam(size_t nParamIndex, int nValue, DWORD dwFlag)
{
    CString strValue;
    strValue.Format(L"%d", nValue);

    return SetParam(nParamIndex, strValue, dwFlag);
}

BOOL CWinCmdLine::SetParam(size_t nParamIndex, INT64 nValue, DWORD dwFlag)
{
    CString strValue;
    strValue.Format(L"%I64d", nValue);

    return SetParam(nParamIndex, strValue, dwFlag);
}

void CWinCmdLine::RemoveParam(size_t nParamIndex)
{
    if (nParamIndex < 0 || nParamIndex >= m_ParamMap.GetCount())
        return;

    size_t nArgIndex = m_ParamMap[nParamIndex];
    assert(nArgIndex >= 0);
    assert(nArgIndex < m_RawArgArray.GetCount());

    m_RawArgArray[nArgIndex].m_Type = CWinCmdArg::em_Null;
    m_ParamMap.RemoveAt(nParamIndex);
    for (size_t i = nParamIndex; i < m_ParamMap.GetCount(); ++i)
    {
        assert(nParamIndex >= 0);
        assert(nParamIndex < m_ParamMap.GetCount());

        nArgIndex = m_ParamMap[i];
        assert(m_RawArgArray[nArgIndex].IsParam());
        m_RawArgArray[nArgIndex].m_nParamIndex--;
    }
}

void CWinCmdLine::RemoveAllParams()
{
    m_OptionMap.RemoveAll();

    for (size_t i = 0; i < m_RawArgArray.GetCount(); ++i)
    {
        if (m_RawArgArray[i].IsParam())
            m_RawArgArray[i].m_Type = CWinCmdArg::em_Null;
    }
}











//////////////////////////////////////////////////////////////////////////
void CWinCmdLine::QuoteStringIfSpace(CString& strText)
{
    if (-1 == strText.Find(L' ', 0))
        return;

    if (strText.GetLength() >= 2  &&
        L'"' == strText[0]        &&
        L'"' == strText[strText.GetLength() - 1])
    {
        return;
    }

    strText.Insert(0, L'"');
    strText.AppendChar(L'"');
}

int CWinCmdLine::UnquoteString(CString& strQuoted)
{
    int nQuotedLevel = 0; 
    while (
        strQuoted.GetLength() >= 2  &&
        L'"' == strQuoted[0]        &&
        L'"' == strQuoted[strQuoted.GetLength() - 1])
    {
        ++nQuotedLevel;
        strQuoted = strQuoted.Mid(1, strQuoted.GetLength() - 2);
    }

    return nQuotedLevel;
}

LPCWSTR CWinCmdLine::TrimSpace(LPCWSTR lpszText)
{
    while (*lpszText && iswspace(*lpszText))
    {
        ++lpszText;
    }

    return lpszText;
}

const CWinCmdLine::CWinCmdArg CWinCmdLine::GetArgument(size_t nArgIndex)
{
    assert(nArgIndex >= 0);
    assert(nArgIndex < m_RawArgArray.GetCount());
    if (nArgIndex < 0 || nArgIndex >= m_RawArgArray.GetCount())
        return CWinCmdArg();

    return m_RawArgArray[nArgIndex];
}