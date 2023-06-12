#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QPixmap>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = 0);
    ~AboutDialog();

private slots:

private:
    Ui::AboutDialog *ui;
    QPixmap         *m_pixmap;
};

#endif // SOUNDCARDDIALOG_H
