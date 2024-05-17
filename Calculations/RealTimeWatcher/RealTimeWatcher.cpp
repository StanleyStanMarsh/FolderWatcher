#include "RealTimeWatcher.h"

RealTimeWatcher::RealTimeWatcher(QFileSystemModel *_dir, QObject *parent)
    : QObject{parent}
{
    this->dir = _dir;
}

// void RealTimeWatcher::changeDir(const QString &_dir_path)
// {
//     qDebug() << "here";
//     this->dir_path = _dir_path;
//     this->file = CreateFile(this->dir_path.toStdWString().c_str(),
//         FILE_LIST_DIRECTORY,
//         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
//         NULL,
//         OPEN_EXISTING,
//         FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
//         NULL);
//     assert(this->file != INVALID_HANDLE_VALUE);

//     this->overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);
//     uint8_t change_buf[1024];
//     BOOL success = ReadDirectoryChangesW(
//         this->file, change_buf, 1024, TRUE,
//         FILE_NOTIFY_CHANGE_FILE_NAME |
//         FILE_NOTIFY_CHANGE_DIR_NAME |
//         FILE_NOTIFY_CHANGE_LAST_WRITE,
//         NULL, &(this->overlapped), NULL);
// }

void RealTimeWatcher::watch()
{

    qDebug() << "watching";
    uint8_t change_buf[1024];
    while (true) {

        QString q_dir_path = this->dir->rootDirectory().absolutePath();
        const wchar_t* dir_path = q_dir_path.toStdWString().c_str();
        // qDebug() << q_dir_path;
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

        BOOL success = ReadDirectoryChangesW(
            file, change_buf, 1024, TRUE,
            FILE_NOTIFY_CHANGE_FILE_NAME |
            FILE_NOTIFY_CHANGE_DIR_NAME |
            FILE_NOTIFY_CHANGE_LAST_WRITE,
            NULL, &overlapped, NULL);

        if (q_dir_path.isEmpty()) continue;

        DWORD result = WaitForSingleObject(overlapped.hEvent, 0);



        if (result == WAIT_OBJECT_0) {

            DWORD bytes_transferred;
            GetOverlappedResult(file, &overlapped, &bytes_transferred, FALSE);

            FILE_NOTIFY_INFORMATION* event = (FILE_NOTIFY_INFORMATION*)change_buf;

            for (;;) {

                DWORD name_len = event->FileNameLength / sizeof(wchar_t);

                switch (event->Action) {
                case FILE_ACTION_ADDED: {
                    qDebug() << "added";
                    wprintf(L"       Added: %.*s\n", name_len, event->FileName);
                } break;

                case FILE_ACTION_REMOVED: {
                    qDebug() << "removed";
                    wprintf(L"     Removed: %.*s\n", name_len, event->FileName);
                } break;

                case FILE_ACTION_MODIFIED: {
                    qDebug() << "Modified";
                    wprintf(L"    Modified: %.*s\n", name_len, event->FileName);
                } break;

                case FILE_ACTION_RENAMED_OLD_NAME: {
                    qDebug() << "Renamed";
                    wprintf(L"Renamed from: %.*s\n", name_len, event->FileName);
                } break;

                case FILE_ACTION_RENAMED_NEW_NAME: {
                    qDebug() << "to";
                    wprintf(L"          to: %.*s\n", name_len, event->FileName);
                } break;

                default: {
                    qDebug() << "unknown action";
                    printf("Unknown action!\n");
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
                file, change_buf, 1024, TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME |
                FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_LAST_WRITE,
                NULL, &overlapped, NULL);

        }

        // Do other loop stuff here...
    }
}
