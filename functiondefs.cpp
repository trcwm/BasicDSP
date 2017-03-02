/*

  BasicDSP function definitions for lexer and parser

  Copyright: Niels A. Moseley 2016,2017

*/

#include "virtualmachine.h"
#include "functiondefs.h"

const functionInfo_t g_functionDefs[] =
{
    {"sin",P_sin,1},
    {"cos",P_cos,1},
    {"sin1",P_sin1,1},
    {"cos1",P_cos1,1},
    {"mod1",P_mod1,1},
    {"abs",P_abs,1},
    {"round",P_round,1},
    {"sqrt",P_sqrt,1},
    {"tan",P_tan,1},
    {"tanh",P_tanh,1},
    {"pow",P_pow,2},
    {"limit",P_limit,1},
    {"atan2",P_atan2,2},
    {"sign",P_sign,1},
    {"noise",P_noise,0},
    {"trunc",P_trunc,1},
    {"ceil",P_ceil,1},
    {"floor",P_floor,1}
};

int32_t functionDefs::getNumberOfArguments(uint32_t functionID)
{
    uint32_t i=0;
    while(i<g_functionDefsLen)
    {
        if (g_functionDefs[i].ID == functionID)
            return g_functionDefs[i].nargs;
        i++;
    }
    return -1;
}
