#ifndef LOADINGWINDOW_H
#define LOADINGWINDOW_H

#include <QMessageBox>
#include <QWidget>
#include <QLabel>
#include <QFormLayout>
#include <QBoxLayout>
#include <QCloseEvent>


class LoadingWindow : public QMessageBox
{
    Q_OBJECT

public:
    LoadingWindow(QWidget *parent = nullptr);
    void closeEvent(QCloseEvent *event) override;
};

#endif // LOADINGWINDOW_H
