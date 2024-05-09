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
    // (не нужно, список обновляется при открытии окна)
    //updateDirectoriesList();

    ui->right_table_view->setModel(SQLmodel);
    ui->right_table_view->resizeColumnsToContents();

    // Вставляем начальный текст в окно сравнения
    ui->comparison_output->insertPlainText("Hello there\n");
    // Устанавливаем индекс -1, чтобы знать, что мы ещё ничего не выбирали
    ui->dir_box->setCurrentIndex(-1);

    // Коннектим выбор директории в комбо боксе с изменением доступных снапшотов
    connect(ui->dir_box, &QComboBox::currentIndexChanged, this, &CompareWindow::updateSnapshotsList);

    // Коннектим выбор снапшотов в комбо боксе с их сравнением
    connect(ui->left_snaphots_box, &QComboBox::currentIndexChanged, this, &CompareWindow::compareSnapshots);
    connect(ui->right_snapshots_box, &QComboBox::currentIndexChanged, this, &CompareWindow::compareSnapshots);
}

void CompareWindow::closeEvent(QCloseEvent *event) {
    SQLmodel->setFilter("");
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

    // Удаляем старые варианты
    ui->left_snaphots_box->clear();
    ui->right_snapshots_box->clear();
    ui->comparison_output->clear();

    // Если директория ещё не выбрана, то ничего не делаем
    if (ui->dir_box->currentIndex() == -1) return;

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

void CompareWindow::compareSnapshots(){
    // сравниваем выбранные снапшоты


    int left_index = ui->left_snaphots_box->currentIndex();
    int right_index = ui->right_snapshots_box->currentIndex();

    // Если хотя бы один из снапшотов ещё не выбрана, то ничего не делаем
    if (left_index == -1) return;
    if (right_index == -1) return;

    // Начало сравнения, очищаем окно вывода
    qDebug() << "\nCompare start";
    ui->comparison_output->clear();

    // получаем из БД выбранные пути
    QString left_path = SQLmodel->record(left_index).value("SnapshotPath").toString();
    QString right_path = SQLmodel->record(right_index).value("SnapshotPath").toString();

    qDebug() <<left_path << ' ' << right_path;

    // создаем снапшоты по полученным путям
    Snapshot left_snap(left_path);
    Snapshot right_snap(right_path);

    // Получаем разницу в сделанных снапшотах
    QVector<ComparisonAnswer> compare_result = left_snap.compareSnapshots(right_snap);

    // Проходим по всем изменениям и выводим их
    int mismatches_num = compare_result.size();
    for(int i = 0; i < mismatches_num; i++){
        QString flag;
        switch (compare_result[i].flag) {
        case ComparisonFlags::deleted:
            flag = " <b>Удален:</b>";
            break;

        case ComparisonFlags::edited:
            flag = " <b>Изменен:</b>";
            break;

        case ComparisonFlags::appeared:
            flag = " <b>Добавлен:</b>";
            break;

        default:
            break;
        }

        ui->comparison_output->append(flag);

        ui->comparison_output->append(compare_result[i].path);
        ui->comparison_output->append("\n");
    }

    // Если изменений нет, то выводим сообщение об этом
    if (mismatches_num == 0) ui->comparison_output->append("Снапшоты идентичны");


}
