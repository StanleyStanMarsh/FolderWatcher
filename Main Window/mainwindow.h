#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <QMainWindow>
#include <QSplitter>
#include <QListView>
#include <QTreeView>
#include <QDir>
#include <QFileSystemModel>
#include <QStorageInfo>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QStack>
#include <QVector>
#include <QThread>
#include <QPair>
#include <QDesktopServices>
#include <experimental/filesystem>
#include <string>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QProcess>

#include "../Calculations/Hash Sum/HashSum.h"
#include "../Loading Window/LoadingWindow.h"
#include "ShortcutsEventFilter.h"
#include "../Logger/Logger.h"
#include "../Calculations/Snapshots/snapshot.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace fsys = std::experimental::filesystem;

/**
 * Главное окно приложения FolderWatcher
 *
 * @author [Астафьев Игорь](https://github.com/StanleyStanMarsh)
 * @author [Якунин Дмитрий](https://github.com/tutibase)
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    /**
     * Слот для перехода к выделенной папке
     *
     * @param index Индекс элемента в модели данных,
     * который соответствует директории, в которую пользователь
     * хочет перейти "вниз".
     */
    void goDownDir(const QModelIndex &index);

    /**
     * Слот для возврата на одну директорию выше
     *
     */
    void goUpDir();

    /**
     * Слот для вывода основной инфы о текущей директории в таблице
     *
     * Функция showMainInfo отвечает за вывод основной информации о
     * файлах и папках в текущей директории в таблицу {@link #info}. Эта
     * информация включает имя, дату последнего изменения, тип и размер
     * каждого элемента. Для директорий размер может быть подсчитан, если
     * это предусмотрено настройками интерфейса пользователя.
     */
    void showMainInfo();

    /**
     * Слот для открытия информационного сообщения
     */
    void on_info_message_triggered() const;

    /**
     * Слот для сохранения снапшота из меню
     */
    void on_actionSaveSnap_triggered();

    /**
     * Слот для перехода в корень выбранного локального хранилища
     *
     * @param storage_path Путь к целевому носителю хранения,
     * который станет новым корневым путем для отображения в
     * виджете {@link #listView} и для модели {@link #dir}.
     */
    void goToStorage(const QString &storage_path);

    /**
     * Слот для выбора алгоритма SHA-256
     *
     * @see #chooseSHA_512()
     * @see #chooseMD5()
     * @see #calcFileHashSumTriggered()
     */
    void chooseSHA_256();

    /**
     * Слот для выбора алгоритма SHA-512
     *
     * @see #chooseSHA_256()
     * @see #chooseMD5()
     * @see #calcFileHashSumTriggered()
     */
    void chooseSHA_512();

    /**
     * Слот для выбора алгоритма MD5
     *
     * @see #chooseSHA_256()
     * @see #chooseSHA_512()
     * @see #calcFileHashSumTriggered()
     */
    void chooseMD5();

    /**
     * Слот для вызова сигнала returnHashSum
     *
     * @param hashAlgorithm Хэш алгоритм для подсчета контрольных сумм
     * @see #returnHashSum()
     */
    void calcFileHashSumTriggered(const ALG_ID &hashAlgorithm);

    /**
     * Слот для принятия результатов подсчета контрольных сумм
     *
     * @param vec_rows Вектор пар <имя файла/папки, хеш-сумма>
     * @param elapsed_time Потраченное на подсчеты время
     */
    void handleHashSumCalculations(const HashSumRow &vec_rows, const QString &elapsed_time);

    /**
     * @brief handleSnapshotCalculations
     */
    void handleSnapshotCalculations(const QString file_name, const QDateTime current_time);

    /**
     * Слот для перехвата ошибок при вычислении контрольных сумм
     *
     * @param error Возникающие ошибки
     * @param file_path Путь до файла, при обработке которого возникла ошибка
     * @see #HashSum::HashSumErrors
     */
    [[maybe_unused]] void handleHashSumErrors(const HashSumErrors &error, const QString &file_path);

    /**
     * Слот для отображения логов о вычислении КС
     *
     * Функция showHashSumLogs отображает логи процесса вычисления
     * контрольных сумм в диалоговом окне. Если лог пуст, функция
     * информирует пользователя о том, что контрольные суммы посчитаны
     * успешно. В противном случае показывается текст лога, содержащий
     * возможные ошибки или сообщения о ходе выполнения операции.
     */
    [[maybe_unused]] void showHashSumLogs();

    // FIXME: функция не работает как и сам объект
    /**
     * Слот для открытия файла логов
     */
    void on_show_log_2_triggered();

signals:

    /**
     * Сигнал, который отправляет список при нажатии на кнопку вычисления
     *
     * @param selected_files Выделенные в отображении файлы и папки
     * @param dir_info Основная файловая модель, в которой содержится необходимая информация о текущей директории
     * @param hashAlgorithm Хэш алгоритм, которым будут считаться контрольные суммы
     */
    void returnHashSum(const QModelIndexList& selected_files, const QFileSystemModel *dir_info,
                       const ALG_ID &hashAlgorithm);

    /**
     * @brief returnSnapshot
     * @param dir_path
     * @param hash_algorithm
     */
    void returnSnapshot(const QString dir_path, const QString fil_name,
                        const ALG_ID hash_algorithm, const QDateTime current_time);

    void errorOccured(const std::exception &e, const QString &file_path);

private:
    /**
     * Функция для получения размера директории
     *
     * @param[in] rootFolder путь до папки
     * @param[out] f_size размер посчитанной папки
     * @see [Оригинальная функция](https://stackoverflow.com/questions/15495756/how-can-i-find-the-size-of-all-files-located-inside-a-folder)
     */
    void getFoldersizeIterative(std::wstring rootFolder, unsigned long long &f_size);

    /**
     * Функция задержки
     *
     * @param n время в миллисекундах
     */
    void delay(int n) const;

    /**
     * Функция для сокращения записи размера файла
     *
     * @param f_size размер файла/папки в байтах
     * @return строка с размером файла/папки в сокращенном виде и единицей измерения ((K-, M-, G-)bytes)
     */
    QString getMinimizedFormSize(double &f_size);

    /**
     * Функция для создания директории под снапшоты
     *
     * @param path - путь до нужной директории
     * @return true - директория уже была/создана, false - не удалось создать директорию
     */
    bool createDirectory(const QString &path);

    /// Графическая форма окна
    Ui::MainWindow *ui;

    /// Модель для взаимодействия с директориями
    QFileSystemModel *dir;
    /// Модель работы с информацией о директории
    QStandardItemModel *info;

    /// Отдельный поток для контрольных сумм
    QThread hash_sum_thread;

    /// Отдельный поток для ведения логов
    QThread logger_thread;

    /// Отдельный поток для снапшотов
    QThread snapshot_thread;

    /// Окно загрузки
    LoadingWindow *loading_window;

    /// Фильтр событий отключающий горячие клавиши
    ShortcutsEventFilter *filter;

    /// Снапшоты директорий
    Snapshot *snap;

    /// Лог для сбора ошибок при подсчете КС
    QString hash_sum_log;

    /// Работа с БД
    QSqlDatabase db;
    QSqlQuery *query;
    QSqlTableModel *SQLmodel;
};
#endif // MAINWINDOW_H
