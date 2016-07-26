/*

  Source code reader for the BasicDSP language
  Niels A. Moseley 2016

  License: GPLv2

*/

#ifndef reader_h
#define reader_h

#include <QString>
#include <stdint.h>
#include <stack>
#include <vector>

/** The reader object reads a source file into memory and provides
    an interface to the characters that supports roll-backs.
*/

class Reader
{
public:
  virtual ~Reader();

  struct position_info
  {
    size_t  offset;     // offset into m_source
    size_t  line;       // the line number
    size_t  pos;        // the position within the line
  };

  /** Create a reader object using source code in a QString
      NULL is returned when an error occured. */
  static Reader* create(const QString &sourceCodeString);

  /** Rollback the read pointer to the last marked position.
      When succesfull, the marked position is removed from
      the stack.
      Returns false if there are no markers left.
    */
  bool rollback();

  /** Mark the current read position so we can roll back to
      it later */
  void mark();

  /** Get the charater at the current read position.
        The read position is not advanced.
        When there are no characters to read,
        it returns 0.
    */
  char peek();

  /** Read the character at the current read position.
      The read position is advanced one character.
      When there are no characters to read,
      it returns 0.
    */
  char accept();

  /** Get the current read position */
  position_info getPos() const
  {
    return m_curpos;
  }

protected:
  /** Hide the constructor so the user can only get
      a Reader object by using 'open'.
    */
  Reader();

  std::vector<char>   m_source;               // the source code
  std::stack<position_info>  m_positions;     // a stack to hold roll-back positions.
  position_info   m_curpos;                   // the current read position.
};

#endif
