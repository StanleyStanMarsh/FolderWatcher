#include "LoadingWindow.h"

LoadingWindow::LoadingWindow(QWidget *parent) : QMessageBox(parent) {
    this->setStyleSheet("color: rgb(0, 0, 0)");
    this->setText("Молчать! Идет подсчет КОНТРОЛЬНЫХ СУММ!");
    this->setStandardButtons(QMessageBox::NoButton);
}

void LoadingWindow::closeEvent(QCloseEvent *event) {
    event->ignore();
}
