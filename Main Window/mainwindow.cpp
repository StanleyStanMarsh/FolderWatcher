#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // настройка модели работы с директориями
    ui->setupUi(this);
    dir = new QFileSystemModel;
    dir->setRootPath(QDir::currentPath());
    ui->listView->setModel(dir);
    ui->listView->setRootIndex(dir->index(QDir::currentPath()));
    // Отключаем горячие клавиши для listView
    filter = new ShortcutsEventFilter(this);
    ui->listView->installEventFilter(filter);

    ui->path_line->setText(QDir::currentPath());

    // настройка модели работы с инфой о директориях
    info = new QStandardItemModel;
    ui->tableView->resizeColumnsToContents();
    QHeaderView *header = ui->tableView->horizontalHeader();
    header->setStretchLastSection(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setModel(info);

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

    // Коннектим нажатие на кнопки подсчета КС к слоту-отправителю сигнала с данными о выделенных
    // файлах и папках
    connect(ui->actionSHA_256, &QAction::triggered, this, &MainWindow::chooseSHA_256);
    connect(ui->actionSHA_512, &QAction::triggered, this, &MainWindow::chooseSHA_512);
    connect(ui->actionMD5, &QAction::triggered, this, &MainWindow::chooseMD5);

    // Коннектим завершение потока с планированием удаления "вычислителя" КС
    connect(&hash_sum_thread, &QThread::finished, calculator, &QObject::deleteLater);
    // Коннектим сигнал с данными о выделенных файлах и папках со слотом вычисления КС
    connect(this, &MainWindow::returnHashSum, calculator, &HashSum::getHashSums);
    // Коннектим сигнал о завершении вычисления КС с внесением полученных данных в таблицу
    connect(calculator, &HashSum::hashSumsReady, this, &MainWindow::handleHashSumCalculations);
    // Коннектим сигнал о завершении вычисления КС с показом логов
    connect(calculator, &HashSum::hashSumsReady, this, &MainWindow::showHashSumLogs);
    // Коннектим сигнал об ошибках со сбором ошибок
    connect(calculator, &HashSum::errorOccured, this, &MainWindow::handleHashSumErrors);

    hash_sum_thread.start();

    // Коннектим двойное нажатие по папке/файлу к его открытию
    QObject::connect(ui->listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(goDownDir(QModelIndex)));
    // Коннектим нажатие по кнопке с поднятием на одну папку наверх
    QObject::connect(ui->back_button, SIGNAL(clicked(bool)), this, SLOT(goUpDir()));
    // Коннектим нажатие по кнопке с отображением инфы
    QObject::connect(ui->main_info_button, SIGNAL(clicked(bool)), this, SLOT(showMainInfo()));
    // Коннектим выбор доступного хранилища в комбо боксе с его открытием модели
    QObject::connect(ui->storages_box, SIGNAL(textActivated(QString)), this, SLOT(goToStorage(QString)));
    // Коннектим изменение корневого пути в модели с его отображением в строке
    QObject::connect(dir, SIGNAL(rootPathChanged(QString)), ui->path_line, SLOT(setText(QString)));
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
    info->setColumnCount(4);
    info->setHorizontalHeaderLabels(QStringList{"name", "date", "type", "size"});

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
                }

            }
            // Если галочки нет - в таблице для папок размер не указывается
            else
                row << new QStandardItem("");
        }

        info->appendRow(row);
    }
    ui->info_label->setStyleSheet("color: rgb(0, 0, 0)");
    ui->info_label->setText("Информация");
}

MainWindow::~MainWindow()
{
    delete dir;
    delete info;
    delete ui;
    delete loading_window;
    delete filter;
    hash_sum_thread.quit();
    hash_sum_thread.wait();
}

void MainWindow::on_info_message_triggered() const
{
    QMessageBox info_box;
    info_box.setWindowTitle("О программе...");
    info_box.setBaseSize(200, 100);
    info_box.setIcon(QMessageBox::Information);
    info_box.setText("<b>Программа FolderWatcher ver. 0.2</b>");
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
                    //qDebug() << e.what();
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
    while (f_size > 1024 && i < 2) {
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
    loading_window = new LoadingWindow();
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
