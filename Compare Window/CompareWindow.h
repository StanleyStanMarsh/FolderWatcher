#ifndef COMPAREWINDOW_H
#define COMPAREWINDOW_H

#include <QWidget>
#include <QCloseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class CompareWindow; }
QT_END_NAMESPACE

class CompareWindow : public QWidget
{
    Q_OBJECT
public:
    explicit CompareWindow(QWidget *parent = nullptr);

    void closeEvent(QCloseEvent *event) override;

signals:
    void closed();

private:
    Ui::CompareWindow *ui;

};

#endif // COMPAREWINDOW_H
