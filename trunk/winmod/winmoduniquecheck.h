/**
* @file    winmoduniquecheck.h
* @brief   ...
* @author  bbcallen
* @date    2011-03-08 14:44
*/

#ifndef WINMODUNIQUECHECK_H
#define WINMODUNIQUECHECK_H

#include "winmodbase.h"

NS_WINMOD_BEGIN

class CWinModUniqueCheck: public ATL::CHandle
{
public:
    // lpszMutexString务必加上"Global\\"前缀
    BOOL CheckUnique(LPCWSTR lpszMutexString)
    {
        Attach(::CreateMutex(NULL, TRUE, lpszMutexString));
        if(!m_h || ::GetLastError() == ERROR_ALREADY_EXISTS)
        {
            Close();
            return FALSE;
        }	

        return TRUE;
    }
};

NS_WINMOD_END

#endif//WINMODUNIQUECHECK_H
