/*

  Named slider
  Niels A. Moseley 2016

  License: GPLv2

*/

#include <QDebug>
#include "namedslider.h"

NamedSlider::NamedSlider(QString name, QWidget *parent)
    : QWidget(parent)
{
    m_layout = new QHBoxLayout(this);

    m_label = new QLabel(name, this);
    m_layout->addWidget(m_label);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(0, 1000);
    m_layout->addWidget(m_slider);

    m_edit = new QLineEdit(this);
    m_edit->setText("0.000");
    m_edit->setMaxLength(5);

    // fix the size of the line edit box
    QString text = m_edit->text();
    QFontMetrics fm = m_edit->fontMetrics();
    int w = fm.boundingRect(text).width()*1.5;
    m_edit->setFixedSize(w, m_edit->height());
    m_layout->addWidget(m_edit);

    // remove margins to get a tigher layout!
    m_layout->setMargin(0);

    //BUG: double validator doesn't seem to work properly..
    //m_validator.setRange(0.0, 10.0, 5);
    //m_edit->setValidator(&m_validator);

    v = 0;

    connect(m_edit, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
}

void NamedSlider::editingFinished()
{
    // get number from line edit
    bool ok;
    v = m_edit->text().toFloat(&ok);
    v = v > 1.0f ? 1.0f : v;
    v = v < 0.0f ? 0.0f : v;

    if (ok)
    {
        // update slider
        m_slider->setValue(static_cast<int>(v*1000.0));
        emit valueChanged(v);
    }
}

void NamedSlider::sliderChanged(int sliderValue)
{
    v = static_cast<float>(sliderValue) / 1000.0f;
    v = v > 1.0f ? 1.0f : v;
    v = v < 0.0f ? 0.0f : v;

    QString txt = QString("%1").arg(v, 3,'f',3);
    m_edit->setText(txt);
    emit valueChanged(v);
}

QSize NamedSlider::sizeHint() const
{
    return m_layout->sizeHint();
}
