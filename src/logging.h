/*

  BasicDSP logging functions

  Description:  logging system

  Copyright: Niels A. Moseley 2016,2017

*/

#ifndef logging_h
#define logging_h

#include <string>

typedef enum {LOG_INFO = 1, LOG_DEBUG = 2, LOG_WARN = 4, LOG_ERROR = 8} logtype_t;

/** enable the debug output */
void setDebugging(bool enabled = true);

/** log something */
void doLog(logtype_t t, const char *format, ...);

#endif
