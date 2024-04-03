#include "ShortcutsEventFilter.h"

bool ShortcutsEventFilter::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::ShortcutOverride || event->type() == QEvent::KeyPress ||
        event->type() == QEvent::KeyRelease) {
        return true;
    }
    return QObject::eventFilter(watched, event);
}
