/**
* @file    winregfind.cpp
* @brief   ...
* @author  bbcallen
* @date    2009-04-23  14:55
*/

#include "stdafx.h"
#include "winregfind.h"

static FILETIME m_ftZero = {0, 0};

using namespace WinMod;

void CWinRegFindDepthFirst::Reset()
{
    while (!m_findNodeStack.IsEmpty())
    {
        CWinRegKeyFind* pNode = m_findNodeStack.GetTail();
        if (pNode && pNode != &m_root)
            delete pNode;

        m_findNodeStack.RemoveTailNoReturn();
    }

    m_hKeyParent = NULL;
    m_root.Reset();
}

BOOL CWinRegFindDepthFirst::FindFirstKey(HKEY hKeyParent, LPCWSTR lpszKeyName, REGSAM samDesired)
{
    Reset();

    samDesired |= KEY_ENUMERATE_SUB_KEYS;
    BOOL bFind = m_root.FindFirstSubKey(hKeyParent, lpszKeyName, samDesired);
    if (!bFind)
    {
        Reset();
        return FALSE;
    }

    m_hKeyParent = hKeyParent;
    m_findNodeStack.AddTail(&m_root);
    return TRUE;
}

BOOL CWinRegFindDepthFirst::FindNextKey(REGSAM samDesired)
{
    while (!m_findNodeStack.IsEmpty())
    {
        CWinRegKeyFind* pTopNode = m_findNodeStack.GetTail();
        assert(pTopNode);


        //////////////////////////////////////////////////////////////////////////
        // 向下试探
        if (pTopNode->IsEndOfFind())
        {   // 当前结点已经完成遍历
            m_findNodeStack.RemoveTailNoReturn();
            continue;
        }


        CWinRegKeyFind* pSubNode = new CWinRegKeyFind();
        if (!pSubNode)
        {
            Reset();
            return FALSE;
        }


        // 优先查找当前结点的子结点
        samDesired |= KEY_ENUMERATE_SUB_KEYS;
        if (pSubNode->FindFirstSubKey(m_hKeyParent, pTopNode->GetFullKeyName(), samDesired))
        {
            m_findNodeStack.AddTail(pSubNode);
            return TRUE;
        }


        // 没有发现子结点,删除之
        delete pSubNode;
        pSubNode = NULL;



        //////////////////////////////////////////////////////////////////////////
        // 回溯,并查找路径上每个结点的兄弟结点
        while (!m_findNodeStack.IsEmpty())
        {
            pTopNode = m_findNodeStack.GetTail();
            if (pTopNode->FindNextSubKey())
            {
                return TRUE;
            }

            m_findNodeStack.RemoveTailNoReturn();
        }
    }


    Reset();
    return FALSE;
}
