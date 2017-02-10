/*

  Description:  The parser takes a list of tokens,
                checks the grammar and produces
                a parse tree.

  Author: Niels A. Moseley (c) 2016,2017

*/

#include <iostream>
#include "functiondefs.h"
#include "parser.h"

Parser::Parser() : m_tokens(NULL)
{
    m_lastErrorPos.line=0;
    m_lastErrorPos.offset=0;
    m_lastErrorPos.pos=0;
    m_lastError = std::string("Unknown error");
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
    while (*tokenIDlist != 0)
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

        ASTNode *node = 0;
        if ((node=acceptAssignment(s)) != 0)
        {
            productionAccepted = true;
            statements.push_back(node);
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

ASTNode* Parser::acceptAssignment(state_t &s)
{
    // production: IDENT EQUAL expr SEMICOL
    state_t savestate = s;

    if (!match(s,TOK_IDENT))
    {
        s = savestate;
        return NULL;
    }

    if (!match(s,TOK_EQUAL))
    {
        error(s,"Expected '='");
        s = savestate;
        return NULL;
    }

    std::string identifier = getToken(s, -2).txt;

    ASTNode *exprNode = 0;
    if ((exprNode=acceptExpr(s)) == 0)
    {
        error(s,"Expression expected");
        s = savestate;
        return NULL;
    }

    /* we've match an assignment node! */

    ASTNode *assignNode = new ASTNode(ASTNode::NodeAssign);
    assignNode->info.txt = identifier;
    assignNode->right = exprNode;

    return assignNode;
}

ASTNode* Parser::acceptExpr(state_t &s)
{
    // productions: term expr'
    //
    // the term is the first term
    // and must therefore be
    // added as the left leaf
    //

    state_t savestate = s;

    ASTNode *leftNode = 0;
    if ((leftNode=acceptTerm(s)) != NULL)
    {
        // the term is the left-hand size of the expr'
        // the right hand side and the operation node
        // itself still need to be found.
        //
        // note, exprAccentNode is never NULL
        // because of it's epsilon solution
        //
        ASTNode *exprAccentNode = acceptExprAccent(s, leftNode);
        return exprAccentNode;
    }
    s = savestate;
    return NULL;
}

ASTNode* Parser::acceptExprAccent(state_t &s, ASTNode *leftNode)
{
    // production: - term expr' | + term expr' | epsilon
    //
    // we already have the left-hand side of the
    // addition or subtraction.
    //
    // if we encounter the epsilon,
    // the resulting node is just
    // the leftNode, which was already
    // matched
    //

    state_t savestate = s;

    ASTNode *topNode = 0;
    if ((topNode = acceptExprAccent1(s, leftNode)) != 0)
    {
        return topNode;
    }

    s = savestate;
    if ((topNode = acceptExprAccent2(s, leftNode)) != 0)
    {
        return topNode;
    }

    // if nothing matched, that's ok
    // because we have an epsilon
    // solution
    s = savestate;
    return leftNode;
}

ASTNode* Parser::acceptExprAccent1(state_t &s, ASTNode *leftNode)
{
    // production: - term expr'
    state_t savestate = s;

    if (!match(s, TOK_MINUS))
    {
        return NULL;
    }

    ASTNode *rightNode = 0;
    if ((rightNode=acceptTerm(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // create a new 'head' node here
    // with the right leaf being
    // the term that was just found
    // and the left leaf the argument node
    //
    // supply the new head node to the next
    // acceptExprAccent function

    ASTNode *operationNode = new ASTNode(ASTNode::NodeSub);
    operationNode->left = leftNode;
    operationNode->right = rightNode;

    // note: acceptExprAccent will never return NULL
    ASTNode *headNode = acceptExprAccent(s, operationNode);
    return headNode;
}

ASTNode* Parser::acceptExprAccent2(state_t &s, ASTNode *leftNode)
{
    // production: + term expr'
    state_t savestate = s;

    if (!match(s, TOK_PLUS))
    {
        return NULL;
    }

    ASTNode *rightNode = 0;
    if ((rightNode=acceptTerm(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // create a new 'head' node here
    // with the right leaf being
    // the term that was just found
    // and the left leaf the argument node
    //
    // supply the new head node to the next
    // acceptExprAccent function

    ASTNode *operationNode = new ASTNode(ASTNode::NodeAdd);
    operationNode->left = leftNode;
    operationNode->right = rightNode;

    // note: acceptExprAccent will never return NULL
    ASTNode *headNode = acceptExprAccent(s, operationNode);
    return headNode;
}

ASTNode* Parser::acceptTerm(state_t &s)
{
    // production: factor term'
    state_t savestate = s;

    ASTNode *leftNode = 0;
    if ((leftNode=acceptFactor(s)) != NULL)
    {
        // the term is the left-hand size of the term'
        // the right hand side and the operation node
        // itself still need to be found.
        //
        // note, termAccentNode is never NULL
        // because of it's epsilon solution
        //
        ASTNode *termAccentNode = acceptTermAccent(s, leftNode);
        return termAccentNode;
    }
    s = savestate;
    return NULL;
}

ASTNode* Parser::acceptTermAccent(state_t &s, ASTNode *leftNode)
{
    // production: * factor term' | / factor term' | epsilon
    //
    // we already have the left-hand side of the
    // multiplication or division.
    //
    // if we encounter the epsilon,
    // the resulting node is just
    // the leftNode, which was already
    // matched
    //

    state_t savestate = s;

    ASTNode *topNode = 0;
    if ((topNode = acceptTermAccent1(s, leftNode)) != 0)
    {
        return topNode;
    }

    s = savestate;
    if ((topNode = acceptTermAccent2(s, leftNode)) != 0)
    {
        return topNode;
    }

    // if nothing matched, that's ok
    // because we have an epsilon
    // solution
    s = savestate;
    return leftNode;
}

ASTNode* Parser::acceptTermAccent1(state_t &s, ASTNode* leftNode)
{
    // production: * factor term'
    state_t savestate = s;

    if (!match(s, TOK_STAR))
    {
        return NULL;
    }

    ASTNode *rightNode = 0;
    if ((rightNode=acceptTerm(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // create a new 'head' node here
    // with the right leaf being
    // the term that was just found
    // and the left leaf the argument node
    //
    // supply the new head node to the next
    // acceptTermAccent function

    ASTNode *operationNode = new ASTNode(ASTNode::NodeMul);
    operationNode->left = leftNode;
    operationNode->right = rightNode;

    // note: acceptExprAccent will never return NULL
    ASTNode *headNode = acceptExprAccent(s, operationNode);
    return headNode;
}

ASTNode* Parser::acceptTermAccent2(state_t &s, ASTNode* leftNode)
{
    // production: / factor term'
    state_t savestate = s;

    if (!match(s, TOK_SLASH))
    {
        return NULL;
    }

    ASTNode *rightNode = 0;
    if ((rightNode=acceptTerm(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // create a new 'head' node here
    // with the right leaf being
    // the term that was just found
    // and the left leaf the argument node
    //
    // supply the new head node to the next
    // acceptTermAccent function

    ASTNode *operationNode = new ASTNode(ASTNode::NodeDiv);
    operationNode->left = leftNode;
    operationNode->right = rightNode;

    // note: acceptExprAccent will never return NULL
    ASTNode *headNode = acceptExprAccent(s, operationNode);
    return headNode;
}


ASTNode* Parser::acceptFactor(state_t &s)
{
    state_t savestate = s;

    // FUNCTION ( expr )
    ASTNode *factorNode = 0;
    if ((factorNode=acceptFactor1(s)) != NULL)
    {
        return factorNode;
    }

    s = savestate;
    // ( expr )
    if ((factorNode=acceptFactor2(s)) != NULL)
    {
        return factorNode;
    }

    s = savestate;
    // - factor
    if ((factorNode=acceptFactor3(s)) != NULL)
    {
        return factorNode;
    }

    s = savestate;
    if (match(s, TOK_INTEGER))
    {
        factorNode = new ASTNode(ASTNode::NodeInteger);
        factorNode->info.intVal = atoi(getToken(s, -1).txt.c_str());
        return factorNode;    // INTEGER
    }

    if (match(s, TOK_FLOAT))
    {
        factorNode = new ASTNode(ASTNode::NodeFloat);
        factorNode->info.floatVal = atof(getToken(s, -1).txt.c_str());
        return factorNode;    // FLOAT
    }

    if (match(s, TOK_IDENT))
    {
        factorNode = new ASTNode(ASTNode::NodeIdent);
        factorNode->info.txt = getToken(s, -1).txt;
        return factorNode;    // IDENT
    }

    error(s, "Factor is not an integer, float, identifier or parenthesised expression.");
    return NULL;
}

ASTNode* Parser::acceptFactor1(state_t &s)
{
    // production: FUNCTION ( expr )

    state_t savestate = s;
    token_t func = getToken(s);
    if (func.tokID < 100)
    {
        s = savestate;
        return NULL;
    }
    next(s);

    if (!match(s, TOK_LPAREN))
    {
        s = savestate;
        return NULL;
    }

    // check how many argument this function expects

    uint32_t nargs = functionDefs::getNumberOfArguments(func.tokID);
    if (nargs == -1)
    {
        // function not found!
        // this is a severe error because the tokenizer recognized
        // the function but for some reason we can't find it
        // here anymore.. !?!
        error(s, "Internal parser error: cannot find function!");
        return NULL;
    }

    ASTNode* factorNode = new ASTNode(ASTNode::NodeFunction);
    factorNode->info.txt = func.txt;
    factorNode->functionID = func.tokID;
    factorNode->left = 0;
    factorNode->right = 0;

    uint32_t argcnt = 0;
    while(argcnt < nargs)
    {
        ASTNode *exprNode = 0;
        if ((exprNode=acceptExpr(s)) == NULL)
        {
            error(s,"Invalid argument or number of arguments");
            s = savestate;
            delete factorNode;
            return NULL;
        }
        factorNode->function_args.push_back(exprNode);
        argcnt++;

        // if there are arguments left, we need to see a comma
        if (argcnt != nargs)
        {
            if (!match(s, TOK_COMMA))
            {
                error(s,"Expected a comma in arguments list");
                s = savestate;
                delete factorNode;
                return NULL;
            }
        }
    }

    if (!match(s, TOK_RPAREN))
    {
        delete factorNode;
        s = savestate;
        return NULL;
    }

    return factorNode;
}


ASTNode* Parser::acceptFactor2(state_t &s)
{
    state_t savestate = s;
    if (!match(s, TOK_LPAREN))
    {
        s = savestate;
        return NULL;
    }
    ASTNode *exprNode = 0;
    if ((exprNode=acceptExpr(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }
    if (!match(s, TOK_RPAREN))
    {
        delete exprNode;
        s = savestate;
        return NULL;
    }
    return exprNode;
}


ASTNode* Parser::acceptFactor3(state_t &s)
{
    // production: - factor
    state_t savestate = s;
    if (!match(s, TOK_MINUS))
    {
        s = savestate;
        return NULL;
    }
    ASTNode *factorNode = 0;
    if ((factorNode=acceptFactor(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // unary minus node
    ASTNode *exprNode = new ASTNode(ASTNode::NodeUnaryMinus);
    exprNode->left = 0;
    exprNode->right = factorNode;

    return exprNode;
}

