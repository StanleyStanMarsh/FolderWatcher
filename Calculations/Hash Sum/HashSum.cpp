#include "HashSum.h"


QString HashSum::collectAllCheckSumsInFolder(QString folderPath, ALG_ID hashAlgorithm, QString hashString)
{

    WIN32_FIND_DATA findFileData; //информацция о найденных файлах

    HANDLE hFind = FindFirstFile((folderPath + "\\*").toStdWString().c_str(), &findFileData); //дескриптор 1 найденного файла, * - найти любой файл

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return QString("ОШИБКА");
    }

    do
    {

        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) //если это не подпапка
        {
            QString filePath = folderPath + QString("\\").append(QString::fromWCharArray(findFileData.cFileName)); //cобираем путь до файла
            QString fileCheckSum = calculateFileCheckSum(filePath, hashAlgorithm); //получаем кс файла
            hashString += fileCheckSum; //присоединяем к КС остльных файлов

        }
        else if (QString::fromWCharArray(findFileData.cFileName) != QString::fromWCharArray(L".")
                 && QString::fromWCharArray(findFileData.cFileName) != QString::fromWCharArray(L"..")) //если найдена подпапка, не являющаяся исходной или родительской
        {
            QString subFolderPath = folderPath + QString("\\\\") + QString::fromWCharArray(findFileData.cFileName); //путь к подпапке
            collectAllCheckSumsInFolder(subFolderPath, hashAlgorithm, hashString); //рекурсивный вызов
        }
    } while (FindNextFile(hFind, &findFileData)); //обход по всем подпапкам/файлам папки

    FindClose(hFind); //уничтожаем дескриптор обхода
    return hashString; //получившаяся строка КС файлов

}

QString HashSum::calculateFolderCheckSum(QString folderPath, ALG_ID hashAlgorithm, QString hashString)
{
    // QDir::currentPath() - текущая директория в которой лежит исполняемый файл (FolderWatcher.exe)
    HANDLE checksumsFile = CreateFile((QDir::currentPath().replace('/', "\\\\") + QString("\\checksum.txt")).toStdWString().c_str(), GENERIC_WRITE, 0, NULL,
                                      OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); //дескриптор файла, куда загрузим все КС

    if (checksumsFile == INVALID_HANDLE_VALUE)
    {
        // Ошибка Не удалось создать файл контрольных сумм!
        emit errorOccured(HashSumErrors::MakeHashSumFileError, folderPath);
        return QString("");
    }

    DWORD bytesWritten; //буфер с количеством вписанных уже байтов
    WriteFile(checksumsFile, hashString.utf16(), static_cast<DWORD>(hashString.size()), &bytesWritten, NULL); //вписываем строку КС в этот файл
    CloseHandle(checksumsFile); //уничтожаем дескриптор файла

    QString folderCheckSum = calculateFileCheckSum(QDir::currentPath().replace('/', "\\\\") + QString("\\checksum.txt"), hashAlgorithm); //получаем КС папки

    if (!DeleteFile((QDir::currentPath().replace('/', "\\\\") + QString("\\checksum.txt")).toStdWString().c_str())) { //удаляем за собой врмененный файл
        // Ошибка Не удалось удалить файл контрольных сумм!");
        emit errorOccured(HashSumErrors::DeleteHashSumFileError, folderPath);
    }
    return folderCheckSum; //КС папки

}

QString HashSum::calculateFileCheckSum(QString filePath, const ALG_ID &hashAlgorithm)
{
    QString hashString;

    if (!filePath.isEmpty())
    {
        LPCWSTR filePathWin = reinterpret_cast<LPCWSTR>(filePath.utf16());
        //преобразование QString к типу данных WinApi LPCWSTR - аналог const char*

        HANDLE hFile = CreateFile(filePathWin, GENERIC_READ,
                                  FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        //открытие файла для чтения через его дескриптор (handle) - cпец структуру Win API

        if (hFile != INVALID_HANDLE_VALUE)
        {
            HCRYPTPROV hCryptProv; //переменная для хранения криптопровайдера
            HCRYPTHASH hHash; //здесь хранится сам хэш


            if (CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))

            {
                if (CryptCreateHash(hCryptProv, hashAlgorithm, 0, 0, &hHash)) //создали хэш-объект
                {
                    BYTE buffer[4096];
                    DWORD bytesRead; //double word - аналог unsigned int32

                    DWORD hashLen = 64; //cколько займет хэш, зависит от алгоритма хэширования, SHA_256 генерирует 32байтный кэш


                    while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0)
                    //чтение данных из файла в буфер + обновляем хэш итеративно
                    {
                        CryptHashData(hHash, buffer, bytesRead, 0);
                    }
                    BYTE hashBuffer[64]; //для MD5 - 16, SHA_256 - 32, SHA_512 - 64
                    if (CryptGetHashParam(hHash, HP_HASHVAL, hashBuffer, &hashLen, 0)) //получаем cаму контрольную сумму
                    {

                        for (unsigned int i = 0; i < hashLen; i++)
                        {
                            hashString.append(QString::number(static_cast<int>(buffer[i]), 16).rightJustified(2, '0')); //hex
                        }
                    }
                    else
                    {
                        // Ошибка Не удалось получить контрольную сумму файла.
                        emit errorOccured(HashSumErrors::GetHashSumError, filePath);
                    }

                    CryptDestroyHash(hHash);//уничтожаем хэш-объект
                }
                else
                {
                    // Ошибка Не удалось создать хэш.
                    emit errorOccured(HashSumErrors::CreateHashError, filePath);
                }

                CryptReleaseContext(hCryptProv, 0); //освобождаем дескриптор криптопровайдера
            }
            else
            {
                // Ошибка Не удалось получить доступ к криптопровайдеру.
                emit errorOccured(HashSumErrors::ProviderAccessError, filePath);
            }

            CloseHandle(hFile); //уничтожаем дескриптор
        }
        else
        {
            // Ошибка Не удалось открыть файл для чтения.
            emit errorOccured(HashSumErrors::OpenFileError, filePath);
        }
    }

    return hashString;
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
            folder_path.replace('/', "\\\\");
            QString hash_string = this->collectAllCheckSumsInFolder(folder_path, hashAlgorithm, ""); //или CALG_SHA_512, CALG_MD5, ...
            QString folder_check_sum = this->calculateFolderCheckSum(folder_path, hashAlgorithm, hash_string);
            hash_sum = folder_check_sum;
        }
        else
            hash_sum = this->calculateFileCheckSum(dir->filePath(item), hashAlgorithm);
        vec_rows.push_back(QPair<QString, QString>(name, hash_sum));
    }
    emit hashSumsReady(vec_rows, QString::number((double)timer.elapsed() / 1000., 'f', 3));
}
