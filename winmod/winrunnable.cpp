/**
* @file    winrunnable.cpp
* @brief   ...
* @author  bbcallen
* @date    2009-03-27  18:08
*/

#include "stdafx.h"
#include "winrunnable.h"

using namespace WinMod;

AWinRunnable::~AWinRunnable()
{
    ForceStopRunning(0);
}
