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

    // настройка модели работы с инфой о директориях
    info = new QStandardItemModel;
    ui->tableView->resizeColumnsToContents();
    QHeaderView *header = ui->tableView->horizontalHeader();
    header->setStretchLastSection(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setModel(info);

    // Коннектим двойное нажатие по папке/файлу к его открытию
    QObject::connect(ui->listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(goDownDir(QModelIndex)));
    // Коннектим нажатие по кнопке с поднятием на одну папку наверх
    QObject::connect(ui->back_button, SIGNAL(clicked(bool)), this, SLOT(goUpDir()));
    // Коннектим нажатие по кнопке с отображением инфы
    QObject::connect(ui->main_info_button, SIGNAL(clicked(bool)), this, SLOT(showMainInfo()));
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

void MainWindow::showMainInfo() {
    //
    info->clear();

    // установка названий столбцов
    info->setColumnCount(4);
    info->setHorizontalHeaderLabels(QStringList{"name", "date", "type", "size"});

    // Получаем индекс текущей директории и количество файлов в директории
    QModelIndex currentDirIndex = dir->index(dir->rootPath());
    int fileCount = dir->rowCount(currentDirIndex);

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
            row << new QStandardItem(QString::number(dir->size(fileIndex)) + " byte");
        else
            row << new QStandardItem("");

        info->appendRow(row);
    }

}

MainWindow::~MainWindow()
{
    delete dir;
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

