#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
//    main_splitter = new QSplitter(Qt::Horizontal);
//    this->setCentralWidget(main_splitter);

//    previous_tree = new QTreeView(main_splitter);
//    current_list = new QListView(main_splitter);

//    dir = new QFileSystemModel;
//    dir->setRootPath(QDir::currentPath());

//    previous_tree->setModel(dir);
//    previous_tree->setRootIndex(dir->index(QDir::currentPath()));
//    current_list->setModel(dir);
//    current_list->setRootIndex(dir->index(QDir::currentPath()));

    ui->setupUi(this);
    dir = new QFileSystemModel;
    dir->setRootPath(QDir::currentPath());
    ui->listView->setModel(dir);
    ui->listView->setRootIndex(dir->index(QDir::currentPath()));

    ui->treeView->setModel(dir);
    ui->treeView->setRootIndex(dir->index(QDir::currentPath()));

    QObject::connect(ui->listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(goDownDir(QModelIndex)));
    QObject::connect(ui->back_button, SIGNAL(clicked(bool)), this, SLOT(goUpDir()));
}

void MainWindow::goDownDir(const QModelIndex &index) {
    ui->listView->setRootIndex(index);
    dir->setRootPath(dir->filePath(index));
}

void MainWindow::goUpDir() {
    QDir now_dir(dir->rootPath());
    now_dir.cdUp();
    ui->listView->setRootIndex(dir->index(now_dir.absolutePath()));
    dir->setRootPath(now_dir.absolutePath());
}

MainWindow::~MainWindow()
{
//    delete main_splitter;
//    delete current_list;
//    delete previous_tree;
    delete dir;
    delete ui;
}


