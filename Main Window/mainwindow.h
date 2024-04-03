#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <QMainWindow>
#include <QSplitter>
#include <QListView>
#include <QTreeView>
#include <QDir>
#include <QFileSystemModel>
#include <QStorageInfo>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QStack>
#include <QVector>
#include <QThread>
#include <QPair>
#include <experimental/filesystem>
#include <string>

#include "../Calculations/Hash Sum/HashSum.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace fsys = std::experimental::filesystem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

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
    void on_info_message_triggered();
    // Слот для перехода в корень выбранного локального хранилища
    void goToStorage(const QString &storage_path);
    // Слот для вызова сигнала returnHashSum :)))
    void calcFileHashSumTriggered();

public slots:
    // Слот для принятия результатов подсчета контрольных сумм
    void handleHashSumCalculations(QPair<HashSumRow, QString> result_pair);

signals:
    // Сигнал, который отправляет список при нажатии на ui->calc_file_hash_sum
    void returnHashSum(QPair<QModelIndexList, QFileSystemModel&> selected_files);

private:
    // Функция для получения размера директории
    void getFoldersizeIterative(std::wstring rootFolder, unsigned long long &f_size);
    // Функция задержки, n - время в мс
    void delay(int n);
    // Функция возвращающая размер в максимальной единице измерения (bytes -> Kbytes -> Mbytes -> Gbytes)
    QString getMinimizedFormSize(double &f_size);

    Ui::MainWindow *ui;

    // Модель для взаимодействия с директориями
    QFileSystemModel *dir;
    // модель работы с информацией о директории
    QStandardItemModel *info;

    // Отдельный поток для контрольных сумм
    QThread hash_sum_thread;
};
#endif // MAINWINDOW_H
