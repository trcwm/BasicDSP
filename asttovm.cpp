/*

  Description:  Convert the AST from the parser
                to stack-based instructions of
                the VM

  Author: Niels A. Moseley (c) 2016

*/

#include <QDebug>
#include "asttovm.h"

bool ASTToVM::process(const statements_t &s,
                      VM::program_t &program,
                      VM::variables_t &variables)
{
    program.clear();
    variables.clear();

    size_t N = s.size();
    for(size_t i=0; i<N; i++)
    {
        if (!convertNode(s[i], program, variables))
            return false;
    }
    return true;
}

bool ASTToVM::convertNode(const ASTNodePtr node,
                          VM::program_t &program,
                          VM::variables_t &variables)
{
    if (node->left != 0)
    {
        convertNode(node->left, program, variables);
    }

    if (node->right != 0)
    {
        convertNode(node->right, program, variables);
    }

    VM::variable_t var;
    VM::instruction_t instr;
    int32_t idx;
    switch(node->type)
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
        instr.value = node->info.floatVal;
        program.push_back(instr);
        return true;
    case ASTNode::NodeInteger:
        // Literal integer, create an integer on the stack
        instr.icode = P_literal;
        program.push_back(instr);
        instr.value = node->info.intVal;
        program.push_back(instr);
        return true;
    case ASTNode::NodeIdent:
        // resolve the identifier by name
        idx = VM::findVariableByName(variables, node->info.txt);
        if (idx == -1)
        {
            // variable not found, so add it to the list
            var.name = node->info.txt;
            var.value = 0.0f;
            variables.push_back(var);
            idx = variables.size()-1; // set variable index
        }
        instr.icode = P_readvar | idx;
        program.push_back(instr);
        return true;
    // *************************************************
    //  operations that pop and push stuff on the stack
    // *************************************************
    case ASTNode::NodeAssign:
    {
        idx = VM::findVariableByName(variables, node->info.txt);
        if (idx == -1)
        {
            // variable not found, so add it to the list
            var.name = node->info.txt;
            var.value = 0.0f;
            variables.push_back(var);
            idx = variables.size()-1; // set variable index
        }

        instr.icode = P_writevar | idx;
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
        instr.icode = node->functionID;
        program.push_back(instr);
        return true;
    }
    } // end switch
    return true;
}
