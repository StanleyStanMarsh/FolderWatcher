# FolderWatcher

## Описание
FolderWatcher - это программное обеспечение для сравнения содержимого директорий, отслеживания изменений, создания снапшотов и сравнения текущего содержимого с ранее сохраненными слепками.

## Основные возможности
- Сравнение содержимого заданных директорий
- Создание слепков (снапшотов) директорий
- Сравнение текущего содержимого директории с ранее сохраненным слепком
- Отображение изменений в директориях

## Интерфейс
Программа обладает интуитивно понятным GUI, вдохновленным лучшими практиками от таких приложений как Total Commander, 7zip и WinRAR.

## Снапшоты
Снапшоты директорий содержат:
- Имена поддиректорий и файлов
- Размер файлов/директорий
- Контрольные суммы файлов и директорий (SHA-256, SHA512, MD5)
- Контрольные суммы всего содержимого директории
- Информацию о наличии у файлов/директорий альтернативных потоков файловой системы, включая размер, контрольные суммы и имена
- Времена создания, последнего изменения и доступа к файлам/директориям

## Разработка
Программа разработана на Qt C++, использует [Qt](https://doc.qt.io/) версии 6 и среду разработки Qt Creator. Сборка проекта осуществляется с помощью qmake. Код включает в себя основной класс главного окна [MainWindow](https://github.com/StanleyStanMarsh/FolderWatcher/blob/master/Main%20Window/mainwindow.h), наследующийся от QMainWindow, и класс [HashSum](https://github.com/StanleyStanMarsh/FolderWatcher/blob/master/Calculations/Hash%20Sum/HashSum.h) для вычисления контрольных сумм файлов и папок, использующий [Win32 API](https://learn.microsoft.com/en-us/windows/win32/api/) и являющийся производным от QObject. Методы экземпляра класса HashSum запускаются в отдельном [потоке](https://github.com/StanleyStanMarsh/FolderWatcher/commit/9d801ad8192f70f4f61bdf309f03a8ef477ae9a3) для оптимизации производительности.
