/*

  VUMeter Qt edition
  Niels A. Moseley 2016

  based on code created by P.T. de Boer

  License: GPLv2

*/

#include <QPainter>
#include <QFontDatabase>
#include <math.h>
#include "vumeter.h"

VUMeter::VUMeter(const QString &name, QWidget *parent)
    : QWidget(parent), m_name(name)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    init();

    const QFont smallFont = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
    setFont(smallFont);
}

void VUMeter::init()
{
    m_level = 1e-40f;
    m_dbmin = -54.0f;
    m_dbmax = 3.0f;
}

QSize VUMeter::sizeHint() const
{
    QFontMetrics metrics(font());
    return QSize(0, metrics.height()+10);
}

int32_t VUMeter::db2pixels(float db)
{
    return static_cast<int32_t>((db - m_dbmin) / (m_dbmax-m_dbmin) * width());
}

void VUMeter::setLevel(double level)
{
    m_level = level;
}

void VUMeter::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    float db = 20.0*log10(m_level);
    db = db < m_dbmin ? m_dbmin : db;
    db = db > m_dbmax ? m_dbmax : db;

    int32_t pixels = db2pixels(db);

    if (db <= -3.0f)
    {
        // -3 dB or less,
        // draw in green
        painter.fillRect(0,0, pixels, height(), Qt::green);
    }
    else
    {
        // draw green up to -3dB
        // yellow to 0 dB and red over 0dB
        int c = db2pixels(-3.0f);
        painter.fillRect(0,0, c, height(), Qt::green);

        if (db <= 0.0f)
        {
            // not clipping
            painter.fillRect(c,0, pixels-c, height(), Qt::yellow);
        }
        else
        {
            // clipping
            int d = db2pixels(0.0f);
            painter.fillRect(c,0, d-c, height(), Qt::yellow);
            painter.fillRect(d,0, pixels-d, height(), Qt::red);
        }
    }

    // draw the scale
    QString numstr;
    QFontMetrics metrics(font());

    for(int i=-48; i<=0; i+=6)
    {
        int32_t xpos = db2pixels((double)i);
        painter.drawLine(xpos, 0, xpos, 5);
        numstr = QString("%1").arg(i, 2);
        QSize textSize = metrics.size(Qt::TextSingleLine, numstr);
        int32_t textXPos = xpos - textSize.width()/2;
        QPoint upperLeft(textXPos, 7);
        QRect textBox(upperLeft, textSize);
        painter.drawText(textBox, numstr);
    }

    // draw meter name
    QSize textSize = metrics.size(Qt::TextSingleLine, m_name);
    QPoint upperLeft(10, (height()- textSize.height())/2);
    QRect textBox(upperLeft, textSize);
    painter.drawText(textBox, m_name);

    // draw frame
    painter.setPen(Qt::black);
    QRect r = rect();
    r.setRight(r.right()-1);
    r.setBottom(r.bottom()-1);
    painter.drawRect(r);
}

