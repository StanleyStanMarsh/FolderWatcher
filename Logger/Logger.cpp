#include "Logger.h"

void Logger::logHashSumToFile(const HashSumErrors &error, const QString &file_path)
{
    QFile file("log.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) return;
    QTextStream out(&file);
    QString error_str;
    switch (error) {
    case HashSumErrors::MakeHashSumFileError:
        error_str = "Не удалось создать файл контрольных сумм!";
        break;
    case HashSumErrors::DeleteHashSumFileError:
        error_str = "Не удалось удалить файл контрольных сумм!";
        break;
    case HashSumErrors::GetHashSumError:
        error_str = "Не удалось получить контрольную сумму файла!";
        break;
    case HashSumErrors::CreateHashError:
        error_str = "Не удалось создать хэш!";
        break;
    case HashSumErrors::ProviderAccessError:
        error_str = "Не удалось получить доступ к криптопровайдеру!";
        break;
    case HashSumErrors::OpenFileError:
        error_str = "Не удалось открыть файл для чтения или получить доступ к файлам папки!";
        break;
    default:
        break;
    }
    out << QDateTime::currentDateTime().toString() << " " << file_path << ": " << error_str << '\n';
    file.close();
}

void Logger::logExceptionToFile(const std::exception &e, const QString &file_path)
{
    QFile file("log.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) return;
    QTextStream out(&file);
    QString error_str;
    out << QDateTime::currentDateTime().toString() << " " << file_path << ": " << e.what() << '\n';
    file.close();
}

void Logger::logSqlErrorToFile(const QSqlError &e) {
    QFile file("log.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) return;
    QTextStream out(&file);
    QString error_str;
    switch(e.type()) {
    case QSqlError::ConnectionError:
        error_str = "Ошибка подключения к базе данных.";
        break;
    case QSqlError::StatementError:
        error_str = "Ошибка в SQL-запросе.";
        break;
    case QSqlError::TransactionError:
        error_str = "Ошибка транзакции базы данных.";
        break;
    case QSqlError::UnknownError:
        error_str = "Неизвестная ошибка в базе данных.";
        break;
    default:
        break;
    }
    out << QDateTime::currentDateTime().toString() << error_str << '\n';
    file.close();
}

void Logger::checkSqlError(const QSqlError &error)
{
    if (error.type() != QSqlError::NoError) logSqlErrorToFile(error);
}
