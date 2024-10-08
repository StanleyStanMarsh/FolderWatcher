#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    opened = new bool(true);

    this->setWindowTitle("Folder Watcher");

    // -------------------- Настройка модели работы с директориями ----------------
    dir = new QFileSystemModel;

    dir->setRootPath(QDir::currentPath());
    ui->listView->setModel(dir);
    ui->listView->setRootIndex(dir->index(QDir::currentPath()));
    // Отключаем горячие клавиши для listView
    filter = new ShortcutsEventFilter(this);
    ui->listView->installEventFilter(filter);

    ui->path_line->setText(QDir::currentPath());

    // --------------- Настройка отображения с инфой о директориях -------------
    info = new QStandardItemModel;
    ui->tableView->resizeColumnsToContents();
    QHeaderView *header = ui->tableView->horizontalHeader();
    header->setStretchLastSection(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setModel(info);

    // ----------------------- Хранилища ---------------------------
    // Ищем доступные локальные хранилища и добавляем их в комбо бокс
    QStringList storages_paths;
    foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
        if (storage.isValid() && storage.isReady()) {
            storages_paths << storage.rootPath();
        }
    }
    ui->storages_box->addItems(storages_paths);

    // Объект для вычислений КС, заносим вычисления в отдельный поток
    HashSum *calculator = new HashSum(this);
    calculator->moveToThread(&hash_sum_thread);

    // Заносим логгер в отдельный поток
    Logger *logger = new Logger(this);
    logger->moveToThread(&logger_thread);

    // Объект для формирования снапшотов
    Snapshot *snap = new Snapshot();
    snap->moveToThread(&snapshot_thread);

    // Создаем окно загрузки и скрываем его
    loading_window = new LoadingWindow();
    loading_window->hide();

    // Создаем отслеживатель
    RealTimeWatcher *rtw = new RealTimeWatcher(dir, ui->realTimeLog, opened);
    rtw->moveToThread(&rtw_thread);

    // Коннектим нажатие на кнопки подсчета КС к слоту-отправителю сигнала с данными о выделенных
    // файлах и папках
    connect(ui->actionSHA_256, &QAction::triggered, this, &MainWindow::chooseSHA_256);
    connect(ui->actionSHA_512, &QAction::triggered, this, &MainWindow::chooseSHA_512);
    connect(ui->actionMD5, &QAction::triggered, this, &MainWindow::chooseMD5);

    // ---------------------- HashSum -----------------------------------------
    // Коннектим завершение потока с планированием удаления "вычислителя" КС
    connect(&hash_sum_thread, &QThread::finished, calculator, &QObject::deleteLater);
    // Коннектим сигнал с данными о выделенных файлах и папках со слотом вычисления КС
    connect(this, &MainWindow::returnHashSum, calculator, &HashSum::getHashSums);
    // Коннектим сигнал о завершении вычисления КС с внесением полученных данных в таблицу
    connect(calculator, &HashSum::hashSumsReady, this, &MainWindow::handleHashSumCalculations);

    // ---------------------- Logger -----------------------------------------
    // Коннектим завершение потока с планированием удаления логгера
    connect(&logger_thread, &QThread::finished, logger, &QObject::deleteLater);
    // Коннектим сигналы о возникших ошибках с логом
    connect(calculator, &HashSum::errorOccured, logger, &Logger::logHashSumToFile);
    connect(this, &MainWindow::errorOccured, logger, &Logger::logExceptionToFile);
    connect(snap, &Snapshot::errorOccured, logger, &Logger::logExceptionToFile);
    connect(this, &MainWindow::errorSqlOccured, logger, &Logger::logSqlErrorToFile);

    // ---------------------- Snapshot -----------------------------------------
    // Коннектим завершение потока с планированием удаления снапшота
    connect(&snapshot_thread, &QThread::finished, snap, &QObject::deleteLater);
    // Коннектим сигнал с данными о выделенных файлах и папках со слотом вычисления КС
    connect(this, &MainWindow::returnSnapshot, snap, &Snapshot::calculate);
    // Коннектим сигнал о завершении вычисления КС с внесением полученных данных в таблицу
    connect(snap, &Snapshot::snapshotReady, this, &MainWindow::handleSnapshotCalculations);

    // // Коннектим сигнал о завершении вычисления КС с показом логов
    // connect(calculator, &HashSum::hashSumsReady, this, &MainWindow::showHashSumLogs);
    // // Коннектим сигнал об ошибках со сбором ошибок
    // connect(calculator, &HashSum::errorOccured, this, &MainWindow::handleHashSumErrors);


    // ---------------------- Real Time Watcher --------------------------------
    // Коннектим завершение потока с планированием удаления отслеживателя
    connect(&rtw_thread, &QThread::finished, rtw, &QObject::deleteLater);
    // Коннектим изменение корневого пути с изменением отслеживаемой директории
    // connect(this->dir, &QFileSystemModel::rootPathChanged, rtw, &RealTimeWatcher::changeDir);
    //
    connect(this, &MainWindow::showed, rtw, &RealTimeWatcher::watch);
    connect(dir, &QFileSystemModel::rootPathChanged, rtw, &RealTimeWatcher::watch);

    hash_sum_thread.start();

    logger_thread.start();

    snapshot_thread.start();

    rtw_thread.start();

    // ------------------- Кнопки, вьюшки и тд. -------------------------------------
    // Коннектим двойное нажатие по папке/файлу к его открытию
    connect(ui->listView, &QListView::doubleClicked, this, &MainWindow::goDownDir);
    // Коннектим нажатие по кнопке с поднятием на одну папку наверх
    connect(ui->back_button, &QPushButton::clicked, this, &MainWindow::goUpDir);
    // Коннектим нажатие по кнопке с отображением инфы
    connect(ui->main_info_button, &QPushButton::clicked, this, &MainWindow::showMainInfo);
    // Коннектим выбор доступного хранилища в комбо боксе с его открытием модели
    connect(ui->storages_box, &QComboBox::textActivated, this, &MainWindow::goToStorage);
    // Коннектим изменение корневого пути в модели с его отображением в строке
    connect(dir, &QFileSystemModel::rootPathChanged, ui->path_line, &QLineEdit::setText);

    // --------------------------- Настройка БД ------------------------------------
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("testDB.db");
    // Проверяем успешность открытия БД
    Logger::checkSqlError(db.lastError());

    // Если в БД нет таблицы с инфой о снапшотах, то создаем её
    query = new QSqlQuery(db);
    //query->exec("DROP TABLE IF EXISTS Snaps;");
    query->exec("CREATE TABLE IF NOT EXISTS Snaps(DirectoryPath TEXT, SnapshotPath TEXT, SaveDate DATE);");
    Logger::checkSqlError(query->lastError());
    // Задаем Qt модель для работы
    SQLmodel = new QSqlTableModel(this, db);
    Logger::checkSqlError(SQLmodel->lastError());
    SQLmodel->setTable("Snaps");
    Logger::checkSqlError(SQLmodel->lastError());
    SQLmodel->select();
    Logger::checkSqlError(SQLmodel->lastError());
    SQLmodel->submitAll();
    Logger::checkSqlError(SQLmodel->lastError());
    SQLmodel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    Logger::checkSqlError(SQLmodel->lastError());
    // ui->tableView->setModel(SQLmodel);

    // Создаем директорию под снапшоты
    createDirectory("./snapshots");

    // -------------------- Compare Window --------------------------------------
    // Окно сравнения
    compare_window = new CompareWindow(SQLmodel);
    // compare_window->moveToThread(&compare_window_thread);
    compare_window->hide();
    // Коннектим закрытие окна сравнения с открытием главного окна
    connect(compare_window, &CompareWindow::closed, this, &MainWindow::show);
}

void MainWindow::goDownDir(const QModelIndex &index) {
    // Назначаем новый корень для отображения
    ui->listView->setRootIndex(index);
    // Назначаем новый корень для самой модели
    dir->setRootPath(dir->filePath(index));
}

void MainWindow::goUpDir() {
    // Получаем адрес директории в которой сейчас находимся,
    // затем пермещаемся на директорию (cdUp()) выше и переназначаем корни
    // для модели и отображения
    QDir now_dir(dir->rootPath());
    now_dir.cdUp();
    ui->listView->setRootIndex(dir->index(now_dir.absolutePath()));
    dir->setRootPath(now_dir.absolutePath());
}

void MainWindow::goToStorage(const QString &storage_path) {
    // Назначаем новый корень для отображения
    ui->listView->setRootIndex(dir->index(storage_path));
    // Назначаем новый корень для самой модели
    dir->setRootPath(storage_path);
}

void MainWindow::showMainInfo() {
    //
    info->clear();

    // установка названий столбцов
    info->setColumnCount(5);
    info->setHorizontalHeaderLabels(QStringList{"name", "date", "type", "size", "alt"});

    // Получаем индекс текущей директории и количество файлов в директории
    QModelIndex currentDirIndex = dir->index(dir->rootPath());
    int fileCount = dir->rowCount(currentDirIndex);

    if (ui->dir_size_box->isChecked()) {
        ui->info_label->setText("Молчать! Идет подсчет размера директорий");
        ui->info_label->setStyleSheet("color: rgb(255, 165, 0)");
    }

    // Проходимся по всем файлам и печатаем информацию о них
    for (int i = 0; i < fileCount; i++) {
        QModelIndex fileIndex = dir->index(i, 0, currentDirIndex);

        // Получаем тип файла (расширение)
        QString fileType = (dir->isDir(fileIndex)) ? "Folder" : dir->fileInfo(fileIndex).completeSuffix();
        if (fileType == "") fileType = "File";

        // добавляем строку с инфой о файле
        QList<QStandardItem *> row;
        row << new QStandardItem(dir->fileName(fileIndex));
        row << new QStandardItem(dir->lastModified(fileIndex).toString("yyyy-MM-dd HH:mm:ss"));
        row << new QStandardItem(fileType);
        if (fileType != "Folder")
        {
            // Делаем читаемый размер
            double file_size = dir->size(fileIndex);
            row << new QStandardItem(getMinimizedFormSize(file_size));
        }
        else
        {
            // Если стоит галочка в чек боксе, считаем еще и размер папок
            if (ui->dir_size_box->isChecked()) {
                // Задержка для постепенной отрисовки строк
                delay(100);
                unsigned long long dir_size = 0;
                // Переделываем слеши в бэкслеши для понятного формата для функции
                std::wstring path_for_fsys = dir->filePath(fileIndex).replace('/', "\\\\").toStdWString();

                try {
                    getFoldersizeIterative(path_for_fsys, dir_size);
                    // Делаем читаемый размер
                    double d_dir_size = dir_size;
                    row << new QStandardItem(getMinimizedFormSize(d_dir_size));
                } catch(std::exception &e) {
                    // Вписываем сообщение об ошибке
                    row << new QStandardItem("ОШИБКА");
                    emit errorOccured(e, dir->filePath(fileIndex));
                }

            }
            // Если галочки нет - в таблице для папок размер не указывается
            else
                row << new QStandardItem("");
        }

        // Получаем названия альтернативных потоков
        QString alt = "Нет альт.потоков";
        alt = checkForAltDS(dir->filePath(fileIndex));
        row << new QStandardItem(alt);

        info->appendRow(row);
    }
    ui->info_label->setStyleSheet("color: rgb(0, 0, 0)");
    ui->info_label->setText("Информация");
}

MainWindow::~MainWindow()
{
    *opened = false;
    rtw_thread.quit();
    rtw_thread.wait();
    delete dir;
    delete info;
    delete ui;
    delete loading_window;
    delete filter;
    hash_sum_thread.quit();
    hash_sum_thread.wait();
    logger_thread.quit();
    logger_thread.wait();
    snapshot_thread.quit();
    snapshot_thread.wait();

    delete compare_window;
    delete SQLmodel;
    delete query;
    delete opened;
}

void MainWindow::on_info_message_triggered() const
{
    QMessageBox info_box;
    info_box.setWindowTitle("О программе...");
    info_box.setBaseSize(200, 100);
    info_box.setIcon(QMessageBox::Information);
    info_box.setText("<b>Программа FolderWatcher ver. 1.2 release</b><br>"
                     "<a href='http://www.trolltech.com'>GitHub проекта</a>");
    info_box.setInformativeText("<b>Разработчик ДИПМаксМакс</b>");
    info_box.exec();
}

void MainWindow::getFoldersizeIterative(std::wstring rootFolder, unsigned long long &f_size) {
    // Функция от пользователя Amit взята на
    // https://stackoverflow.com/questions/15495756/how-can-i-find-the-size-of-all-files-located-inside-a-folder
    // и переписана в итеративный вид
    // Стек для хранения вложенных путей
    QStack<std::wstring> folders;
    folders.push(rootFolder);

    while (!folders.empty()) {
        std::wstring current_folder = folders.top();
        folders.pop();

        fsys::path folderPath(current_folder);
        if (fsys::exists(folderPath)) {
            fsys::directory_iterator end_itr;
            for (fsys::directory_iterator dirIte(current_folder, fsys::directory_options::skip_permission_denied);
                 dirIte != end_itr; ++dirIte) {
                fsys::path filePath(dirIte->path());
                try {
                    if (!fsys::is_directory(dirIte->status())) {
                        f_size += fsys::file_size(filePath);
                    } else {
                        folders.push(filePath.wstring());
                    }
                } catch (std::exception &e) {
                    // записываем в лог ошибки
                    emit errorOccured(e, QString::fromStdString(filePath.string()));
                }
            }
        }
    }
}

void MainWindow::delay(int n) const
{
    QTime dieTime= QTime::currentTime().addMSecs(n);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

QString MainWindow::getMinimizedFormSize(double &f_size) {
    QString unit = "bytes";
    QVector<QString> units {"Kbytes", "Mbytes", "Gbytes"};
    int i = 0;
    // Делим пока можем на 1024 и меняем соответсвтенно приставку
    while (f_size > 1024 && i < units.size()) {
        f_size /= 1024.0;
        unit = units[i++];
    }
    if (i == 0)
        return QString::number(f_size, 'f', 0) + " " + unit;
    return QString::number(f_size, 'f', 2) + " " + unit;
}

void MainWindow::handleHashSumCalculations(const HashSumRow &vec_rows, const QString &elapsed_time)
{
    // Готовим таблицу для хэш сумм
    info->clear();
    info->setColumnCount(2);
    info->setHorizontalHeaderLabels(QStringList{"name", "hash sum"});

    // Заполняем таблицу данными полученными вычислителем КС
    for (auto item : vec_rows) {
        QList<QStandardItem *> row;
        row << new QStandardItem(item.first);
        row << new QStandardItem(item.second);
        info->appendRow(row);
    }

    ui->info_label->setStyleSheet("color: rgb(0, 0, 0)");
    ui->info_label->setText("Информация. Время вычисления " +
                            elapsed_time + " c.");
    // скрываем окно
    loading_window->hide();
}

void MainWindow::calcFileHashSumTriggered(const ALG_ID &hashAlgorithm) {
    // очищаем лог
    hash_sum_log = "";
    // открываем окно
    loading_window->show();
    emit returnHashSum(ui->listView->selectionModel()->selectedIndexes(), dir, hashAlgorithm);
}

void MainWindow::handleHashSumErrors(const HashSumErrors &error, const QString &file_path) {
    switch (error) {
    case HashSumErrors::MakeHashSumFileError:
        hash_sum_log += file_path + ": Не удалось создать файл контрольных сумм!\n";
        break;
    case HashSumErrors::DeleteHashSumFileError:
        hash_sum_log += file_path + ": Не удалось удалить файл контрольных сумм!\n";
        break;
    case HashSumErrors::GetHashSumError:
        hash_sum_log += file_path + ": Не удалось получить контрольную сумму файла!\n";
        break;
    case HashSumErrors::CreateHashError:
        hash_sum_log += file_path + ": Не удалось создать хэш!\n";
        break;
    case HashSumErrors::ProviderAccessError:
        hash_sum_log += file_path + ": Не удалось получить доступ к криптопровайдеру!\n";
        break;
    case HashSumErrors::OpenFileError:
        hash_sum_log += file_path + ": Не удалось открыть файл для чтения или получить"
                                    "доступ к файлам папки!\n";
        break;
    default:
        break;
    }
}

void MainWindow::showHashSumLogs() {
    QMessageBox info_box;
    info_box.setWindowTitle("Логи вычисления контрольных сумм");
    info_box.setBaseSize(200, 100);
    info_box.setIcon(QMessageBox::Information);
    if (hash_sum_log.isEmpty()) info_box.setText("Контрольные суммы успешно посчитаны");
    else info_box.setText(hash_sum_log);
    info_box.exec();
}

void MainWindow::chooseSHA_256() {
    calcFileHashSumTriggered(CALG_SHA_256);
}

void MainWindow::chooseSHA_512() {
    calcFileHashSumTriggered(CALG_SHA_512);
}

void MainWindow::chooseMD5() {
    calcFileHashSumTriggered(CALG_MD5);
}

void MainWindow::on_show_log_2_triggered()
{
    QProcess proc;
    proc.startDetached("notepad.exe", QStringList{"log.txt"});
}

void MainWindow::on_actionSaveSnap_triggered()
{
    loading_window->show();

    // имя файла - текущее время
    QDateTime currentTime = QDateTime::currentDateTime();
    QString fileName = QFileInfo(dir->rootPath()).fileName() + "_" + currentTime.toString("hhmmsszzzddMMyyyy");

    emit returnSnapshot(dir->rootPath(), fileName, CALG_SHA_256, currentTime);
    // qDebug() << "here";
}

bool MainWindow::createDirectory(const QString &path) {
    QDir dir;
    if (dir.exists(path)) {
        qDebug() << "Directory already exists:" << path;
        return true;
    } else {
        if (dir.mkpath(path)) {
            qDebug() << "Directory created:" << path;
            return true;
        } else {
            qDebug() << "Failed to create directory:" << path;
            return false;
        }
    }
}

void MainWindow::handleSnapshotCalculations(const QString file_name, const QDateTime current_time) {
    QSqlRecord record = SQLmodel->record();
    record.setValue("DirectoryPath", dir->rootPath());
    record.setValue("SnapshotPath", QDir::currentPath() + "/snapshots/" + file_name + ".json");
    record.setValue("SaveDate", current_time);

    SQLmodel->setFilter("");
    SQLmodel->insertRecord(SQLmodel->rowCount(), record);
    SQLmodel->submitAll();
    loading_window->hide();
}

void MainWindow::on_action_load_snap_triggered()
{
    // передаем в окно сравнения путь нужной директории
    //compare_window->catchDirPath(dir->rootPath());

    // обновляем список снапшотов
    compare_window->updateDirectoriesList();
    compare_window->show();
    this->close();
}

QString MainWindow::checkForAltDS(QString filePath) {
    QString alts = "";

    WIN32_FIND_STREAM_DATA findStreamData; //cтруктура с информацией о потоке

    LPCWSTR filePathWin = reinterpret_cast<LPCWSTR>(filePath.utf16()); //приводим путь к формату win api LPCWSTR

    bool hasAltDS = false; //флаг наличия у файла/директории потока, кроме главного (наличие альт потоков)

    HANDLE hFind = FindFirstStreamW(filePathWin, FindStreamInfoStandard, &findStreamData, 0);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            QString nameADS = QString::fromWCharArray(findStreamData.cStreamName); // получаем название альт потока
            if (nameADS != "::$DATA")
            {
                nameADS = nameADS.section(':', 1, 1).section('$', 0, 0); //возвращаемое имя потока имеет вид ":имя_потока:$DATA", данными функциями вырезаем имя_потока
                alts += nameADS + ", ";
                hasAltDS = true;
            }

        } while (FindNextStreamW(hFind, &findStreamData));

        FindClose(hFind);
    }
    if (!hasAltDS) {
        alts = "Нет альт.потоков  ";
    }

    alts.removeLast();
    alts.removeLast();

    return alts;
}

void MainWindow::showEvent(QShowEvent *event)
{
    emit showed();
}

void MainWindow::on_drop_db_action_triggered()
{
    // delete query;
    // query = new QSqlQuery(db);
    query->exec("DELETE FROM Snaps;");
    Logger::checkSqlError(query->lastError());
    // query;
}

