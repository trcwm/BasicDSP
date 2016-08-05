#ifndef SCOPEWINDOW_H
#define SCOPEWINDOW_H

#include <QDialog>
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

    void submit256Samples(float *buffer);

private:
    Ui::ScopeWindow *ui;

    ScopeWidget *m_scope;
};

#endif // SCOPEWINDOW_H
