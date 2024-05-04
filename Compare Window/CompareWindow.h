#ifndef COMPAREWINDOW_H
#define COMPAREWINDOW_H

#include <QWidget>
#include <QCloseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class CompareWindow; }
QT_END_NAMESPACE

class CompareWindow : public QWidget
{
    Q_OBJECT
public:
    explicit CompareWindow(QWidget *parent = nullptr);

    /**
     * Функия для принятия пути до папки от главного окна, для которой будет производиться сравнение снапшотов
     *
     * @param dir_path Путь до папки
     */
    void catchDirPath(QString dir_path) { this->dir_path = dir_path; }

    /**
     * Перегруженная функция события закрытия окна, в которой отправляется сигнал о закрытии
     *
     * @param event Событие закрытия
     * @see closed()
     */
    void closeEvent(QCloseEvent *event) override;

signals:
    /**
     * Сигнал который испускается при закрытии окна
     */
    void closed();

private:
    /// Графическая форма окна
    Ui::CompareWindow *ui;

    /// Путь до папки
    QString dir_path;

};

#endif // COMPAREWINDOW_H
