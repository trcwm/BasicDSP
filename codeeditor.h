/*

  Description:  BasicDSP code editor widget

  Author: Niels A. Moseley (c) 2016

  License: GPLv2

  Based on: http://doc.qt.io/qt-4.8/qt-widgets-codeeditor-example.html

*/

#ifndef codeeditor_h
#define codeeditor_h

#include <QPlainTextEdit>
#include <QWidget>
#include <QResizeEvent>
#include <stdint.h>


/** code editor class with line numbering */
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    uint32_t lineNumberAreaWidth();

    /** set the line number which caused the
        parse error.
        set to 0 if no error
     */
    void setErrorLine(uint32_t lineNumber);

protected:
    void resizeEvent(QResizeEvent *e);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightErrorLine();
    void updateLineNumberArea(const QRect &rect, int line);

private:
    QColor  m_bkColor;
    QColor  m_numColor;
    QColor  m_errorColor;
    QColor  m_lineColor;
    uint32_t m_errorLine;
    QWidget *lineNumberArea;
};


/** line number area widget helper class */
class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor) {
        codeEditor = editor;
    }

    QSize sizeHint() const {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *codeEditor;
};

#endif
