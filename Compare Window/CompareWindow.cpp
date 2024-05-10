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

    // Настраиваем таблицу для вывода файлов снапшота
    first_snap_files = new QStandardItemModel;
    first_snap_files->setColumnCount(2);
    first_snap_files->setHorizontalHeaderLabels(QStringList{"Имя", "Тип"});
    ui->left_table_view->resizeColumnsToContents();
    ui->left_table_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->left_table_view->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->left_table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->left_table_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->left_table_view->setModel(first_snap_files);

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
    first_snap_files->removeRows(0, first_snap_files->rowCount());

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


    first_snap_files->removeRows(0, first_snap_files->rowCount());

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

    //qDebug() <<left_path << ' ' << right_path;

    // создаем снапшоты по полученным путям
    Snapshot left_snap(left_path);
    Snapshot right_snap(right_path);

    // Получаем все внещние файлы и папки левого снапшота
    QList<QPair<QString, QString>> left_files = left_snap.externalFiles();
    // Сортируем (сначала папки)
    std::sort(left_files.begin(), left_files.end(), [](const QPair<QString, QString>& a, const QPair<QString, QString>& b) {
            return a.second < b.second;
        });
    // Заполняем таблицу
    for (int i=0; i<left_files.size(); i++){
        QStandardItem* item1 = new QStandardItem(left_files[i].first);
        QStandardItem* item2 = new QStandardItem(left_files[i].second);

        QList<QStandardItem*> rowItems;
        rowItems << item1 << item2;

        first_snap_files->appendRow(rowItems);
    }

    // Получаем разницу в сделанных снапшотах
    QVector<ComparisonAnswer> compare_result = left_snap.compareSnapshots(right_snap);

    // Проходим по всем изменениям и выводим их
    int mismatches_num = compare_result.size();
    for(int i = 0; i < mismatches_num; i++){
        QString flag;
        switch (compare_result[i].flag) {
        case ComparisonFlags::deleted:
            flag = " <b><font color=#ff0000>Удален:</font></b>";
            break;

        case ComparisonFlags::edited:
            flag = " <b><font color=#ffa500>Изменен:</font></b>";
            break;

        case ComparisonFlags::appeared:
            flag = " <b><font color=#008000>Добавлен:</font></b>";
            break;

        default:
            break;
        }

        ui->comparison_output->append(flag);

        // Удаление первой папки
        int firstSlash = compare_result[i].path.indexOf('\\');
        QString newPath;
        if (firstSlash != -1)
            newPath = compare_result[i].path.mid(firstSlash + 1);

        ui->comparison_output->append(newPath);
        ui->comparison_output->append("");
    }

    // Если изменений нет, то выводим сообщение об этом
    if (mismatches_num == 0) ui->comparison_output->append("Снапшоты идентичны");


}
