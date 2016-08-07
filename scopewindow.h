#ifndef SCOPEWINDOW_H
#define SCOPEWINDOW_H

#include <QDialog>
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

private:
    Ui::ScopeWindow *ui;

    ScopeWidget *m_scope;
};

#endif // SCOPEWINDOW_H
