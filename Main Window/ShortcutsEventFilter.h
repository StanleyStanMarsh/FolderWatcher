#ifndef SHORTCUTSEVENTFILTER_H
#define SHORTCUTSEVENTFILTER_H

#include <QObject>
#include <QEvent>
#include <QDebug>

class ShortcutsEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit ShortcutsEventFilter(QObject *parent = nullptr) : QObject(parent) {}
protected:
    /**
     * Перегрузка сигнала класса QObject
     *
     * eventFilter является переопределенным методом,
     * который служит для перехвата и фильтрации событий
     * ввода на клавиатуре прежде, чем они достигнут виджета.
     * Этот метод реализован в классе ShortcutsEventFilter,
     * который должен устанавливаться как фильтр событий для
     * объекта, через который проходят события нажатия клавиш.
     *
     * @param watched Указатель на объект, за которым наблюдается (где фильтруются события)
     * @param event Указатель на объект события, который содержит подробности о событии, попавшем в фильтр
     * @return Возвращает true, если событие должно быть отфильтровано.
     * Возвращает результат метода QObject::eventFilter в противном случае.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // SHORTCUTSEVENTFILTER_H
