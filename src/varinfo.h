/*

  Variable information for parser and VM

*/

#ifndef varinfo_h
#define varinfo_h

#include <string>
#include <stdint.h>

/** variable related information */
struct varInfo
{
    varInfo()
    {
        m_type   = TYPE_VAR;
        m_data   = 0;
        m_value  = 0.0f;
        m_idx    = 0;
        m_length = 0;
    }

    virtual ~varInfo()
    {
        if (m_data != 0)
            delete[] m_data;
    }

    enum type_t {TYPE_VAR, TYPE_DELAY};
    std::string     m_name;     // variable or delay name
    type_t          m_type;     // type of the variable
    float           m_value;    // current value for the VM
    int32_t         m_idx;      // current delay index
    int32_t         m_length;   // delay length
    float           *m_data;    // delay memory pointer
};

#endif
