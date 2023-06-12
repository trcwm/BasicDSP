/*

  Description:  The parser takes a list of tokens,
                checks the grammar and produces
                a parse tree.

  Author: Niels A. Moseley (c) 2016,2017

*/

#include <iostream>
#include "functiondefs.h"
#include "parser.h"

// ********************************************************************************
//   parseContext
// ********************************************************************************

void ParseContext::addStatement(ASTNode *statement)
{
    m_statements.push_back(statement);
}

int32_t ParseContext::createVariable(const std::string &name, varInfo::type_t varType)
{
    int32_t checkIdx = getVariableByName(name);
    if (checkIdx == -1)
    {
        varInfo info;
        info.m_name = name;
        info.m_type = varType;
        m_variables.push_back(info);
        return (int32_t)(m_variables.size()-1);
    }
    else
    {
        return checkIdx;
    }
}

int32_t ParseContext::getVariableByName(const std::string &name)
{
    const int32_t N=(int32_t)m_variables.size();

    for(int32_t i=0; i<N; i++)
    {
        if (m_variables[i].m_name == name)
            return i;
    }
    return -1;
}


// ********************************************************************************
//   Parser
// ********************************************************************************

Parser::Parser() : m_tokens(NULL)
{
    m_lastErrorPos.line=0;
    m_lastErrorPos.offset=0;
    m_lastErrorPos.pos=0;
    m_lastError = std::string("Unknown error");
}

void Parser::error(const ParseContext &s, const std::string &txt)
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

bool Parser::match(ParseContext &s, uint32_t tokenID)
{
    token_t tok = getToken(s);
    if (tok.tokID != tokenID)
    {
        return false;
    }
    next(s);
    return true;
}

bool Parser::matchList(ParseContext &s, const uint32_t *tokenIDlist)
{
    while (*tokenIDlist != 0)
    {
        if (!match(s, *tokenIDlist++))
            return false;
    }
    return true;
}

bool Parser::process(const std::vector<token_t> &tokens, ParseContext &context)
{
    m_lastError.clear();

    if (tokens.size() == 0)
    {
        error(0,"Internal error: token list is empty");
        return false;
    }

    // prepare for iteration
    m_tokens = &tokens;

    context.tokIdx = 0;
    return acceptProgram(context);
}

bool Parser::acceptProgram(ParseContext &context)
{
    // productions: assignment | delaydefinition | NEWLINE | SEMICOL | EOF

    bool productionAccepted = true;
    token_t tok = getToken(context);

    while(productionAccepted == true)
    {
        productionAccepted = false;

        ASTNode *node = 0;
        if ((node=acceptAssignment(context)) != 0)
        {
            productionAccepted = true;
            context.addStatement(node);
        }
        else if ((node=acceptDelayDefinition(context)) != 0)
        {
            productionAccepted = true;
            context.addStatement(node);
        }
        else if (match(context, TOK_NEWLINE))
        {
            productionAccepted = true;
            // do nothing..
        }
        else if (match(context, TOK_SEMICOL))
        {
            productionAccepted = true;
            // do nothing..
        }
        else if (match(context, TOK_EOF))
        {
            return true;
        }
    }
    return false;
}


ASTNode* Parser::acceptAssignment(ParseContext &s)
{
    // production: IDENT EQUAL expr SEMICOL
    ParseContext savestate = s;

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
        //error(s,"Expression expected");
        s = savestate;
        return NULL;
    }

    /* we've match an assignment node! */
    int32_t varIdx = s.createVariable(identifier);
    ASTNode *assignNode = 0;
    if (s.m_variables[varIdx].m_type == varInfo::TYPE_DELAY)
    {
        // this is a delay assignment!
        assignNode = new ASTNode(ASTNode::NodeDelayAssign);
        assignNode->m_varIdx = varIdx;
        assignNode->right = exprNode;
    }
    else
    {
        // this is regular assignment!
        assignNode = new ASTNode(ASTNode::NodeAssign);
        assignNode->m_varIdx = varIdx;
        assignNode->right = exprNode;
    }

    return assignNode;
}


ASTNode* Parser::acceptDelayDefinition(ParseContext &s)
{
    // production: DELAY IDENT '[' INTEGER ']'
    ParseContext savestate = s;

    if (!match(s,TOK_DELAY))
    {
        s = savestate;
        return NULL;
    }

    if (!match(s,TOK_IDENT))
    {
        s = savestate;
        return NULL;
    }

    if (!match(s,TOK_LBRACKET))
    {
        s = savestate;
        return NULL;
    }

    if (!match(s,TOK_INTEGER))
    {
        error(s,"Integer expected");
        s = savestate;
        return NULL;
    }

    if (!match(s,TOK_RBRACKET))
    {
        s = savestate;
        return NULL;
    }

    std::string identifier = getToken(s, -4).txt;
    std::string lenstr     = getToken(s,-2).txt;

    int32_t delayLen = atoi(lenstr.c_str());
    if (delayLen <= 0)
    {
        error(s, "Delay length cannot be zero or negative!");
        return NULL;
    }

    /* create delay variable.
       Note: do not allocate the delay memory here
             because the destructor will de-allocate it
             and varInfo's are copied before they
             reach the virtual machine.

             Allocation is done by the VM itself!
    */

    ASTNode *delayNode = new ASTNode(ASTNode::NodeDelayDefinition);
    delayNode->m_varIdx = s.createVariable(identifier, varInfo::TYPE_DELAY);
    s.m_variables[delayNode->m_varIdx].m_length = delayLen;
    return delayNode;
}


ASTNode* Parser::acceptExpr(ParseContext &s)
{
    // productions: term expr'
    //
    // the term is the first term
    // and must therefore be
    // added as the left leaf
    //

    ParseContext savestate = s;

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

ASTNode* Parser::acceptExprAccent(ParseContext &s, ASTNode *leftNode)
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

    ParseContext savestate = s;

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

ASTNode* Parser::acceptExprAccent1(ParseContext &s, ASTNode *leftNode)
{
    // production: - term expr'
    ParseContext savestate = s;

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

ASTNode* Parser::acceptExprAccent2(ParseContext &s, ASTNode *leftNode)
{
    // production: + term expr'
    ParseContext savestate = s;

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

ASTNode* Parser::acceptTerm(ParseContext &s)
{
    // production: factor term'
    ParseContext savestate = s;

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

ASTNode* Parser::acceptTermAccent(ParseContext &s, ASTNode *leftNode)
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

    ParseContext savestate = s;

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

ASTNode* Parser::acceptTermAccent1(ParseContext &s, ASTNode* leftNode)
{
    // production: * factor term'
    ParseContext savestate = s;

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
    ASTNode *headNode = acceptTermAccent(s, operationNode);
    return headNode;
}

ASTNode* Parser::acceptTermAccent2(ParseContext &s, ASTNode* leftNode)
{
    // production: / factor term'
    ParseContext savestate = s;

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

    // note: acceptTermAccent will never return NULL
    ASTNode *headNode = acceptTermAccent(s, operationNode);
    return headNode;
}


ASTNode* Parser::acceptFactor(ParseContext &s)
{
    ParseContext savestate = s;

    // DELAYIDENT '[' expr ']'
    ASTNode *factorNode = 0;
    if ((factorNode=acceptFactor1(s)) != NULL)
    {
        return factorNode;
    }

    // FUNCTION ( expr )
    if ((factorNode=acceptFactor2(s)) != NULL)
    {
        return factorNode;
    }

    s = savestate;
    // ( expr )
    if ((factorNode=acceptFactor3(s)) != NULL)
    {
        return factorNode;
    }

    s = savestate;
    // - factor
    if ((factorNode=acceptFactor4(s)) != NULL)
    {
        return factorNode;
    }

    s = savestate;
    if (match(s, TOK_INTEGER))
    {
        factorNode = new ASTNode(ASTNode::NodeInteger);
        factorNode->m_literalInt = atoi(getToken(s, -1).txt.c_str());
        return factorNode;    // INTEGER
    }

    if (match(s, TOK_FLOAT))
    {
        factorNode = new ASTNode(ASTNode::NodeFloat);
        factorNode->m_literalFloat = atof(getToken(s, -1).txt.c_str());
        return factorNode;    // FLOAT
    }

    if (match(s, TOK_IDENT))
    {        
        // we have to check here if we have a delay
        // variable because these cannot be used as
        // regular variables
        uint32_t varIdx = s.getVariableByName(getToken(s, -1).txt);
        if (varIdx != -1)
        {
           if (s.m_variables[varIdx].m_type == varInfo::TYPE_DELAY)
           {
               error(s, "Cannot use a delay variable without an index");
               return NULL;
           }
        }

        // if needed, create the variable
        varIdx = s.createVariable(getToken(s,-1).txt);
        factorNode = new ASTNode(ASTNode::NodeIdent);
        factorNode->m_varIdx = varIdx;
        return factorNode;    // IDENT
    }

    error(s, "Factor is not an integer, float, identifier or parenthesised expression.");
    return NULL;
}


ASTNode* Parser::acceptFactor1(ParseContext &s)
{
    // DELAYIDENT '[' expr ']'
    ParseContext savestate = s;
    if (!match(s, TOK_IDENT))
    {
        s = savestate;
        return NULL;
    }

    if (!match(s, TOK_LBRACKET))
    {
        s = savestate;
        return NULL;
    }

    std::string varname = getToken(s,-2).txt;

    // get the expression argument
    ASTNode *exprNode = 0;
    if ((exprNode=acceptExpr(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // check if identifier is of a delay type!
    int32_t varIdx = s.getVariableByName(varname);
    if (varIdx < 0)
    {
        delete exprNode;
        s = savestate;
        error(s,"Undefined delay variable!");
        return NULL;
    }

    if (!match(s, TOK_RBRACKET))
    {
        delete exprNode;
        s = savestate;
        return NULL;
    }

    // create a delayline lookup node
    ASTNode *delayNode = new ASTNode(ASTNode::NodeDelayLookup);
    delayNode->m_varIdx = varIdx;
    delayNode->left = exprNode;
    return delayNode;
}


ASTNode* Parser::acceptFactor2(ParseContext &s)
{
    // production: FUNCTION ( expr )

    ParseContext savestate = s;
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
    factorNode->m_functionID = func.tokID;
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
        factorNode->m_functionArgs.push_back(exprNode);
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


ASTNode* Parser::acceptFactor3(ParseContext &s)
{
    ParseContext savestate = s;
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


ASTNode* Parser::acceptFactor4(ParseContext &s)
{
    // production: - factor
    ParseContext savestate = s;
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

