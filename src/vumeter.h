/*

  VUMeter Qt edition
  Niels A. Moseley 2016

  based on code created by P.T. de Boer

  License: GPLv2

*/

#ifndef vumeter_h
#define vumeter_h

#include <stdint.h>
#include <QWidget>

class VUMeter : public QWidget
{
    Q_OBJECT

public:
    VUMeter(const QString &name, QWidget *parent);

    /** init the internal VU values */
    void init();

    /** set the VU level in linear amount, 0.0 .. 1.0 */
    void setLevel(double level);

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);

    int32_t db2pixels(float db);

    float m_level;
    float m_dbmin;
    float m_dbmax;
    QString m_name;
};

#endif
