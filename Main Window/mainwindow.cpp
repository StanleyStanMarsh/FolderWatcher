#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    dir = new QFileSystemModel;
    dir->setRootPath(QDir::currentPath());
    ui->listView->setModel(dir);
    ui->listView->setRootIndex(dir->index(QDir::currentPath()));

    ui->treeView->setModel(dir);
    ui->treeView->setRootIndex(dir->index(QDir::currentPath()));

    // Коннектим двойное нажатие по папке/файлу к его открытию
    QObject::connect(ui->listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(goDownDir(QModelIndex)));
    // Коннектим нажатие по кнопке с поднятием на одну папку наверх
    QObject::connect(ui->back_button, SIGNAL(clicked(bool)), this, SLOT(goUpDir()));
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

MainWindow::~MainWindow()
{
    delete dir;
    delete ui;
}


