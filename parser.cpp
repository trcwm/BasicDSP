/*

  Description:  The parser takes a list of tokens,
                checks the grammar and produces
                a parse tree.

  Author: Niels A. Moseley (c) 2016

*/

#include <iostream>
#include "parser.h"

Parser::Parser() : m_tokens(NULL)
{
}

void Parser::error(const state_t &s, const std::string &txt)
{
    m_lastError = txt;
    m_lastErrorPos = s.tokPos;
    std::cout << txt.c_str() << std::endl;
}

void Parser::error(uint32_t dummy, const std::string &txt)
{
    (dummy);
    m_lastError = txt;
    std::cout << txt.c_str() << std::endl;
}

bool Parser::match(state_t &s, uint32_t tokenID)
{
    token_t tok = getToken(s);
    if (tok.tokID != tokenID)
    {
        return false;
    }
    next(s);
    return true;
}

bool Parser::matchList(state_t &s, const uint32_t *tokenIDlist)
{
    while (*tokenIDlist != NULL)
    {
        if (!match(s, *tokenIDlist++))
            return false;
    }
    return true;
}

bool Parser::process(const std::vector<token_t> &tokens, statements_t &result)
{
    m_lastError.clear();

    if (tokens.size() == 0)
    {
        error(0,"Internal error: token list is empty");
        return false;
    }

    // prepare for iteration
    m_tokens = &tokens;

    state_t state;
    state.tokIdx = 0;

    return acceptProgram(state, result);
}

bool Parser::acceptProgram(state_t &s, statements_t &statements)
{
    // productions: assignment | NEWLINE | SEMICOL | EOF

    bool productionAccepted = true;
    token_t tok = getToken(s);


    while(productionAccepted == true)
    {
        productionAccepted = false;
        ASTNodePtr newNode(new ASTNode());

        if (acceptAssignment(s, newNode))
        {
            productionAccepted = true;
            statements.push_back(newNode);
        }
        else if (match(s, TOK_NEWLINE))
        {
            productionAccepted = true;
            // do nothing..
        }
        else if (match(s, TOK_SEMICOL))
        {
            productionAccepted = true;
            // do nothing..
        }
        else if (match(s, TOK_EOF))
        {
            return true;
        }
    }
    return false;
}

bool Parser::acceptAssignment(state_t &s, ASTNodePtr newNode)
{
    // production: IDENT EQUAL expr SEMICOL
    state_t savestate = s;

    if (!match(s,TOK_IDENT))
    {
        s = savestate;
        return false;
    }

    if (!match(s,TOK_EQUAL))
    {
        error(s,"Expected '='");
        s = savestate;
        return false;
    }

    std::string identifier = getToken(s, -2).txt;

    ASTNodePtr exprNode(new ASTNode());

    if (!acceptExpr(s, exprNode))
    {
        error(s,"Expression expected");
        s = savestate;
        return false;
    }

    /*
    if (!match(s, TOK_SEMICOL))
    {
        error(s,"Assignments must end with a semicolon.");
        s = savestate;
        return false;
    }
    */

    newNode->type = ASTNode::NodeAssign;
    newNode->info.txt  = identifier;
    newNode->right = exprNode;

    return true;
}

bool Parser::acceptExpr(state_t &s, ASTNodePtr newNode)
{
    // productions: term + expr | term - expr | term
    state_t savestate = s;

    if (acceptExpr1(s, newNode))
    {
         return true;
    }

    s = savestate;
    if (acceptExpr2(s, newNode))
    {
        return true;
    }

    s = savestate;
    if (acceptTerm(s, newNode))
    {
        return true;
    }

    //error("Error parsing expression");
    return false;
}

bool Parser::acceptExpr1(state_t &s, ASTNodePtr newNode)
{
    // production: term + expr
    state_t savestate = s;

    ASTNodePtr termNode(new ASTNode());
    ASTNodePtr exprNode(new ASTNode());

    if (!acceptTerm(s, termNode))
    {
        s = savestate;
        return false;
    }
    if (!match(s, TOK_PLUS))
    {
        s = savestate;
        return false;
    }
    if (!acceptExpr(s, exprNode))
    {
        s = savestate;
        return false;
    }
    newNode->type = ASTNode::NodeAdd;
    newNode->left = termNode;
    newNode->right = exprNode;
    return true;
}

bool Parser::acceptExpr2(state_t &s, ASTNodePtr newNode)
{
    // production: term - expr
    state_t savestate = s;

    ASTNodePtr termNode(new ASTNode());
    ASTNodePtr exprNode(new ASTNode());

    if (!acceptTerm(s, termNode))
    {
        s = savestate;
        return false;
    }
    if (!match(s, TOK_MINUS))
    {
        s = savestate;
        return false;
    }
    if (!acceptExpr(s, exprNode))
    {
        s = savestate;
        return false;
    }
    newNode->type = ASTNode::NodeSub;
    newNode->left = termNode;
    newNode->right = exprNode;
    return true;
}

bool Parser::acceptTerm(state_t &s, ASTNodePtr newNode)
{
    // production: term1 | term2 | term3 | factor
    state_t savestate = s;
    if (acceptTerm1(s, newNode))
    {
        return true;
    }

    s = savestate;
    if (acceptTerm2(s, newNode))
    {
        return true;
    }

    s = savestate;
    if (acceptTerm3(s, newNode))
    {
        return true;
    }

    s = savestate;
    if (acceptFactor(s, newNode))
    {
        return true;
    }
    return false;
}

bool Parser::acceptTerm1(state_t &s, ASTNodePtr newNode)
{
    // MINUS factor
    state_t savestate = s;
    if (!match(s, TOK_MINUS))
    {
        s = savestate;
        return false;
    }
    ASTNodePtr factorNode(new ASTNode());
    if (!acceptFactor(s, factorNode))
    {
        s = savestate;
        return false;
    }

    newNode->type = ASTNode::NodeUnaryMinus;
    newNode->right = factorNode;
    return true;
}

bool Parser::acceptTerm2(state_t &s, ASTNodePtr newNode)
{
    // production: factor * term
    ASTNodePtr factorNode(new ASTNode());
    ASTNodePtr termNode(new ASTNode());

    state_t savestate = s;
    if (!acceptFactor(s, factorNode))
    {
        s = savestate;
        return false;
    }
    if (!match(s, TOK_STAR))
    {
        s = savestate;
        return false;
    }
    if (!acceptTerm(s, termNode))
    {
        s = savestate;
        return false;
    }
    newNode->type = ASTNode::NodeMul;
    newNode->left = factorNode;
    newNode->right = termNode;
    return true;
}

bool Parser::acceptTerm3(state_t &s, ASTNodePtr newNode)
{
    // production: factor / term
    ASTNodePtr factorNode(new ASTNode());
    ASTNodePtr termNode(new ASTNode());

    state_t savestate = s;
    if (!acceptFactor(s, factorNode))
    {
        s = savestate;
        return false;
    }
    if (!match(s, TOK_SLASH))
    {
        s = savestate;
        return false;
    }
    if (!acceptTerm(s, termNode))
    {
        s = savestate;
        return false;
    }
    newNode->type = ASTNode::NodeDiv;
    newNode->left = factorNode;
    newNode->right = termNode;
    return true;
}

bool Parser::acceptFactor(state_t &s, ASTNodePtr newNode)
{
    state_t savestate = s;

    // FUNCTION ( expr )
    if (acceptFactor1(s, newNode))
    {
        return true;
    }

    s = savestate;
    // ( expr )
    if (acceptFactor2(s, newNode))
    {
        return true;
    }

    s = savestate;
    if (match(s, TOK_INTEGER))
    {
        newNode->type = ASTNode::NodeInteger;
        newNode->info.intVal = atoi(getToken(s, -1).txt.c_str());
        return true;    // INTEGER
    }

    if (match(s, TOK_FLOAT))
    {
        newNode->type = ASTNode::NodeFloat;
        newNode->info.floatVal = atof(getToken(s, -1).txt.c_str());
        return true;    // FLOAT
    }

    if (match(s, TOK_IDENT))
    {
        newNode->type = ASTNode::NodeIdent;
        newNode->info.txt = getToken(s, -1).txt;
        return true;    // IDENT
    }

    error(s, "Factor is not an integer, float, identifier or parenthesised expression.");
    return false;
}

bool Parser::acceptFactor1(state_t &s, ASTNodePtr newNode)
{
    ASTNodePtr argNode(new ASTNode());

    state_t savestate = s;
    token_t func = getToken(s);
    if (func.tokID < 100)
    {
        s = savestate;
        return false;
    }
    next(s);

    if (!match(s, TOK_LPAREN))
    {
        s = savestate;
        return false;
    }
    // TODO:
    // FIXME:
    // add functions with more than one argument
    if (!acceptExpr(s, argNode))
    {
        s = savestate;
        return false;
    }
    if (!match(s, TOK_RPAREN))
    {
        s = savestate;
        return false;
    }

    // lookup the function in the function list


    newNode->type = ASTNode::NodeFunction;
    newNode->info.txt = func.txt;
    newNode->functionID = func.tokID;
    newNode->left = 0;
    newNode->right = argNode;
    return true;
}


bool Parser::acceptFactor2(state_t &s, ASTNodePtr newNode)
{
    state_t savestate = s;
    if (!match(s, TOK_LPAREN))
    {
        s = savestate;
        return false;
    }
    if (!acceptExpr(s, newNode))
    {
        s = savestate;
        return false;
    }
    if (!match(s, TOK_RPAREN))
    {
        s = savestate;
        return false;
    }
    return true;
}
