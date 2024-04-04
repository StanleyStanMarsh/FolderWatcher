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

// Переименовываем вектор пар <название, контрольная сумма>
using HashSumRow = QVector<QPair<QString, QString>>;

enum HashSumErrors {
    MakeHashSumFileError,
    DeleteHashSumFileError,
    GetHashSumError,
    CreateHashError,
    ProviderAccessError,
    OpenFileError
};

class HashSum : public QObject
{
    Q_OBJECT
    QWidget *parent;
    QMessageBox *warning;
public:
    HashSum(QWidget *_parent) { parent = _parent; }
    QString calculateFileCheckSum(QString filePath, const ALG_ID &hashAlgorithm);
    QString collectAllCheckSumsInFolder(QString folderPath, ALG_ID hashAlgorithm, QString hashString);
    QString calculateFolderCheckSum(QString folderPath, ALG_ID hashAlgorithm, QString hashString);

public slots:
    // Слот для подсчета КС
    void getHashSums(const QModelIndexList& selected_files, const QFileSystemModel *dir_info,
                     const ALG_ID &hashAlgorithm);

signals:
    // Сигнал о завершении подсчета КС, передает КС и время вычисления
    void hashSumsReady(const HashSumRow &vec_rows, const QString &elapsed_time);
    // Сигнал об ошибке
    void errorOccured(const HashSumErrors &error, const QString &file_path);
};
#endif // HASHSUM_H
