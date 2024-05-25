#ifndef LOADINGWINDOW_H
#define LOADINGWINDOW_H

#include <QMessageBox>
#include <QWidget>
#include <QLabel>
#include <QFormLayout>
#include <QBoxLayout>
#include <QCloseEvent>

/**
 * @brief The LoadingWindow class
 *
 * LoadingWindow - это пользовательский диалоговый класс, производный от QMessageBox,
 * предназначенный для отображения окна загрузки.
 * В классе перегружено событие (event) закрытия окна таким образом, чтобы окно
 * нельзя было закрыть.
 */
class LoadingWindow : public QMessageBox
{
    Q_OBJECT

public:
    /**
     * При создании объекта LoadingWindow:
     * - Стиль CSS: Устанавливается стиль CSS для текста окна,
     * делая его черным (rgb(0, 0, 0)).
     * - Текст сообщения: Отображается текст "Молчать! Идет подсчет
     * КОНТРОЛЬНЫХ СУММ!", который информирует пользователя о процессе,
     * который в данный момент не должен быть прерван.
     * - Кнопки: Устанавливается, что стандартные кнопки не будут
     * отображаться (QMessageBox::NoButton), тем самым предотвращая
     * любые возможности пользователя закрыть окно стандартным способом
     * до завершения операции.
     *
     * @param parent родительский виджет.
     */
    LoadingWindow(QWidget *parent = nullptr);

    /**
     * Переопределенный метод, который запрещает закрытие окна.
     *
     * @param event указатель на объект события закрытия
     */
    void closeEvent(QCloseEvent *event) override;
};

#endif // LOADINGWINDOW_H
