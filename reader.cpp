/*

  Source code reader for the BasicDSP language
  Niels A. Moseley 2016

  License: GPLv2

*/

#include <stdio.h>
#include "reader.h"

Reader::Reader()
{
    m_curpos.line = 0;
    m_curpos.offset = 0;
    m_curpos.pos = 0;
}

Reader::~Reader()
{
}

Reader* Reader::create(const QString &sourceCodeString)
{
    Reader *reader = new Reader();

    QByteArray ar = sourceCodeString.toLocal8Bit();
    reader->m_source.resize(ar.length());

    if (ar.length() == 0)
        return NULL;

    memcpy(&(reader->m_source[0]), ar.constData(), ar.length());
    return reader;
}

bool Reader::rollback()
{
    if (!m_positions.empty())
    {
        position_info p = m_positions.top();
        m_positions.pop();
        m_curpos = p;
        return true;
    }
    return false;
}

char Reader::peek()
{
    if (m_curpos.offset < m_source.size())
    {
        return m_source[m_curpos.offset];
    }
    else
        return 0;
}

char Reader::accept()
{
    char c = peek();
    if (c != 0)
    {
        // read succesful!
        m_curpos.offset++;
        m_curpos.pos++;
        if (c == 10)
        {
            m_curpos.line++;
            m_curpos.pos = 0;
        }
        return c;
    }
    return 0;
}

void Reader::mark()
{
    m_positions.push(m_curpos);
}
