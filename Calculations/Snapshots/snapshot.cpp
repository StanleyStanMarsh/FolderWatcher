#include "snapshot.h"


Snapshot::Snapshot()
{

}

void Snapshot::calculate(const QString dir_path, const QString file_name,
                         const ALG_ID hash_algorithm, const QDateTime current_time)
{
    QString new_dir_path(dir_path);
    new_dir_path.replace("/","\\\\"); // адаптируем путь до диреткории
    m_dir_path = new_dir_path;
    QFileInfo info(m_dir_path);
    m_size = 0;
    m_name = info.fileName();
    m_last_change_date = info.lastModified().toString("hh:mm:ss.zzz dd.MM.yyyy");
    m_hash_algorithm = hash_algorithm;
    //создали типы данных необходимые для расчета всех внутренних файлов
    QVariantList inner_files;
    QJsonArray result;
    QString hash_sum;
    int counter = 0;
    // сбор внутренних файлов, расчет размера папок, контрольных сумм
    if(info.isDir())
    {
        m_is_dir = true;
        collectInnerFilesInDir(m_dir_path, inner_files, result, counter, hash_sum);
        m_inner_files = result;
        m_Hash_sum = m_hash.calculateStringHash(m_Hash_sum,  m_hash_algorithm);

    }
    // если наша директория не директория, а файл (шпион типа)
    else
    {
        m_is_dir = false;
        m_size = info.size();
        m_Hash_sum = m_hash.calculateStringHash(m_hash.calculateFileCheckSum(m_dir_path,  m_hash_algorithm), m_hash_algorithm);
    }

    this->writeToFile("./snapshots/" + file_name);
    emit snapshotReady(file_name, current_time);
}

void Snapshot::writeToFile(QString address)
{
    if(address.size()==0) address = "./"+ m_name;// если значения по умолчанию, записываем в текущую директорию
    address += ".json";
    // открыли файл
    QFile fileJson(address);
    fileJson.open(QIODevice::WriteOnly); // если файла нет, создаем
    //создаем QJsonObject, в который пишем информацию как в обычный словарь
    QJsonObject tmp;
    tmp.insert("name", m_name);
    tmp.insert("hash_sum",  m_Hash_sum);
    tmp.insert("last_changed_date", m_last_change_date);
    tmp.insert("size", m_size);
    // если наша директория не директория, а файл (шпион типа)
    if (m_is_dir) tmp.insert("inner_files", m_inner_files);
    //выкидываем в документ
    fileJson.write(QJsonDocument(tmp).toJson());
    qDebug() << "file has been written" << QFileInfo{fileJson}.absolutePath() << m_size;
    fileJson.close();
}



QJsonObject Snapshot::createSnapshotFile(QString file_path ,QJsonArray inner_files, int& counter, QString& hashsum/**/)
{
    QFileInfo info(file_path); // информация о файле/папке
    QJsonObject tmp;
    tmp.insert("name", info.fileName());
    tmp.insert("hash_sum",  m_hash.calculateStringHash(hashsum,  m_hash_algorithm));
    tmp.insert("last_changed_date", info.lastModified().toString("hh:mm:ss.zzz dd.MM.yyyy"));
    // если создаем снапшот папки, размер папки берем из counter, и вписываем внутренние файлы, содержащиеся в папке.
    // если папка пустая, в снапшот попадет пустой список внутренних файлов.
    if(info.isDir())
    {
        tmp.insert("size", counter);
        tmp.insert("inner_files", inner_files);
    }
    // если не папка, достаем размер из информации о файле
    else tmp.insert("size", info.size());
    return tmp;
}


QVariantList Snapshot::collectInnerFilesInDir(QString folder_path, QVariantList& inner_files, QJsonArray& result, int& counter, QString hashString)
{
    WIN32_FIND_DATA findFileData; //информацция о найденных файлах

    HANDLE hFind = FindFirstFile((folder_path + "\\*").toStdWString().c_str(), &findFileData); //дескриптор 1 найденного файла, * - найти любой файл
     //прописать ошибку в файле
//    if (hFind == INVALID_HANDLE_VALUE)
//    {
//       // return QString("ОШИБКА");
//    }
    do
    {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) //если это не подпапка
        {

            m_calls.push(1); // кладем в стек вызово 1, так как сейчас работаем с файлом
            QString file_path = folder_path + QString("\\").append(QString::fromWCharArray(findFileData.cFileName)); //cобираем путь до файла
            QString file_check_sum = m_hash.calculateFileCheckSum(file_path, m_hash_algorithm); // вычисляем кс файла
            hashString += file_check_sum; // сохраняем кс для текущей внутренней папки
            m_Hash_sum += hashString; // сохраняем кс для глобальной директории
            counter += QFileInfo {file_path}.size(); // сохраняем размер для текущей внутренней папки
            m_size += QFileInfo {file_path}.size(); // сохраняем размер для глобальной директории
            QJsonObject file_snapshot = createSnapshotFile(file_path ,QJsonArray::fromVariantList(inner_files), counter, hashString); //получаем JSON object файла
            inner_files.append(file_snapshot); // закидываем снапшот файла в список внутренних файлов для текущей папки
            m_calls.pop(); // файл обработан, выкидываем его из стека вызовов.
        }
        else if (QString::fromWCharArray(findFileData.cFileName) != QString::fromWCharArray(L".")
                 && QString::fromWCharArray(findFileData.cFileName) != QString::fromWCharArray(L"..")) //если найдена подпапка, не являющаяся исходной или родительской
        {
            if (m_calls.empty()) // если стек вызовов пуст, значит мы закончили сбор внутренних файлов для внутреннего файл, можно закидывать в результат
            {
                for (int i=0; i<inner_files.size(); i++)
                {
                    result.append(QJsonArray::fromVariantList(inner_files).at(i));
                }
                inner_files.clear();
                counter = 0;
                hashString.clear();
            }
            m_calls.push(2); // кладем 2, тк работаем с папкой
            QVariantList array; // создали пустой массив для сбора внутренних файлов внутренней папки
            QString sub_folder_path = folder_path + QString("\\\\") + QString::fromWCharArray(findFileData.cFileName); //путь к подпапке
            QJsonObject dir_snapshot = createSnapshotFile(sub_folder_path, QJsonArray::fromVariantList(collectInnerFilesInDir(sub_folder_path, array, result, counter
                                                                                                                              , hashString)), counter, hashString);
            //получили снапшот внутренней папки, закидываем его в список внутренних файлов
            inner_files.append(dir_snapshot);
            m_calls.pop(); // папка обработана, выкидываем ее из стека вызовов.
            if (m_calls.empty()) // если все папки обработаны, вписываем все в резульат
            {
                result.append(dir_snapshot);
                inner_files.clear();
             }
        }
    } while (FindNextFile(hFind, &findFileData)); //обход по всем подпапкам/файлам папки

    if (m_calls.empty())// если все папки обработаны, вписываем все в резульат
    {
        for (int i=0; i<inner_files.size(); i++)
        {
            result.append(QJsonArray::fromVariantList(inner_files).at(i));
        }

    }
    FindClose(hFind); //уничтожаем дескриптор обхода
    return inner_files;
}

