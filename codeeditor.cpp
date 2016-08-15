/*

  Description:  BasicDSP code editor widget

  Author: Niels A. Moseley (c) 2016

  License: GPLv2

  Based on: http://doc.qt.io/qt-4.8/qt-widgets-codeeditor-example.html

*/


#include <QPainter>
#include <QTextBlock>
#include "codeeditor.h"

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(const QRect&,int)), this, SLOT(updateLineNumberArea(const QRect &, int)));
    //connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightErrorLine()));

    m_bkColor = QColor(Qt::lightGray).lighter(100);
    m_numColor = m_bkColor.darker();
    m_lineColor = Qt::white;
    m_errorColor = QColor(Qt::red);

    updateLineNumberAreaWidth(0);
}

void CodeEditor::setErrorLine(uint32_t lineNumber)
{
    m_errorLine = lineNumber;
    highlightErrorLine();
    update();
}

uint32_t CodeEditor::lineNumberAreaWidth()
{
    uint32_t digits = 1;
    uint32_t max = qMax(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }

    uint32_t space = fontMetrics().width(QLatin1Char('X')) * (digits+1);

    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int newBlockCount)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int line)
{
    if (line)
        lineNumberArea->scroll(0, line);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                      lineNumberAreaWidth(),
                                      cr.height()));
}

void CodeEditor::highlightErrorLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly() && (m_errorLine > 0))
    {
        QTextEdit::ExtraSelection selection;

        QTextDocument *doc = document();
        QTextCursor cursorPosition(doc->findBlockByLineNumber(m_errorLine-1));
        selection.format.setBackground(m_errorColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = cursorPosition;
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}


void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);

    painter.fillRect(event->rect(), m_bkColor);

    QTextBlock block = firstVisibleBlock();
    uint32_t blockNumber = block.blockNumber();
    uint32_t top = (uint32_t)blockBoundingGeometry(block).translated(contentOffset()).top();
    uint32_t bottom = top + (uint32_t) blockBoundingRect(block).height();

    while(block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(m_numColor);
            painter.drawText(0, top, lineNumberArea->width()-3,
                             fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (uint32_t) blockBoundingRect(block).height();
        blockNumber++;
    }
}

