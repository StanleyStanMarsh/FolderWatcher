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

class RealTimeWatcher : public QObject
{
    Q_OBJECT
    QFileSystemModel *dir;
public:
    explicit RealTimeWatcher(QFileSystemModel *_dir, QObject *parent = nullptr);

public slots:
    // void changeDir(const QString &_dir_path);
    void watch();

signals:

};

#endif // REALTIMEWATCHER_H
