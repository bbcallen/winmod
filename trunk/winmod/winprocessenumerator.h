/**
* @file    winprocessenumerator.h
* @brief   ...
* @author  bbcallen
* @date    2009-02-25  11:27
*/

#ifndef WINPROCESSENUMERATOR_H
#define WINPROCESSENUMERATOR_H

#include <psapi.h>
#include <tlhelp32.h>

#include <atlbase.h>
#include <atlcoll.h>
#include <atlstr.h>
#include "winmod\winmodbase.h"
#include "winmod\psapimod.h"

NS_WINMOD_BEGIN

class CWinModuleEnumerator
{
public:
    HRESULT EnumAllModules(DWORD dwProcessID);
    BOOL    FindFirstModule();
    BOOL    FindNextModule();

    BOOL    EndOfModules();
    HMODULE GetModule();
    HRESULT GetModulePath(CString& strPath);

protected:
    void    Reset();

    CHandle             m_hProc;
    CAtlArray<HMODULE>  m_moduleList;
    size_t              m_moduleIndex;
};




class CWinProcessEnumerator
{
public:
    CWinProcessEnumerator();


    HRESULT EnumAllProcesses();
    BOOL    FindFirstProcess();
    BOOL    FindNextProcess();

    BOOL    EndOfProcesses();
    DWORD   GetProcessID();


    HRESULT GetProcessPath(CString& strProcessPath);

    HRESULT GetProcessImageName(CString& strProcessImageName);

    HRESULT GetProcessName(CString& strProcessName);


protected:
    void    Reset();

    CAtlArray<DWORD>    m_procList;
    size_t              m_procIndex;

    CWinModule_psapi    dll_psapi;
};



class CWinProcessToolHelpEnumerator
{
public:
    CWinProcessToolHelpEnumerator();

    BOOL    FindProcessID(DWORD dwProcessID);

    HRESULT EnumAllProcesses();
    BOOL    FindFirstProcess();
    BOOL    FindNextProcess();

    BOOL    EndOfProcesses();
    DWORD   GetProcessID();
    HRESULT GetProcessName(CString& strProcessName);

private:
    CHandle         m_hProcessSnap;
    PROCESSENTRY32  m_pe32;
    BOOL            m_bValieProcEntry;
};

NS_WINMOD_END

#endif//WINPROCESSENUMERATOR_H