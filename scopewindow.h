#ifndef SCOPEWINDOW_H
#define SCOPEWINDOW_H

#include <QDialog>

namespace Ui {
class ScopeWindow;
}

class ScopeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ScopeWindow(QWidget *parent = 0);
    ~ScopeWindow();

private:
    Ui::ScopeWindow *ui;
};

#endif // SCOPEWINDOW_H
