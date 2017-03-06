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
        m_type        = nodeType;
        m_functionID  = 0xFFFFFFFF;
    }

    ~ASTNode()
    {
        if (left != 0)
            delete left;
        if (right != 0)
            delete right;
        for(uint32_t i=0; i<m_functionArgs.size(); i++)
        {
            delete m_functionArgs[i];
        }
    }

    void dump(std::ostream &stream, uint32_t level = 0) const
    {
        if (left != 0)
        {
            left->dump(stream, level+1);
        }

        if (right != 0)
        {
            right->dump(stream, level+1);
        }

        for(uint32_t i=0; i<m_functionArgs.size(); i++)
        {
            m_functionArgs[i]->dump(stream, level+1);
        }

        // indent according to level
        for(uint32_t i=0; i<level; i++)
            stream << "  ";

        switch(m_type)
        {
        case NodeUnknown:
            stream << "Unknown";
            break;
        case NodeAssign:
            if (m_varIdx != -1)
            {
                stream << "Assign varIdx = " << m_varIdx;
            }
            else
            {
                stream << "undefined variable!\n";
            }
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
            if (m_varIdx != -1)
            {
                stream << "varIdx = " << m_varIdx;
            }
            else
            {
                stream << "undefined variable!\n";
            }
            break;
        case NodeInteger:
            if (m_varIdx != -1)
                stream << m_literalInt << "(INT)";
            break;
        case NodeFunction:
            stream << "Function ";
            switch(m_functionID)
            {
            default:
                stream << "unimplemented!\n";
            }
            break;
        case NodeFloat:

            stream << m_literalFloat << "(FLOAT)";
            break;
        default:
            stream << "???";
            break;
        }
        stream << std::endl;
    }

    node_t          m_type;         // the type of the node
    uint32_t        m_varIdx;       // index into m_varInfo array of parseContext
    int32_t         m_literalInt;
    float           m_literalFloat;
    uint32_t        m_functionID;   // function ID

    ASTNode  *left;
    ASTNode  *right;

    // function arguments go here instead of the left and right pointers!
    std::vector<ASTNode *> m_functionArgs;
};

typedef std::vector<ASTNode*> statements_t;



/** object that keeps track of the state */
class ParseContext
{
public:
    size_t                tokIdx;
    Reader::position_info tokPos;

    /** get variable by name, or -1 if not found */
    int32_t getVariableByName(const std::string &name);

    /** create a variable and return its index */
    int32_t createVariable(const std::string &name);

    /** add a statement to the list of statement */
    void addStatement(ASTNode* statement);

    /** get (const) access to the statements */
    const statements_t& getStatements() const
    {
        return m_statements;
    }

    /** get (const) access to variables */
    const std::vector<varInfo>& getVariables() const
    {
        return m_varInfo;
    }

protected:
    std::vector<varInfo>  m_varInfo;
    statements_t          m_statements;
};



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
    bool process(const std::vector<token_t> &tokens, ParseContext &context);

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
    /* The following methods return true if the tokens starting from
       index 'tokIdx' are consistent with the production from the
       BasicDSP grammar.
    */

    bool acceptProgram(ParseContext &context);

    /** production: assignment -> IDENT = expr */
    ASTNode* acceptAssignment(ParseContext &s);

    /** production: expr -> term expr' */
    ASTNode* acceptExpr(ParseContext &s);

    /** production: expr' -> - term expr' | + term expr' | e

        This function will return leftNode when an
        epsilon production is invoked. Therfore,
        it will never return NULL.
    */
    ASTNode* acceptExprAccent(ParseContext &s, ASTNode *leftNode);

    /** production: expr' -> - term expr' */
    ASTNode* acceptExprAccent1(ParseContext &s, ASTNode *leftNode);

    /** production: expr' -> + term expr' */
    ASTNode* acceptExprAccent2(ParseContext &s, ASTNode *leftNode);

    /** production: term -> factor term' */
    ASTNode* acceptTerm(ParseContext &s);

    /** production: term' -> * factor term' | / factor term' | e

        This function will return leftNode when an
        epsilon production is invoked. Therfore,
        it will never return NULL.
    */
    ASTNode* acceptTermAccent(ParseContext &s, ASTNode *leftNode);

    /** production: term' -> * factor term' */
    ASTNode* acceptTermAccent1(ParseContext &s, ASTNode *leftNode);

    /** production: term' -> / factor term' */
    ASTNode* acceptTermAccent2(ParseContext &s, ASTNode *leftNode);

    ASTNode* acceptFactor(ParseContext &s);

    /** production: FUNCTION ( expr ) */
    ASTNode* acceptFactor1(ParseContext &s);

    /** production: ( expr ) */
    ASTNode* acceptFactor2(ParseContext &s);

    /** production: - factor */
    ASTNode* acceptFactor3(ParseContext &s);

    /** match a token, return true if matched and advance the token index. */
    bool match(ParseContext &s, uint32_t tokenID);

    /** match a NULL-terminated list of tokens. */
    bool matchList(ParseContext &s, const uint32_t *tokenIDlist);

    /** Advance the token index and get the next token */
    token_t next(ParseContext &s)
    {
        s.tokIdx++;
        token_t tok = getToken(s);
        s.tokPos = tok.pos;
        return getToken(s);
    }

    /** Get the current token, which or without an offset w.r.t.*/
    token_t getToken(const ParseContext &s, int32_t offset = 0)
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
    void error(const ParseContext &s, const std::string &txt);
    void error(uint32_t dummy, const std::string &txt);

    std::string   m_lastError;
    Reader::position_info m_lastErrorPos;
    const std::vector<token_t>  *m_tokens;
};

#endif
