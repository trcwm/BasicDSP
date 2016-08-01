/*

  Description:  Convert the AST from the parser
                to stack-based instructions of
                the VM

  Author: Niels A. Moseley (c) 2016

*/

#ifndef asttovm_h
#define asttovm_h

#include "virtualmachine.h"
#include "parser.h"

class ASTToVM
{
public:
    static bool process(const statements_t &s, VM::program_t &program, VM::variables_t &variables);

protected:
    static bool convertNode(const ASTNodePtr node, VM::program_t &program, VM::variables_t &variables);
};

#endif
