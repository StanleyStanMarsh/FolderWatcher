#include "CompareWindow.h"
#include "ui_CompareWindow.h"

CompareWindow::CompareWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::CompareWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("Compare snapshots");

}

void CompareWindow::closeEvent(QCloseEvent *event) {
    emit closed();
    event->accept();
}
