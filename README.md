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

## Установка (Windows)

Скачать последний релиз можно [здесь](https://github.com/StanleyStanMarsh/FolderWatcher/releases).

## Поддержка
Если у вас возникли сложности или вопросы по использованию приложения, создайте 
[обсуждение](https://github.com/StanleyStanMarsh/FolderWatcher/issues/new/choose) в данном репозитории.

## Описание коммитов
| Название | Описание                                                        |
|----------|-----------------------------------------------------------------|
| docs	   | Обновление документации                                         |
| feat	   | Добавление нового функционала                                   |
| fix	   | Исправление ошибок                                              |
| refactor | Правки кода без исправления ошибок или добавления новых функций |
| revert   | Откат на предыдущие коммиты                                     |
| style	   | Правки по кодстайлу (табы, отступы, точки, запятые и т.д.)      |

![image](https://github.com/StanleyStanMarsh/FolderWatcher/assets/96591356/f2fa0409-11df-40e7-96c9-a79eec80292c)

## Лицензия (License)
Shield: [![CC BY-NC 4.0][cc-by-nc-shield]][cc-by-nc]

This work is licensed under a
[Creative Commons Attribution-NonCommercial 4.0 International License][cc-by-nc].

[![CC BY-NC 4.0][cc-by-nc-image]][cc-by-nc]

[cc-by-nc]: https://creativecommons.org/licenses/by-nc/4.0/
[cc-by-nc-image]: https://licensebuttons.net/l/by-nc/4.0/88x31.png
[cc-by-nc-shield]: https://img.shields.io/badge/License-CC%20BY--NC%204.0-lightgrey.svg
