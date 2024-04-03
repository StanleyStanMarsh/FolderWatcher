#ifndef HASHSUM_H
#define HASHSUM_H

#include <QWidget>
#include <QPushButton>
#include <windows.h>
#include <string>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>

class HashSum
{

    QWidget *parent;
public:
    HashSum(QWidget *_parent) { parent = _parent; }
    QString calculateFileCheckSum(QString filePath, ALG_ID hashAlgorithm);
    QString collectAllCheckSumsInFolder(QString folderPath, ALG_ID hashAlgorithm, QString hashString);
    QString calculateFolderCheckSum(QString folderPath, ALG_ID hashAlgorithm, QString hashString);
};
#endif // HASHSUM_H
