#include "CompareWindow.h"
#include "ui_CompareWindow.h"

CompareWindow::CompareWindow(QSqlTableModel *SQLmodel, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::CompareWindow)
{
    ui->setupUi(this);

    // Название окна
    this->setWindowTitle("Compare snapshots");

    // Достаем объект БД из MainWindow
    this->SQLmodel = SQLmodel;

    // Добавляем список директорий со снапшотами из БД в comboBox
    updateDirectoriesList();

    // Устанавливаем индекс -1, чтобы знать, что мы ещё ничего не выбирали
    ui->dir_box->setCurrentIndex(-1);

    // Коннектим выбор директории в комбо боксе с изменением доступных снапшотов
    connect(ui->dir_box, &QComboBox::textActivated, this, &CompareWindow::updateSnapshotsList);
}

void CompareWindow::closeEvent(QCloseEvent *event) {
    emit closed();
    event->accept();
}

void CompareWindow::updateDirectoriesList(){
    // Ищем директории, для которых есть снапшоты и добавляем их в comboBox

    // Очищаем старые варианты
    ui->dir_box->clear();
    // Смотрим все строки БД
    SQLmodel->setFilter("");
    // Получим количество строк и номер столбца в модели
    int rows = SQLmodel->rowCount();
    int column = 0;

    // Получаем все уникальные (с помощью QSet) директории из БД
    QSet<QString> uniqueDirs;
    for (int i = 0; i < rows; ++i) {
        QSqlRecord record = SQLmodel->record(i);
        QString value = record.value(column).toString();
        uniqueDirs.insert(value);
    }

    // NOTE скорее всего неэффективное преобразование
    QStringList uniqueDirsList(QList<QString>(uniqueDirs.values()));
    // устанавливаем значения в comboBox
    ui->dir_box->addItems(uniqueDirsList);

    // сбрасываем comboBox выбора снапшотов и обновляем доступные снапшоты
    ui->left_snaphots_box->setCurrentIndex(-1);
    ui->right_snapshots_box->setCurrentIndex(-1);

    updateSnapshotsList();
}


void CompareWindow::updateSnapshotsList(){
    // Ищем снапшоты для данной директории и добавляем их в comboBox

    // Если директория ещё не выбрана, то ничего не делаем
    if (ui->dir_box->currentIndex() == -1) return;

    // Удаляем старые варианты
    ui->left_snaphots_box->clear();
    ui->right_snapshots_box->clear();

    // Смотрим только снапшоты выбранной директории
    SQLmodel->setFilter(QString("DirectoryPath = '%1'").arg(ui->dir_box->currentText()));
    // Получим количество строк и номер столбца в модели
    int rows = SQLmodel->rowCount();
    int column = 2;

    // Получаем все снапшоты директории из БД
    QStringList snapshots_list;
    for (int i = 0; i < rows; ++i) {
        QSqlRecord record = SQLmodel->record(i);
        QString value = record.value(column).toDateTime().toString("dd.MM.yyyy, hh:mm:ss");
        snapshots_list << value;
    }

    // устанавливаем значения в comboBox
    ui->left_snaphots_box->addItems(snapshots_list);
    ui->right_snapshots_box->addItems(snapshots_list);
}
