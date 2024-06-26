#ifndef COMPAREWINDOW_H
#define COMPAREWINDOW_H

#include <QWidget>
#include <QCloseEvent>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QStringList>
#include <QSet>
#include <QDateTime>

#include "../Calculations/Snapshots/snapshot.h"
#include "../Logger/Logger.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CompareWindow; }
QT_END_NAMESPACE

class CompareWindow : public QWidget
{
    Q_OBJECT
public:
    explicit CompareWindow(QSqlTableModel *SQLmodel, QWidget *parent = nullptr);

    /**
     * Функция для принятия пути до папки от главного окна, для которой будет производиться сравнение снапшотов
     *
     * @param dir_path Путь до папки
     */
    [[maybe_unused]] void catchDirPath(QString dir_path) { this->dir_path = dir_path; }

    /**
     * Перегруженная функция события закрытия окна, в которой отправляется сигнал о закрытии
     *
     * @param event Событие закрытия
     * @see closed()
     */
    void closeEvent(QCloseEvent *event) override;

public slots:
    /**
     * Слот для обновления списка доступных директорий
     */
    void updateDirectoriesList();

    /**
     * Слот для обновления списка доступных снапшотов
     */
    void updateSnapshotsList();

    /**
     * Слот для сравнения выбранных снапшотов
     */
    void compareSnapshots();

signals:
    /**
     * Сигнал который испускается при закрытии окна
     */
    void closed();

private:
    /// Графическая форма окна
    Ui::CompareWindow *ui;

    /// Путь до папки
    [[maybe_unused]] QString dir_path;

    /// Объект модели БД
    QSqlTableModel *SQLmodel;

    /// Модель работы с файлами снапшота
    QStandardItemModel *first_snap_files;
};

#endif // COMPAREWINDOW_H
