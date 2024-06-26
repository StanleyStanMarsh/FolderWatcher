#include "RealTimeWatcher.h"

RealTimeWatcher::RealTimeWatcher(QFileSystemModel *_dir, QTextBrowser *_out, bool *_opened, QObject *parent)
    : QObject{parent}
{
    this->dir = _dir;
    this->out = _out;
    this->opened = _opened;
}

void RealTimeWatcher::watch()
{

    //
    QString q_dir_path = this->dir->rootDirectory().absolutePath();
    const wchar_t* dir_path = q_dir_path.toStdWString().c_str();

    HANDLE file = CreateFile(dir_path,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL);
    assert(file != INVALID_HANDLE_VALUE);
    OVERLAPPED overlapped;
    overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);

    uint8_t change_buf[1024];
    BOOL success = ReadDirectoryChangesW(
                file,                                              // Дескриптор открываемой директории
                change_buf,                                        // Буфер для хранения изменений
                1024,                                   // Размер буфера
                FALSE,                                             // Мониторим также поддиректории
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES |
                FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION |
                FILE_NOTIFY_CHANGE_SECURITY,                      // Типы изменений, которые мониторятся
                NULL,                                   // Количество прочитанных байт
                &overlapped,                                             // Перекрытие, не используется
                NULL                                              // Коллбэк, не используется
            );

    //qDebug() << "watching";
    //out->clear();
    while (true) {
        if (!(*opened)) return;
        if (q_dir_path.isEmpty()) continue;

        DWORD result = WaitForSingleObject(overlapped.hEvent, 0);

        if (result == WAIT_OBJECT_0) {

            DWORD bytes_transferred;
            GetOverlappedResult(file, &overlapped, &bytes_transferred, FALSE);

            FILE_NOTIFY_INFORMATION* event = (FILE_NOTIFY_INFORMATION*)change_buf;

            for (;;) {
                if (!(*opened)) return;

                size_t name_len = event->FileNameLength / sizeof(WCHAR);
                QString file_name = QString::fromWCharArray(event->FileName, name_len);
                switch (event->Action) {
                case FILE_ACTION_ADDED: {
                    out->insertHtml(QString("<font face=«Arial», color=#008000>"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;&nbsp;"
                                            "<b>Добавлен:</b></font> %1 <br>").arg(file_name));
                } break;

                case FILE_ACTION_REMOVED: {
                    out->insertHtml(QString("<font face=«Arial», color=#ff0000>"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;&nbsp;&nbsp;"
                                            "<b>Удален:</b></font> %1<br>").arg(file_name));
                } break;

                case FILE_ACTION_MODIFIED: {
                    out->insertHtml(QString("<font face=«Arial», color=#ffa500>"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "<b>Изменен:</b></font> %1<br>").arg(file_name));
                } break;

                case FILE_ACTION_RENAMED_OLD_NAME: {
                    out->insertHtml(QString("<font face=«Arial», color=#ffa500>"
                                            "<b>Переименован из:</b></font> %1<br>").arg(file_name));
                } break;

                case FILE_ACTION_RENAMED_NEW_NAME: {
                    out->insertHtml(QString("<font face=«Arial», color=#ffa500>"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;&nbsp;&nbsp;&nbsp;"
                                            "&nbsp;<b>в:</b></font> %1<br>").arg(file_name));
                } break;

                default: {
                    qDebug() << "unknown action";
                } break;
                }

                // Are there more events to handle?
                if (event->NextEntryOffset) {
                    *((uint8_t**)&event) += event->NextEntryOffset;
                }
                else {
                    break;
                }
            }




            // Queue the next event
            BOOL success = ReadDirectoryChangesW(
                        file,                                              // Дескриптор открываемой директории
                        change_buf,                                        // Буфер для хранения изменений
                        1024,                                   // Размер буфера
                        FALSE,                                             // Мониторим также поддиректории
                        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES |
                        FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION |
                        FILE_NOTIFY_CHANGE_SECURITY,                      // Типы изменений, которые мониторятся
                        NULL,                                   // Количество прочитанных байт
                        &overlapped,                                             // Перекрытие, не используется
                        NULL                                              // Коллбэк, не используется
                    );

        }
        if (!(*opened)) return;
        // При смене директории выходим из функции
        if (q_dir_path != this->dir->rootDirectory().absolutePath()){
            // out->clear();
            //qDebug() << "Конец слежки, папка изменилась";

            out->insertHtml(QString("<font face=«Arial»>"
                                    "Отслеживаемая директория изменена на ") +
                            this->dir->rootDirectory().absolutePath() +
                            QString(" </font><br><br><br>"));
            // out->verticalScrollBar()->setValue( out->verticalScrollBar()->maximum() );
            return;
        }

    }
}
