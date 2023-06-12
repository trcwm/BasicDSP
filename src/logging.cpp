/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  logging system

  Author: Niels A. Moseley

*/

#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include "logging.h"

static bool g_debugEnabled = false;

void setDebugging(bool enabled)
{
    g_debugEnabled = enabled;
}

void doLog(logtype_t t, const char *format, ...)
{
    switch(t)
    {
    case LOG_INFO:
        std::cout << "INFO: ";
        break;
    case LOG_DEBUG:
        if (!g_debugEnabled) return;
        std::cout << "DEBUG: ";
        break;
    case LOG_WARN:
        std::cout << "WARNING: ";
        break;
    case LOG_ERROR:
        std::cout << "ERROR: ";
        break;
    default:
        break;
    }

    //FIXME: change to C++ style
    va_list argptr;
    va_start(argptr, format);
    vprintf(format, argptr);
    va_end(argptr);
}
