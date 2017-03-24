/*

  BasicDSP - tokenizer

  Description:  The tokenizer takes input from a
                source reader and produces a
                list of tokens that can be
                fed into a parser.

  Copyright Niels A. Moseley (c) 2016,2017.

*/

#include <sstream>
#include "virtualmachine.h"
#include "functiondefs.h"
#include "tokenizer.h"

Tokenizer::Tokenizer()
{
}

bool Tokenizer::isDigit(char c) const
{
    if ((c >= '0') && (c <= '9')) return true;
    return false;
}

bool Tokenizer::isWhitespace(char c) const
{
    if ((c == ' ') || (c == '\t')) return true;
    return false;
}

bool Tokenizer::isAlpha(char c) const
{
    if ((c >= 'A') && (c <= 'Z')) return true;
    if ((c >= 'a') && (c <= 'z')) return true;
    return false;
}

bool Tokenizer::isNumeric(char c) const
{
    if ((c >= '0') && (c <= '9')) return true;
    return false;
}

bool Tokenizer::isAlphaNumeric(char c) const
{
    if (isAlpha(c)) return true;
    if (isNumeric(c)) return true;
    return false;
}

bool Tokenizer::isAlphaNumericExtended(char c) const
{
    if (isAlpha(c)) return true;
    if (isNumeric(c)) return true;
    if (c == '_') return true;
    return false;
}

bool Tokenizer::process(Reader *r, std::vector<token_t> &result)
{
    if (r == 0)
    {
        m_lastError = std::string("Reader object was NULL!");
        return false;
    }

    tok_state_t state = S_BEGIN;
    token_t tok;
    while(state != S_DONE)
    {
        char c = r->peek();

        // check for end of file
        // if found, exit this while loop..
        if (c == 0)
        {
            // if the state is not S_BEGIN
            // it means we were still
            // processing the previous token
            //
            // we continue execution with c == 0
            // to that the token will be emitted
            // and the state will be S_BEGIN
            if (state == S_BEGIN)
            {
                tok.txt.clear();
                tok.pos = r->getPos();
                tok.tokID = TOK_EOF;
                result.push_back(tok);

                state = S_DONE;
                continue;   // exit asap.
            }
        }

        switch(state)
        {
        case S_BEGIN:

            tok.txt.clear();
            tok.pos = r->getPos();
            tok.tokID = TOK_UNKNOWN;

            // remove whitespace
            if (isWhitespace(c))
            {
                r->accept();
                continue;
            }

            // check for a line-spanning comment
            if (c == '%')
            {
                r->accept();
                state = S_COMMENT;
                continue;
            }

            // do everything else..
            if (isAlpha(c))
            {
                tok.txt += r->accept();
                state = S_IDENT;
            }
            else if (c == 10)   // character is newline?
            {
                // emit a newline
                tok.tokID = TOK_NEWLINE;
                result.push_back(tok);
                r->accept();
            }
            else if (c == 13)   // character is a carriage return?
            {
                // ignore carriage return
                r->accept();
            }
            else if (c == ')')
            {
                tok.tokID = TOK_RPAREN;
                result.push_back(tok);
                r->accept();
            }
            else if (c == '(')
            {
                tok.tokID = TOK_LPAREN;
                result.push_back(tok);
                r->accept();
            }
            else if (c == ']')
            {
                tok.tokID = TOK_RBRACKET;
                result.push_back(tok);
                r->accept();
            }
            else if (c == '[')
            {
                tok.tokID = TOK_LBRACKET;
                result.push_back(tok);
                r->accept();
            }
            else if (c == '=')
            {
                tok.tokID = TOK_EQUAL;
                result.push_back(tok);
                r->accept();
            }
            else if (c == '+')
            {
                tok.tokID = TOK_PLUS;
                result.push_back(tok);
                r->accept();
            }
            else if (c == '-')
            {
                // This could be the start of a negative
                // number, or a regular subtraction
                // For now, we'll interpret it as a
                // regular subtraction.

                r->accept();

                /* OBSOLETE CODE but keep for future reference

                if (isNumeric(r->peek()))
                {
                    // it's part of an integer, or float!
                    state = S_INTEGER;
                    tok.txt = "-";
                }
                else
                {
                    // it's a single minus!
                    tok.tokID = TOK_MINUS;  // assume it's a single minus
                    result.push_back(tok);
                }
                */

                tok.tokID = TOK_MINUS;
                result.push_back(tok);
            }
            else if (c == '*')
            {
                tok.tokID = TOK_STAR;
                result.push_back(tok);
                r->accept();
            }
            else if (c == '/')
            {
                tok.tokID = TOK_SLASH;
                result.push_back(tok);
                r->accept();
            }
            else if (c == '>')
            {
                // we can't just emit a single '>'
                // because we might be looking at a '>>'
                // so, we use an additional state
                state = S_LARGER;
                r->accept();
            }
            else if (c == '<')
            {
                // we can't just emit a single '<'
                // because we might be looking at a '<<'
                // so, we use an additional state
                state = S_SMALLER;
                r->accept();
            }
            else if (c == ',')
            {
                tok.tokID = TOK_COMMA;
                result.push_back(tok);
                r->accept();
            }
            else if (c == ';')
            {
                tok.tokID = TOK_SEMICOL;
                result.push_back(tok);
                r->accept();
            }
            else if (isNumeric(c))
            {
                // we could have an integer,
                // float (with or without exponent ..)
                // first, we assume it's an integer

                tok.txt += c;
                r->accept();
                state = S_INTEGER;
            }
            else
            {
                // unknown token character!
                std::stringstream ss;
                ss << "Unknown character: ";
                ss << tok.txt;
                ss << c;
                ss << " on line: " << tok.pos.line+1 << " column " << tok.pos.pos+1;
                m_lastError = ss.str();
                m_lastErrorPos = tok.pos;
                return false;
            }
            break;
        case S_LARGER:
            tok.tokID = TOK_LARGER;
            result.push_back(tok);
            state = S_BEGIN;
            break;
        case S_SMALLER:
            // we have found a single larger than
            // and we must not accept the current
            // character!
            tok.tokID = TOK_SMALLER;
            result.push_back(tok);
            state = S_BEGIN;
            break;
        case S_IDENT:   // read in keywords and identifiers
            if (isAlphaNumericExtended(c))
            {
                tok.txt += c;
                r->accept();
            }
            else
            {
                // check if it is a function
                bool found = false;
                for(size_t i=0; i<g_functionDefsLen; i++)
                {
                    if (g_functionDefs[i].name == tok.txt)
                    {
                        // keyword found!
                        tok.tokID = g_functionDefs[i].ID;
                        result.push_back(tok);
                        found = true;
                        continue;
                    }
                }
                // check if it's a keyword
                if (tok.txt == std::string("delay"))
                {
                    tok.tokID = TOK_DELAY;
                    found = true;
                    result.push_back(tok);
                }
                if (!found)
                {
                    // if we end up here, it must be an
                    // identifier!
                    tok.tokID = TOK_IDENT;
                    result.push_back(tok);
                }
                state = S_BEGIN;
            }
            break;
        case S_INTEGER:
            if (isNumeric(c))
            {
                tok.txt += c;
                r->accept();
            }
            else if (c == '.')
            {
                // period means we're reading a float value
                tok.txt += '.';
                r->accept();
                state = S_FLOAT;
            }
            else
            {
                tok.tokID = TOK_INTEGER;
                result.push_back(tok);
                state = S_BEGIN;
            }
            break;
        case S_FLOAT:
            if (isNumeric(c))
            {
                tok.txt += c;
                r->accept();
            }
            else if (c == 'e')
            {
                // we're reading a float with an exponent
                tok.txt += c;
                r->accept();
                state = S_FLOAT_WITH_EXP;
            }
            else
            {
                tok.tokID = TOK_FLOAT;
                result.push_back(tok);
                state = S_BEGIN;
            }
            break;
        case S_FLOAT_WITH_EXP:
            // the exponent may be negative
            // so check for this..
            if (isNumeric(c))
            {
                // positive exponent
                tok.txt += c;
                r->accept();
                state = S_FLOAT_WITH_POSEXP;
            }
            else if (c == '-')
            {
                tok.txt += '-';
                r->accept();
                state = S_FLOAT_WITH_NEGEXP;
            }
            else
            {
                // ill-formatted floating point!
                m_lastError = std::string("Floating point has ill-formatted exponent!");
                m_lastErrorPos = tok.pos;
                return false;
            }
            break;
        case S_FLOAT_WITH_POSEXP:
        case S_FLOAT_WITH_NEGEXP:
            if (isNumeric(c))
            {
                tok.txt += c;
                r->accept();
            }
            else
            {
                // end of floating point
                tok.tokID = TOK_FLOAT;
                result.push_back(tok);
                state = S_BEGIN;
            }
            break;
        case S_COMMENT:
            // only EOF and newline can break a line spanning comment!
            if ((c != 0) && (c != 10))
            {
                r->accept();
            }
            else
            {
                state = S_BEGIN;
            }
            break;
        case S_DONE:
            break;
        default:
            tok.tokID = TOK_UNKNOWN;
            m_lastError = std::string("Unknown token found.");
            m_lastErrorPos = tok.pos;
            return false;
        }
    }
    return true;
}


#if 0
void Tokenizer::dumpTokens(std::ostream &stream, const std::vector<token_t> &tokens)
{

    doLog(LOG_INFO, "Dumping tokens: \n");

    // show the tokens
    uint32_t newlineCnt = 0;
    for(size_t i=0; i<tokens.size(); i++)
    {
        token_t token  = tokens[i];
        if (token.tokID != TOK_NEWLINE)
        {
            newlineCnt = 0;
        }

        switch(token.tokID)
        {
        default:
        case TOK_UNKNOWN:
            stream << "Unknown\n";
            break;
        case TOK_NEWLINE:   // suppress superfluous newlines
            if (newlineCnt == 0)
                stream <<"\n";
            newlineCnt++;
            break;
        case TOK_IDENT:
            stream << "<ident>" << token.txt.c_str();
            break;
        case TOK_INTEGER:
            stream << "<int>" << token.txt.c_str();
            break;
        case TOK_FLOAT:
            stream << "<float>" << token.txt.c_str();
            break;
        case TOK_PLUS:
            stream << " + ";
            break;
        case TOK_MINUS:
            stream << " - ";
            break;
        case TOK_STAR:
            stream << " * ";
            break;
        case TOK_EQUAL:
            stream << " = ";
            break;
        case TOK_LPAREN:
            stream << " ( ";
            break;
        case TOK_RPAREN:
            stream << " ) ";
            break;
        case TOK_COMMA:
            stream << " , ";
            break;
        case TOK_SEMICOL:
            stream << ";";
            break;
        case TOK_SHL:
            stream << " << ";
            break;
        case TOK_SHR:
            stream << " >> ";
            break;
        case TOK_ROL:
            stream << " <<< ";
            break;
        case TOK_ROR:
            stream << " >>> ";
            break;
        case TOK_EOF:
            stream << "\nEOF\n";
            break;
        case 100:
            stream << "DEFINE";
            break;
        case 101:
            stream << "INPUT";
            break;
        case 102:
            stream << "CSD";
            break;
        }
    }

}
#endif
