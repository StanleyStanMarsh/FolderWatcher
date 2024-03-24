#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QListView>
#include <QTreeView>
#include <QDir>
#include <QFileSystemModel>

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
    // Слот для переходу к выделенной папке
    void goDownDir(const QModelIndex &index);
    // Слот для возврата на одну директорию выше
    void goUpDir();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
