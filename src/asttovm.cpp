/*

  Description:  Convert the AST from the parser
                to stack-based instructions of
                the VM

  Author: Niels A. Moseley (c) 2016

*/

#include <QDebug>
#include "asttovm.h"

bool ASTToVM::process(const ParseContext &s,
                      VM::program_t &program,
                      VM::variables_t &variables)
{
    program.clear();
    variables.clear();

    // copy all the variables to the VM
    variables = s.m_variables;

    // convert all the statements to VM code
    auto iter2 = s.getStatements().begin();
    while(iter2 != s.getStatements().end())
    {
        if (!convertNode(*iter2, program, variables))
            return false;
        iter2++;
    }
    return true;
}

bool ASTToVM::convertNode(ASTNode *node,
                          VM::program_t &program,
                          VM::variables_t &variables)
{
    if (node == 0)
        return true;

    if (node->left != 0)
    {
        convertNode(node->left, program, variables);
    }

    if (node->right != 0)
    {
        convertNode(node->right, program, variables);
    }

    // push arguments of functions here!
    uint32_t nargs = node->m_functionArgs.size();
    for(uint32_t i=0; i<nargs; i++)
    {
        convertNode(node->m_functionArgs[i], program, variables);
    }

    VM::instruction_t instr;
    switch(node->m_type)
    {
    default:
    case ASTNode::NodeUnknown:
        return false;

    // ************************************************
    //  operations that only push things on the stack
    // ************************************************
    case ASTNode::NodeFloat:    // literal float
        instr.icode = P_literal;
        program.push_back(instr);
        instr.value = node->m_literalFloat;
        program.push_back(instr);
        return true;
    case ASTNode::NodeInteger:
        // Literal integer, create an integer on the stack
        instr.icode = P_literal;
        program.push_back(instr);
        instr.value = node->m_literalInt;
        program.push_back(instr);
        return true;
    case ASTNode::NodeIdent:
        // Push the value of the variable on the stack
        if (node->m_varIdx < 0)
        {
            // error! cannot find variable
            return false;
        }
        instr.icode = P_readvar | node->m_varIdx;
        program.push_back(instr);
        return true;
    case ASTNode::NodeDelayDefinition:
        // do nothing, the variable as already been added
        // to the VM
        return true;
    // *************************************************
    //  operations that pop and push stuff on the stack
    // *************************************************
    case ASTNode::NodeAssign:
    {
        // push the ID of the variable on the stack
        if (node->m_varIdx < 0)
        {
            // error! cannot find variable
            return false;
        }
        instr.icode = P_writevar | node->m_varIdx;
        program.push_back(instr);
        return true;
    }
    case ASTNode::NodeDelayAssign:
    {
        // push the ID of the delay onto the stack
        if (node->m_varIdx < 0)
        {
            // error! cannot find variable
            return false;
        }
        instr.icode = P_writedelay | node->m_varIdx;
        program.push_back(instr);
        return true;
    }
    case ASTNode::NodeDelayLookup:
    {
        // push the ID of the delay onto the stack
        if (node->m_varIdx < 0)
        {
            // error! cannot find variable
            return false;
        }
        instr.icode = P_readdelay | node->m_varIdx;
        program.push_back(instr);
        return true;
    }
    case ASTNode::NodeAdd:
    {
        instr.icode = P_add;
        program.push_back(instr);
        return true;
    }
    case ASTNode::NodeSub:
    {
        instr.icode = P_sub;
        program.push_back(instr);
        return true;
    }
    case ASTNode::NodeMul:
    {
        instr.icode = P_mul;
        program.push_back(instr);
        return true;
    }
    case ASTNode::NodeDiv:
    {
        instr.icode = P_div;
        program.push_back(instr);
        return true;
    }
    case ASTNode::NodeUnaryMinus:
    {
        instr.icode = P_neg;
        program.push_back(instr);
        return true;
    }
    case ASTNode::NodeFunction:
    {
        instr.icode = node->m_functionID;
        program.push_back(instr);
        return true;
    }
    } // end switch
    return true;
}
