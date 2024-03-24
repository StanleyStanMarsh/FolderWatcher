#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QListView>
#include <QTreeView>
#include <QDir>
#include <QFileSystemModel>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    // Модель для взаимодействия с директориями
    QFileSystemModel *dir;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Слот для перехода к выделенной папке
    void goDownDir(const QModelIndex &index);
    // Слот для возврата на одну директорию выше
    void goUpDir();
    // Слот для вывода основной инфы о текущей директории в таблице
    void showMainInfo();
    // Слот для открытия информационного сообщения
    void on_action_8_triggered();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
