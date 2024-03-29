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
}

void MainWindow::on_action_8_triggered()
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

void MainWindow::delay(int n)
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

