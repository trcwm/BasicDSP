#ifndef SCOPEWINDOW_H
#define SCOPEWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QSpinBox>

#include "virtualmachine.h"
#include "scopewidget.h"

namespace Ui {
class ScopeWindow;
}

class ScopeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ScopeWindow(QWidget *parent = 0);
    ~ScopeWindow();

    void submit256Samples(VirtualMachine::ring_buffer_data_t *buffer);

    /** set the sample rate for x-axis scaling */
    void setSampleRate(float rate);

    /** get the name of the channel name */
    std::string getChannelName(uint32_t channel);

signals:
    void channelChanged(uint32_t channel);

private slots:
    void chan1Changed();
    void chan2Changed();
    void triggerChanged();
    void triggerLevelChanged(int);

private:
    Ui::ScopeWindow *ui;

    QGroupBox *createTriggerLevelGroup();
    QGroupBox *createTriggerChannelGroup();

    ScopeWidget *m_scope;
    QLineEdit   *m_chan1;
    QLineEdit   *m_chan2;

    QRadioButton *m_trigNone;
    QRadioButton *m_trigCh1;
    QRadioButton *m_trigCh2;
    QSpinBox     *m_triggerSpin;
};

#endif // SCOPEWINDOW_H
