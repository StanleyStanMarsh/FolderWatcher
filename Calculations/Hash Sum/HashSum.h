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

class HashSum : public QObject
{
    Q_OBJECT
    QWidget *parent;
public:
    HashSum(QWidget *_parent) { parent = _parent; }
    QString calculateFileCheckSum(QString filePath, ALG_ID hashAlgorithm);
    QString collectAllCheckSumsInFolder(QString folderPath, ALG_ID hashAlgorithm, QString hashString);
    QString calculateFolderCheckSum(QString folderPath, ALG_ID hashAlgorithm, QString hashString);

public slots:
    // Слот для подсчета КС
    void getHashSums(QPair<QModelIndexList, QFileSystemModel&> selected_files);

signals:
    // Сигнал о завершении подсчета КС, передает КС и время вычисления
    void hashSumsReady(QPair<HashSumRow, QString> result_pair);
};
#endif // HASHSUM_H
