#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QDateTime>
#include "../Calculations/Hash Sum/HashSum.h"

class Logger : public QObject
{
    Q_OBJECT
    [[maybe_unused]] QObject *parent;

public:
    explicit Logger(QObject *_parent) { parent = _parent; }

public slots:
    static void logHashSumToFile(const HashSumErrors &error, const QString &file_path);
    static void logExceptionToFile(const std::exception &e, const QString &file_path);

};

#endif // LOGGER_H
