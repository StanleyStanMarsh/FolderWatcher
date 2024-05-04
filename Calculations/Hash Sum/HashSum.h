#ifndef HASHSUM_H
#define HASHSUM_H

#include <QWidget>
#include <QPushButton>
#include <windows.h>
#include <string>
#include <QDebug>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QFileSystemModel>
#include <QPair>

// #pragma comment(lib, "Advapi32.lib")
// #pragma comment(lib, "Crypt32.lib")

/// Переименовываем вектор пар <имя, хеш-сумма>
using HashSumRow = QVector<QPair<QString, QString>>;

/**
 * Перечисление для обозначения ошибок
 */
enum HashSumErrors {
    /// Не удалось создать файл хэш суммы
    MakeHashSumFileError,
    /// Не удалось удалить файл хэш суммы
    DeleteHashSumFileError,
    /// Не удалось получить хэш сумму
    GetHashSumError,
    /// Не удалось создать хэш
    CreateHashError,
    /// Не удалось получить доступ к провайдеру
    ProviderAccessError,
    /// Не удалось открыть файл или папку
    OpenFileError
};

/**
 * Класс для вычисления контрольных сумм
 *
 * В классе реализованы методы для подсчета контрольных сумм папок и файлов, сигналы для возникающих ошибок
 *
 * @author [Черепнов Максим](https://github.com/aofitasotr)
 */
class HashSum : public QObject
{
    Q_OBJECT
    [[maybe_unused]] QWidget *parent;
    QMessageBox *warning;
    int m_tmp;
public:
    friend class Snapshot;
    HashSum(QWidget *_parent) { parent = _parent; }
    HashSum(){ m_tmp = 0; }

    /**
     * Метод для вычисления контрольной суммы файла
     *
     * Функция вычисляет контрольную сумму файла,
     * находящегося по пути filePath, используя алгоритм хеширования,
     * заданный параметром hashAlgorithm.
     *
     * @param filePath Путь к файлу, для которого необходимо вычислить контрольную сумму
     * @param hashAlgorithm Константная ссылка на идентификатор алгоритма
     * хеширования, используемого для генерации контрольной суммы
     * @return Строковое представление контрольной суммы в шестнадцатеричном формате QString.
     * Если процесс не удался, возвращается пустая строка.
     * @note Функция может генерировать следующие ошибки:
     * - {@link HashSumErrors#MakeHashSumFileError}: Не удалось открыть указанный файл для чтения.
     * - {@link HashSumErrors#ProviderAccessError}: Не удалось получить доступ к криптопровайдеру.
     * - {@link HashSumErrors#CreateHashError}: Не удалось создать объект хеша для проведения вычислений.
     * - {@link HashSumErrors#GetHashSumError}: Не удалось получить контрольную сумму файла.
     */
    QString calculateFileCheckSum(QString filePath, const ALG_ID &hashAlgorithm);

    /**
     * Метод для сбора контрольных сумм всех файлов
     *
     * Функция рекурсивно обходит все файлы в указанной папке folderPath
     * и вычисляет контрольные суммы каждого файла, используя заданный
     * алгоритм hashAlgorithm. Конкатенирует все полученные контрольные суммы в одну
     * строку hashString, которая и возвращается в качестве результата.
     *
     * @param folderPath Путь к папке, в которой необходимо вычислить контрольные суммы файлов
     * @param hashAlgorithm Идентификатор алгоритма хеширования, который будет использоваться
     * для вычисления контрольных сумм.
     * @param hashString Строка, в которую накапливаются контрольные суммы (изначально может быть пустой).
     * @return Строка QString, содержащая накопленные контрольные суммы всех файлов в папке.
     * Если функция наткнётся на какую-либо ошибку, вернётся строка "ОШИБКА".
     */
    QString collectAllCheckSumsInFolder(QString folderPath, ALG_ID hashAlgorithm, QString hashString);

    /**
     * ...
     *
     * @param hashString Строка, для которой необходимо вычислить хеш-сумму.
     * @param hashAlgorithm Идентификатор алгоритма хеширования из перечисления, используемого cryptography API Windows
     * @return Строка QString, содержащая хеш-сумму в шестнадцатеричном представлении.
     * В случае возникновения ошибки во время процесса хеширования
     * возвращается пустая строка QString("").
     *
     * @note Функция может генерировать следующие ошибки:
     * - {@link HashSumErrors#ProviderAccessError}: Не удалось получить доступ к криптопровайдеру.
     * - {@link HashSumErrors#GetHashSumError}: Не удалось получить контрольную сумму файла.
     */
    QString calculateStringHash(QString hashString, ALG_ID hashAlgorithm);

public slots:
    /**
     * Слот для подсчета котрольных сумм выделенных файлов и папок
     *
     * Функция используется для вычисления контрольных сумм для списка файлов
     * и папок, предоставленных в QModelIndexList. Функция направляет
     * вычисленные контрольные суммы и имя каждого файла или папки в vec_rows,
     * который представляет собой вектор пар <имя, хеш-сумма>. После завершения
     * работы функция эмитирует сигнал hashSumsReady, передавая вычисленные данные
     * и время выполнения в секундах.
     *
     * @param selected_files Список индексов выбранных файлов и папок для вычисления хеш-сумм
     * @param dir_info Указатель на информацию файловой системы, используемый для получения
     * полного пути и других атрибутов файлов
     * @param hashAlgorithm Константная ссылка на идентификатор алгоритма хеширования,
     * который будет использован для вычисления hash-сумм.
     */
    void getHashSums(const QModelIndexList& selected_files, const QFileSystemModel *dir_info,
                     const ALG_ID &hashAlgorithm);

signals:
    /**
     * Сигнал о завершении подсчета контрольной суммы
     *
     * @param vec_rows Вектор пар <имя файла или папки, хеш-сумма>
     * @param elapsed_time Время, затраченное на вычисление контрольных сумм, выраженное в секундах
     */
    void hashSumsReady(const HashSumRow &vec_rows, const QString &elapsed_time);

    /**
     * Сигнал об ошибке
     *
     * @param error Перечисление {@link HashSumErrors}, указывающее на тип возникшей ошибки
     * @param file_path Путь к файлу или папке, для которого или которой произошла ошибка
     */
    void errorOccured(const HashSumErrors &error, const QString &file_path);
};
#endif // HASHSUM_H
