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
    QFileSystemModel *dir;
    QTextBrowser *out;
public:
    explicit RealTimeWatcher(QFileSystemModel *_dir, QTextBrowser *_out, QObject *parent = nullptr);

public slots:
    // void changeDir(const QString &_dir_path);
    void watch();

signals:

};

#endif // REALTIMEWATCHER_H
