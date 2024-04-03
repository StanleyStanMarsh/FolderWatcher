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
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // SHORTCUTSEVENTFILTER_H
