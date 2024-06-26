#include "snapshot.h"


void Snapshot::calculate(const QString dir_path, const QString file_name,
                         const ALG_ID hash_algorithm, const QDateTime current_time)
{
    QString new_dir_path(dir_path);
    new_dir_path.replace("/","\\\\"); // адаптируем путь до диреткории
    m_dir_path = new_dir_path;
    QFileInfo info(m_dir_path);
    m_size =0;
    m_name = info.fileName();
    m_last_change_date = info.lastModified().toString("hh:mm:ss.zzz dd.MM.yyyy");
    m_hash_algorithm = hash_algorithm;
    //создали типы данных необходимые для расчета всех внутренних файлов
    QVariantList inner_files;
    QJsonArray result;
    QString hash_sum= "";
    int counter = 0;
    m_Hash_sum = "";
    // сбор внутренних файлов, расчет размера папок, контрольных сумм
    if(info.isDir())
    {
        m_is_dir = true;
        collectInnerFilesInDir(dir_path, inner_files, result, counter, hash_sum);
        m_inner_files = result;
        QString tmp =  m_Hash_sum;
        m_Hash_sum = m_hash.calculateStringHash(tmp,  m_hash_algorithm);
    }
    // если наша директория не директория, а файл (шпион типа)
    else
    {
        m_is_dir = false;
        m_size = info.size();
        m_Hash_sum = m_hash.calculateFileCheckSum(dir_path,  m_hash_algorithm);
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
    // qDebug() << "file has been written" << QFileInfo{fileJson}.absolutePath() << m_size;
    fileJson.close();
}



QJsonObject Snapshot::createSnapshotFile(QString file_path ,QJsonArray inner_files, int& counter, QString& hashsum/**/)
{
    QFileInfo info(file_path); // информация о файле/папке
    QJsonObject tmp;
    tmp.insert("name", info.fileName());
    //qDebug() << hashsum << info.fileName();
    tmp.insert("last_changed_date", info.lastModified().toString("hh:mm:ss.zzz dd.MM.yyyy"));
    tmp.insert("alt", checkForAltDS(file_path, m_hash_algorithm));
    // если создаем снапшот папки, размер папки берем из counter, и вписываем внутренние файлы, содержащиеся в папке.
    // если папка пустая, в снапшот попадет пустой список внутренних файлов.
    if(info.isDir())
    {
        //qDebug() << m_hash.calculateStringHash(hashsum,  m_hash_algorithm) << hashsum << info.fileName();
        tmp.insert("hash_sum",  m_hash.calculateStringHash(hashsum,  m_hash_algorithm));
        tmp.insert("size", counter);
        tmp.insert("inner_files", inner_files);
    }
    // если не папка, достаем размер из информации о файле
    else
    {
        tmp.insert("hash_sum", m_hash.calculateFileCheckSum(file_path, m_hash_algorithm));
        //qDebug()<< m_hash.calculateFileCheckSum(file_path, m_hash_algorithm) << file_path << 2;
        tmp.insert("size", info.size());
    }
    return tmp;
}




QVariantList Snapshot::collectInnerFilesInDir(QString folder_path, QVariantList& inner_files, QJsonArray& result, int& counter, QString& hashString)
{
    //qDebug() << m_Hash_sum;
    WIN32_FIND_DATA findFileData; //информацция о найденных файлах

    //HANDLE hFind = FindFirstFile((folder_path + "\\*").toStdWString().c_str(), &findFileData); //дескриптор 1 найденного файла, * - найти любой файл
     HANDLE hFind = FindFirstFile((folder_path + "/*").toStdWString().c_str(), &findFileData); //дескриптор 1 найденного файла, * - найти любой файл
     //прописать ошибку в файле
//    if (hFind == INVALID_HANDLE_VALUE)
//    {
//        qDebug() <<"Alert!";
//       // return QString("ОШИБКА");
//    }
    do
    {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) //если это не подпапка
        {
            try {
                m_calls.push(1); // кладем в стек вызово 1, так как сейчас работаем с файлом
                //QString file_path = folder_path + QString("\\").append(QString::fromWCharArray(findFileData.cFileName)); //cобираем путь до файла
                QString file_path = folder_path  + QString("/").append(QString::fromWCharArray(findFileData.cFileName)); //cобираем путь до файла
                QString file_check_sum = m_hash.calculateFileCheckSum(file_path, m_hash_algorithm); // вычисляем кс файла
                hashString += file_check_sum; // сохраняем кс для текущей внутренней папки
                // сохраняем кс для глобальной директории
                m_Hash_sum += file_check_sum;
                counter += QFileInfo {file_path}.size(); // сохраняем размер для текущей внутренней папки
                m_size += QFileInfo {file_path}.size(); // сохраняем размер для глобальной директории
                QJsonObject file_snapshot = createSnapshotFile(file_path ,QJsonArray::fromVariantList(inner_files), counter, hashString); //получаем JSON object файла
                inner_files.append(file_snapshot); // закидываем снапшот файла в список внутренних файлов для текущей папки
                m_calls.pop(); // файл обработан, выкидываем его из стека вызовов.

            } catch (std::exception &e) {
                emit errorOccured(e, folder_path);
            }

        }
        else if (QString::fromWCharArray(findFileData.cFileName) != QString::fromWCharArray(L".")
                 && QString::fromWCharArray(findFileData.cFileName) != QString::fromWCharArray(L"..")) //если найдена подпапка, не являющаяся исходной или родительской
        {
            try {
            if (m_calls.empty()) // если стек вызовов пуст, значит мы закончили сбор внутренних файлов для внутреннего файл, можно закидывать в результат
            {
                for (int i=0; i<inner_files.size(); i++)
                {
                    result.append(QJsonArray::fromVariantList(inner_files).at(i));
                }
                //qDebug() << hashString;
                inner_files.clear();
                counter = 0;
                hashString = "";
            }
            m_calls.push(2); // кладем 2, тк работаем с папкой
            QVariantList array; // создали пустой массив для сбора внутренних файлов внутренней папки
            //QString sub_folder_path = folder_path + QString("\\\\") + QString::fromWCharArray(findFileData.cFileName); //путь к подпапке
            int tmp =0; // также написать для кс
            QString tmp_string = "";
            QString sub_folder_path = folder_path + QString("/") + QString::fromWCharArray(findFileData.cFileName);
            QJsonObject dir_snapshot = createSnapshotFile(sub_folder_path, QJsonArray::fromVariantList(collectInnerFilesInDir(sub_folder_path, array, result, tmp
                                                                                                                              ,tmp_string)), tmp, tmp_string);
            //получили снапшот внутренней папки, закидываем его в список внутренних файлов
            inner_files.append(dir_snapshot);
            counter += tmp;
            tmp_string += hashString;
            m_calls.pop(); // папка обработана, выкидываем ее из стека вызовов.
            if (m_calls.empty()) // если все папки обработаны, вписываем все в резульат
            {
                result.append(dir_snapshot);
                inner_files.clear();
            }
            } catch (std::exception &e) {
                emit errorOccured(e, folder_path);
        }
    } }while (FindNextFile(hFind, &findFileData)); //обход по всем подпапкам/файлам папки

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

Snapshot::Snapshot(QString file_path)
{
    //открыли файл
    QFile fileJson(file_path);
    //только для чтения
    fileJson.open(QIODevice::ReadOnly);
    //считали все из файла в QJsonObject
    QJsonObject jsontmp = QJsonDocument::fromJson(fileJson.readAll()).object();
    //Закинули все значения в поля класса
    m_name = jsontmp["name"].toString();
    m_Hash_sum = jsontmp["hash_sum"].toString();
    m_last_change_date = jsontmp["last_changed_date"].toString();
    m_size = jsontmp["size"].toInt();
    m_inner_files = jsontmp["inner_files"].toArray();
}
QJsonArray Snapshot::checkForAltDS(QString filePath, ALG_ID hashAlgorithm)
{
      WIN32_FIND_STREAM_DATA findStreamData; //cтруктура с информацией о потоке

       LPCWSTR filePathWin = reinterpret_cast<LPCWSTR>(filePath.utf16()); //приводим путь к формату win api LPCWSTR

       bool hasAltDS = false; //флаг наличия у файла/директории потока, кроме главного (наличие альт потоков)

      HANDLE hFind = FindFirstStreamW(filePathWin, FindStreamInfoStandard, &findStreamData, 0);

     QJsonArray result;
      if (hFind != INVALID_HANDLE_VALUE) {

        do {
          QJsonObject obj;
          QString nameADS = QString::fromWCharArray(findStreamData.cStreamName); // получаем название альт потока
          if (nameADS != "::$DATA")
          {

              nameADS = nameADS.section(':', 1, 1).section('$', 0, 0); //возвращаемое имя потока имеет вид ":имя_потока:$DATA", данными функциями вырезаем имя_потока
              // qDebug() << "Имя альтернативного потока: " << nameADS;
              obj.insert("alt_name", nameADS);
              LARGE_INTEGER streamSize = findStreamData.StreamSize;
              // qDebug() << "Размер альтернативного потока: " << streamSize.QuadPart << " байт";
              obj.insert("alt_size", streamSize.QuadPart);
              // Вычисление контрольной суммы ADS
              QString hashSum = m_hash.calculateFileCheckSum(filePath + ':' + nameADS, hashAlgorithm);
              // qDebug() << "Контрольная сумма потока: " << hashSum;
              obj.insert("alt_hash_sum", hashSum);
              hasAltDS = true;
          }
          if(!obj.empty()) result.append(obj);

           } while (FindNextStreamW(hFind, &findStreamData));

        FindClose(hFind);
      }
      if (!hasAltDS) {
          return {};
        // qDebug() << "Альтернативные потоки не найдены.";
      }
      return result;
}

QVector<ComparisonAnswer> Snapshot::compareSnapshots(Snapshot &other)
{
    // результирующий вектор, содрежит в себе пару элементов: путь, до файла, который был изменен
       // и флаг изменения ( отредактирован, удален, создан)
       QVector<ComparisonAnswer> result;
       // составляем словрарик, в котором ключи - путь до файла/папки из директории
       // значения - контрольные суммы фалов/папок
       QMap<QString, QPair<QString, QMap<QString, QString>>> first_result, second_result;

       // создаем словарик всех файлов в двух директориях
       for(auto it = m_inner_files.begin(); it!=m_inner_files.end(); ++it)
       {
            createCompareMap(it->toObject(),m_name, first_result);
       }
       for(auto other_it = other.m_inner_files.begin(); other_it != other.m_inner_files.end(); ++other_it)
       {
            createCompareMap(other_it->toObject(),m_name, second_result);
       }
       //qDebug() << first_result;
       auto tlp = first_result.keys();
       auto key = tlp.begin();
       // сравнение старого снапшота с новым
       // qDebug() << "here";
       while(!first_result.empty())
           {
           // qDebug() << *key;
           // смотрим, различаются ли альт.потоки
               if (first_result[*key].second != second_result[*key].second)
                   {
                      QList<QString> kil = first_result[*key].second.keys();
                      QList<QString> s_kil = second_result[*key].second.keys();
                      // qDebug() << kil;
                      // qDebug() << s_kil;
                      if (kil.empty() and s_kil.empty())
                      {
                          first_result.remove(*key);
                          key++;
                          continue;
                      }

                      // qDebug() << *s_kil.begin();
                       QList<QString>::iterator alt_key;
                       QList<QString>::iterator alt_s_key;
                      //такой же обход как и для обычных файлов.
                      if (!first_result[*key].second.empty())
                      {
                          alt_key = kil.begin();
                      }
                      while(!first_result[*key].second.empty())
                      {
                          // qDebug() << "here 123";
                           // если альтпотока с таким же ключом как в исходном снапшоте нет в новой, значит он удален.
                          if(second_result[*key].second.find(*alt_key) == second_result[*key].second.end())
                          {
                              // qDebug() << "Роберт Полсон" << *alt_key;
                              result.push_back({*key, deleted, true, *alt_key});
                              first_result[*key].second.remove(*alt_key);
                              alt_key++;
                              continue;
                          }
                          // если контрольные суммы альтпотоков в одном адресе не равны, значит файл был именен
                          else if (first_result[*key].second[*alt_key] != second_result[*key].second[*alt_key])
                          {
                              result.push_back({*key,edited, true, *alt_key});
                          }
                          first_result[*key].second.remove(*alt_key);
                          second_result[*key].second.remove(*alt_key);
                          alt_key++;
                      }
                      // если во втором векторе остались значения, значит они новые.
                     // auto alt_s_key = second_result[*key].second.keys().begin();
                      if(!second_result[*key].second.empty()) alt_s_key = s_kil.begin();
                      while(!second_result[*key].second.empty())
                      {
                          result.push_back({*key, appeared, true, *alt_s_key});
                          second_result[*key].second.remove(*alt_s_key);
                          alt_s_key++;
                      }
                   }
           // если элемента с таким же ключом как в исходном снапшоте нет в новой, значит он удален.
           if(second_result.find(*key) == second_result.end())
           {
               result.push_back({*key, deleted,false, ""});
               first_result.remove(*key);
               key++;
               continue;
           }
           // если контрольные суммы файлов/папок в одном адресе не равны, значит файл был именен
           if (first_result[*key].first != second_result[*key].first) result.push_back({*key,edited, false, ""});
           first_result.remove(*key);
           second_result.remove(*key);
           key++;
       }
       // если после проходки по всем файлам первого снапшота, остались файлы во втором, значит
       // они были добавлены
       auto s_key1 = second_result.keys();
       auto s_key = s_key1.begin();
       while(!second_result.empty())
       {
           result.push_back({*s_key, appeared, false, ""});
           second_result.remove(*s_key);
           s_key++;
       }
       // for(int i =0; i< result.size(); i++)
       // {
       //     qDebug() << result.at(i).path << result.at(i).flag << result.at(i).isAlt << result.at(i).alt_name;
       // }
       // возвращаем вектор пар, хранящих отличия первой директории от второй.
       return result;
}

QMap<QString, QPair<QString, QMap<QString, QString>>> Snapshot::createCompareMap (QJsonObject obj, QString& parent_dir, QMap<QString, QPair<QString, QMap<QString, QString>>>& result)
{
    QMap<QString, QString> alts;
    for(int i =0; i< obj["alt"].toArray().size(); i++)
    {
        alts.insert(obj["alt"].toArray().at(i)["alt_name"].toString(),obj["alt"].toArray().at(i)["alt_hash_sum"].toString());
    }
    // если файлик, кидаем в result локальный путь до него, кс и инфу о альт потоках
    if(obj["inner_files"].toArray().empty())
    {
        result.insert(parent_dir + "\\"+ obj["name"].toString(), {obj["hash_sum"].toString(), alts});
    }
   // если папка
    else
    {
        // сохраняем путь до родителськой папки, чтобы вренуться полсе рекурсивного выозова
        QString tmp = parent_dir;
        tmp = parent_dir + "\\"+ obj["name"].toString();
        //дополянем путь до папки
        result.insert(tmp, {obj["hash_sum"].toString(), alts});
        for(int i =0; i< obj["inner_files"].toArray().size(); i++)
        {
            //рекурсивно обходим внутренние папки
            createCompareMap(obj["inner_files"].toArray().at(i).toObject(),tmp, result);
        }

     }
    return result;
}


QList<QPair<QString, QString>> Snapshot::externalFiles(){
    QList<QPair<QString, QString>> result;

    for (int i=0; i< m_inner_files.size(); i++) {
        QJsonValue value = m_inner_files.at(i);
        QJsonObject object = value.toObject();
        // Если это папка
        if (object.contains("inner_files")){
            result.append(QPair<QString, QString>(value["name"].toString(), "Папка"));
        }
        else {
            result.append(QPair<QString, QString>(value["name"].toString(), "Файл"));
        }
    }
    return result;
}
