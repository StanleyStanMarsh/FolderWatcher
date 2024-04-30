#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <QVariantList>
#include <QFileInfo>
#include <QDate>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QVector>
#include <tuple>
#include <QStack>
#include <QFileSystemModel>
#include "../Hash Sum/HashSum.h"
class Snapshot : public QObject
{
    Q_OBJECT
    QWidget *parent;
    //
    QString m_last_change_date;
    QJsonArray m_inner_files;
    QStack<int> m_calls;
    HashSum m_hash;
    ALG_ID m_hash_algorithm;
    QString m_Hash_sum;
    int m_size;
    QString m_name;
    // NOTE: надо прикрутить эти функции к action в дизайнере в Инструменты->Сохранить снапшот директории
    QJsonObject createSnapshotFile (QString file_path ,QJsonArray inner_files, int& counter, QString& hashsum/**/);
    QVariantList collectInnerFilesInDir(QString folder_path, QVariantList& inner_files, QJsonArray& result, int& counter,
                                        QString hashString);
public:
    //Snapshot(){m_size = 0; flag =0; m_hash_algorithm =  CALG_SHA_256;};
    Snapshot(QString dir_path, ALG_ID hash_algorithm);
    void writeToFile(QString adress = "");
};

#endif // SNAPSHOT_H
