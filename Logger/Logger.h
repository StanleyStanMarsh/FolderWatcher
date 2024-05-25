#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QDateTime>
#include <QSqlError>
#include "../Calculations/Hash Sum/HashSum.h"

/**
 * @brief The Logger class
 *
 * Logger является классом для логирования различных
 * типов ошибок и исключений в файлы. Этот класс
 * основан на QObject.
 */
class Logger : public QObject
{
    Q_OBJECT
    [[maybe_unused]] QObject *parent;

public:
    explicit Logger(QObject *_parent) { parent = _parent; }

public slots:
    /**
     * Статический слот для ведения лога, который записывает ошибки, связанные
     * с подсчетом контрольных сумм. Ошибки могут включать неудачу при создании
     * файла контрольных сумм, удалении файла, получении контрольной суммы,
     * создании хеша, доступе к криптопровайдеру и открытие файла для чтения.
     * Все эти ситуации обрабатываются в конструкции switch. Функция записывает
     * информацию об ошибке вместе с временной меткой и путём файла.
     *
     * @param error тип ошибки (#HashSumErrors) при подсчете контрольных сумм.
     * @param file_path путь к файлу, с которым связана ошибка.
     */
    static void logHashSumToFile(const HashSumErrors &error, const QString &file_path);

    /**
     * Статический слот для ведения лога, который записывает ошибки, связанные
     * с любыми исключениями std. Функция записывает информацию об
     * исключении вместе с временной меткой и путём файла, который
     * может быть связан с этим исключением.
     *
     * @param e исключение из std.
     * @param file_path путь к файлу, с которым связана ошибка.
     */
    static void logExceptionToFile(const std::exception &e, const QString &file_path);

    /**
     * Статический слот для ведения лога, который записывает ошибки SQL,
     * возникшие при работе с базой данных через Qt SQL модуль. Функция записывает
     * информацию об исключении вместе с временной меткой.
     *
     * @param e ошибка при работе с [SQL](https://doc.qt.io/qt-6/qsqlerror.html).
     */
    static void logSqlErrorToFile(const QSqlError &e);

    /**
     * Отправляет ошибку SQL в лог
     *
     * @param error возникшая ошибка
     */
    static void checkSqlError(const QSqlError &error);

};

#endif // LOGGER_H
