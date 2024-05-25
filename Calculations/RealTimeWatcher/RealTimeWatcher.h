#ifndef REALTIMEWATCHER_H
#define REALTIMEWATCHER_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QFileSystemModel>
// #include <iostream>
#include <string>
#include <windows.h>
#include <cassert>
#include <QTextBrowser>
#include <QScrollBar>

class RealTimeWatcher : public QObject
{
    Q_OBJECT
    /// Указатель на модель файловой системы
    QFileSystemModel *dir;
    /// Указатель на панель вывода
    QTextBrowser *out;
    /// Флаг открытого главного окна
    bool *opened;

public:
    /**
     * Конструктор, в котором инициализируются поля #dir и #out
     *
     * @param _dir указатель на исходную модель файловой системы.
     * @param _out указатель на исходную панель вывода.
     * @param parent родительский виджет.
     */
    explicit RealTimeWatcher(QFileSystemModel *_dir, QTextBrowser *_out, bool *_opened, QObject *parent = nullptr);

public slots:
    /**
     * Эта функция является частью класса RealTimeWatcher и предназначена для
     * мониторинга изменений в директории в режиме реального времени.
     * Она использует Windows API для отслеживания изменений файлов и
     * каталогов, а также выводит информацию о этих изменениях оформленную
     * в HTML формате.
     *
     */
    void watch();

};

#endif // REALTIMEWATCHER_H
