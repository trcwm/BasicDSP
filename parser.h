/*

  Description:  The parser takes a list of tokens,
                checks the grammar and produces
                a parse tree.

  Author: Niels A. Moseley (c) 2016

  License: GPLv2

*/

#ifndef parser_h
#define parser_h

#include <string>
#include <vector>
#include <memory>
#include <ostream>

#include "tokenizer.h"

/** variable related information */
struct varInfo
{
    std::string     txt;        // identifier name, integer or float value.
    int32_t         intVal;
    float           floatVal;
};

/** Abstract Syntax Tree Node */
class ASTNode
{
public:
  enum node_t {NodeUnknown, NodeHead,
               NodeStatement,
               NodeAssign,
               NodeExpr,
               NodeExprAccent,
               NodeTerm,
               NodeTermAccent,
               NodeFactor,
               NodeAdd,
               NodeSub,
               NodeMul,
               NodeDiv,
               NodeFunction,
               NodeUnaryMinus,
               NodeIdent,
               NodeInteger,
               NodeFloat
              };

    ASTNode(node_t nodeType = NodeUnknown)
    {
        left        = 0;
        right       = 0;
        type        = nodeType;
        functionID  = 0xFFFFFFFF;
    }

    ~ASTNode()
    {
        if (left != 0)
            delete left;
        if (right != 0)
            delete right;
        for(uint32_t i=0; i<function_args.size(); i++)
        {
            delete function_args[i];
        }
    }

    void dump(std::ostream &stream, uint32_t level = 0)
    {
        if (left != 0)
        {
            left->dump(stream, level+1);
        }

        if (right != 0)
        {
            right->dump(stream, level+1);
        }

        // indent according to level
        for(uint32_t i=0; i<level; i++)
            stream << "  ";

        switch(type)
        {
        case NodeUnknown:
            stream << "Unknown";
            break;
        case NodeAssign:
            stream << info.txt << " = ";
            break;
        case NodeAdd:
            stream << "+";
            break;
        case NodeSub:
            stream << "-";
            break;
        case NodeMul:
            stream << "*";
            break;
        case NodeDiv:
            stream << "\\";
            break;
        case NodeUnaryMinus:
            stream << "U-";
            break;
        case NodeIdent:
            stream << info.txt;
            break;
        case NodeInteger:
            stream << info.intVal << "(INT)";
            break;
        case NodeFunction:
            stream << "Function " << info.txt;
            break;
        case NodeFloat:
            stream << info.floatVal << "(FLOAT)";
            //stream << floatVal;
            break;
        default:
            stream << "???";
            break;
        }
        stream << std::endl;
    }

    node_t          type;       // the type of the node
    varInfo         info;       // variable related information
    uint32_t        functionID; // function ID

    ASTNode  *left;
    ASTNode  *right;

    // function arguments go here instead of the left and right pointers!
    std::vector<ASTNode *> function_args;
};

typedef std::vector<ASTNode*> statements_t;

/** Parser to translate token stream from tokenizer/lexer to operation stack. */
class Parser
{
public:
    Parser();

    /** Process a list of tokens and list of statements.
        false is returned when a parse error occurs.
        When an error occurs, call getLastError() to get
        a human-readable string of the error.
    */
    bool process(const std::vector<token_t> &tokens, statements_t &result);

    /** Return a description of the last parse error that occurred. */
    std::string getLastError() const
    {
        return m_lastError;
    }

    /** Get the position in the source code where the last error occurred. */
    Reader::position_info getLastErrorPos() const
    {
        return m_lastErrorPos;
    }

protected:
    struct state_t
    {
        size_t        tokIdx;
        Reader::position_info tokPos;
    };

    /* The following methods return true if the tokens starting from
       index 'tokIdx' are consistent with the production from the
       FPTOOL grammar.

       All functions return false when the production was not succesful.
       Each method is responsible for filling in the 'newNode' information
       and creating any subnodes needed for further processing by
       recusively calling other accpet methods.
    */

    bool acceptProgram(state_t &s, statements_t &result);
    ASTNode* acceptDefinition(state_t &s);

    /** production: assignment -> IDENT = expr */
    ASTNode* acceptAssignment(state_t &s);

    /** production: expr -> term expr' */
    ASTNode* acceptExpr(state_t &s);

    /** production: expr' -> - term expr' | + term expr' | e

        This function will return leftNode when an
        epsilon production is invoked. Therfore,
        it will never return NULL.
    */
    ASTNode* acceptExprAccent(state_t &s, ASTNode *leftNode);

    /** production: expr' -> - term expr' */
    ASTNode* acceptExprAccent1(state_t &s, ASTNode *leftNode);

    /** production: expr' -> + term expr' */
    ASTNode* acceptExprAccent2(state_t &s, ASTNode *leftNode);

    /** production: term -> factor term' */
    ASTNode* acceptTerm(state_t &s);

    /** production: term' -> * factor term' | / factor term' | e

        This function will return leftNode when an
        epsilon production is invoked. Therfore,
        it will never return NULL.
    */
    ASTNode* acceptTermAccent(state_t &s, ASTNode *leftNode);

    /** production: term' -> * factor term' */
    ASTNode* acceptTermAccent1(state_t &s, ASTNode *leftNode);

    /** production: term' -> / factor term' */
    ASTNode* acceptTermAccent2(state_t &s, ASTNode *leftNode);

    ASTNode* acceptFactor(state_t &s);

    /** production: FUNCTION ( expr ) */
    ASTNode* acceptFactor1(state_t &s);

    /** production: ( expr ) */
    ASTNode* acceptFactor2(state_t &s);

    /** production: - factor */
    ASTNode* acceptFactor3(state_t &s);

    /** match a token, return true if matched and advance the token index. */
    bool match(state_t &s, uint32_t tokenID);

    /** match a NULL-terminated list of tokens. */
    bool matchList(state_t &s, const uint32_t *tokenIDlist);

    /** Advance the token index and get the next token */
    token_t next(state_t &s)
    {
        s.tokIdx++;
        token_t tok = getToken(s);
        s.tokPos = tok.pos;
        return getToken(s);
    }

    /** Get the current token, which or without an offset w.r.t.*/
    token_t getToken(const state_t &s, int32_t offset = 0)
    {
        token_t dummy_token;

        if (m_tokens == 0)
            return dummy_token;

        if ((s.tokIdx+offset) < m_tokens->size())
        {
            return m_tokens->at(s.tokIdx+offset);
        }
        else
        {
            return dummy_token;
        }
    }

    /** Report an error */
    void error(const state_t &s, const std::string &txt);
    void error(uint32_t dummy, const std::string &txt);

    std::string   m_lastError;
    Reader::position_info m_lastErrorPos;
    const std::vector<token_t>  *m_tokens;
};

#endif
