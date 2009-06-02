/**
* @file    winregkey.h
* @brief   ...
* @author  bbcallen
* @date    2009-04-25  15:07
*/

#ifndef WINREGKEY_H
#define WINREGKEY_H

#include <atlstr.h>
#include <atlbase.h>
#include "winmod\winmodbase.h"

NS_WINMOD_BEGIN

#define WINMOD_REASONABL_VALUE_LEN  65536

class CWinRegKey: public CRegKey
{
public:

    HRESULT ExQueryStringValue(LPCWSTR lpszValueName, CString& strValue, DWORD dwCchMaxLen = WINMOD_REASONABL_VALUE_LEN);

    HRESULT ExQueryExpendedStringValue(LPCWSTR lpszValueName, CString& strValue, DWORD dwCchMaxLen = WINMOD_REASONABL_VALUE_LEN);
};

NS_WINMOD_END

#endif//WINREGKEY_H