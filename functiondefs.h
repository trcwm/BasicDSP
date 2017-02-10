/*

  BasicDSP function definitions for lexer and parser

  Copyright: Niels A. Moseley 2016,2017

*/

#ifndef functiondefs_h
#define functiondefs_h

#include <string>
#include <stdint.h>

struct functionInfo_t
{
    std::string   name;   // function name
    uint32_t      ID;     // function ID
    uint32_t      nargs;  // expected number of arguments
};

#define  g_functionDefsLen 14
extern const functionInfo_t g_functionDefs[];

namespace functionDefs
{
    int32_t getNumberOfArguments(uint32_t functionID);
}

#endif
