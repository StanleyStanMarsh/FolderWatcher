#include "HashSum.h"


QString HashSum::collectAllCheckSumsInFolder(QString folderPath, ALG_ID hashAlgorithm, QString hashString)
{
    WIN32_FIND_DATA findFileData; //информацция о найденных файлах

    HANDLE hFind = FindFirstFile((folderPath + "/*").toStdWString().c_str(), &findFileData); //дескриптор 1 найденного файла, * - найти любой файл

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return QString("ОШИБКА");
    }

    do
    {

        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) //если это не подпапка
        {
            QString filePath = folderPath + QString("/").append(QString::fromWCharArray(findFileData.cFileName)); //cобираем путь до файла
            QString fileCheckSum = calculateFileCheckSum(filePath, hashAlgorithm); //получаем кс файла
            hashString += fileCheckSum; //присоединяем к КС остльных файлов

        }
        else if (QString::fromWCharArray(findFileData.cFileName) != QString::fromWCharArray(L".")
                 && QString::fromWCharArray(findFileData.cFileName) != QString::fromWCharArray(L"..")) //если найдена подпапка, не являющаяся исходной или родительской

        {
            QString subFolderPath = folderPath + QString("/") + QString::fromWCharArray(findFileData.cFileName); //путь к подпапке
            collectAllCheckSumsInFolder(subFolderPath, hashAlgorithm, hashString); //рекурсивный вызов
        }
    } while (FindNextFile(hFind, &findFileData)); //обход по всем подпапкам/файлам папки

    FindClose(hFind); //уничтожаем дескриптор обхода
    return hashString; //получившаяся строка КС файлов

}

QString HashSum::calculateFileCheckSum(QString filePath, const ALG_ID &hashAlgorithm)
{
    QString hashString = ""; //будущая контрольная сумма

    if (!filePath.isEmpty()) //если путь к файлу не пустой
    {
        QByteArray fileContents; //считанные данные из текстового потока файла

        QFile file(filePath); //открываем файл

        if (file.open(QIODevice::ReadOnly)) //открываем для чтения
        {
            while (!file.atEnd())
            {
            fileContents = file.read(4096); //вписываем все в строку в массив байтов
            }
            file.close();
        }
        else
        {
            qDebug() << "ошибка открытия файла";
        }

       hashString = calculateStringHash(fileContents.toHex(), hashAlgorithm); //получаем контрольную сумму от длинной строки fileContents
    }
    qDebug() << hashString << filePath;
    return hashString;
}

QString HashSum::calculateStringHash(QString hashString, ALG_ID hashAlgorithm)
{
    // Переменные для хранения криптопровайдера и хэша
    HCRYPTPROV hCryptProv = NULL;
    HCRYPTHASH hHash = NULL;

    // Пытаемся получить криптопровайдер
    if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        //Ошибка при получении криптопровайдера.;
        emit errorOccured(HashSumErrors::ProviderAccessError, "");
        return QString("");
    }

    // Пытаемся создать хэш-объект
    if (!CryptCreateHash(hCryptProv, hashAlgorithm, 0, 0, &hHash)) {
        // Ошибка Не удалось создать хэш.
        emit errorOccured(HashSumErrors::CreateHashError, "");
        CryptReleaseContext(hCryptProv, 0);
        return QString("");
    }

    // Пытаемся обновить хэш данными
    if (!CryptHashData(hHash, (BYTE*)hashString.toStdWString().c_str(), hashString.length(), 0)) {
        //"Ошибка при обновлении хэша данными.";
        CryptDestroyHash(hHash);
        CryptReleaseContext(hCryptProv, 0);
        return QString("");
    }

    // Получаем размер хэша
    DWORD cbHashSize;
    DWORD dwDataLen = sizeof(DWORD);
    if (!CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&cbHashSize, &dwDataLen, 0)) {
        //Ошибка при получении размера хэша;
        emit errorOccured(HashSumErrors::GetHashSumError, "");
        CryptDestroyHash(hHash);
        CryptReleaseContext(hCryptProv, 0);
        return QString("");
    }

    // Получаем сам хэш
    QByteArray hashOutput(cbHashSize, 0);
    if (!CryptGetHashParam(hHash, HP_HASHVAL, reinterpret_cast<BYTE*>(hashOutput.data()), &cbHashSize, 0)) {
        //Ошибка при получении хэша;
        emit errorOccured(HashSumErrors::GetHashSumError, "");
        CryptDestroyHash(hHash);
        CryptReleaseContext(hCryptProv, 0);
        return QString("");
    }

    // Очищаем хэш-объект и освобождаем криптопровайдер
    CryptDestroyHash(hHash);
    CryptReleaseContext(hCryptProv, 0);

    // Возвращаем хэш-строку
    return QString::fromLatin1(hashOutput.toHex());

}

void HashSum::getHashSums(const QModelIndexList &selected_files, const QFileSystemModel *dir_info,
                          const ALG_ID &hashAlgorithm) {
    QModelIndexList index_list(selected_files);
    const QFileSystemModel *dir = dir_info;
    // Таймер для подсчета времени вычислений
    QElapsedTimer timer;
    timer.start();

    HashSumRow vec_rows;
    // Заполнение строки HashSumRow значениями КС для выделенных папок и файлов
    for (auto item : index_list) {
        QString name;
        QString hash_sum;
        name = dir->fileName(item);
        if (dir->isDir(item)) {
            QString folder_path = dir->filePath(item);

            /* Уходит в бесконечную загрузку, без проверки все работает
            QDir directory(folder_path);
            if (directory.exists() && directory.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count() == 0)
            {
                qDebug() << "Пустая папка!";
                return;
            }*/


            //folder_path.replace('/', "\\\\");
            QString hash_string = this->collectAllCheckSumsInFolder(folder_path, hashAlgorithm, ""); //или CALG_SHA_512, CALG_MD5, ...
            if (hash_string == "")
            {
                emit errorOccured(HashSumErrors::OpenFileError, folder_path); //если нет разрешения на открытие файла или возникли ошибки

            }
            QString folder_check_sum = this->calculateStringHash(hash_string, hashAlgorithm);
            hash_sum = folder_check_sum;
        }
        else
            hash_sum = this->calculateFileCheckSum(dir->filePath(item), hashAlgorithm);
        vec_rows.push_back(QPair<QString, QString>(name, hash_sum));
    }
    emit hashSumsReady(vec_rows, QString::number((double)timer.elapsed() / 1000., 'f', 3));
}



