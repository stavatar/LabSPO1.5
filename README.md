# СПО Лабораторная работа №1.5
 
## Задание
 
Разработать способ организации данных в файле, позволяющий хранить, выбирать и гранулярно обновлять наборы записей общим объёмом от 10GB соответствующего варианту вида. Реализовать модуль или библиотеку для работы с ним в режиме курсора.

Используя данный способ сериализации, воспользоваться существующей библиотекой для описания схемы и реализации модуля, обеспечивающего функционирование протокола обмена запросами создания, выборки, модификации и удаления данных, и результатами их выполнения.

Использовать средство синтаксического анализа по выбору, реализовать модуль для разбора некоторого подмножества языка запросов по выбору в соответствии с вариантом формы данных. Должна быть обеспечена возможность описания команд создания, выборки, модификации и удаления данных.

Используя созданные модули разработать в виде консольного приложения две программы: клиентскую и серверную части. Серверная часть – получающая по сети запросы и операции описанного формата и выполняющая их над файлом, организованным в соответствии с разработанным способом. Имя файла данных для работы получать с аргументами командной строки, создавать новый в случае его отсутствия. Клиентская часть – получающая от пользователя команду, пересылающая её на сервер, получающая ответ и выводящая его в человекопонятном виде.

### Вариант 1

Форма данных - документное дерево

Протокол обмена - XML

### Доп. задание

Рекурсивное чтение регистра windows, запись структуры в файл с помощью реализованных команд. Список команд генерируется скриптом.


## Доступные команды

- `create path` (создать пустой элемент) или `create path[value]` (создать элемент со значением) 
- `update path[value]`
- `read path`
- `delete path` (удалить элемент) или `delete path[]` (удалить значение элемента)


## Инструкция по запуску:

1. `server [port] [file_name]`
2. `client [port]`

### Для чтения регистра:

1. В скрипте `script.vbs` указать путь до нужного раздела регистра в HKLM в переменной `slashPath`. Например: `SYSTEM\Input`
2. Выполнить скрипт, например `cscript script.vbs`. Скрипт сгенерирует тектовый файл `commands.txt` с командами (имя файла задается в скрипте переменной `outFile`)
3. `server [port] [file_name]`
4. `cat commands.txt | client [port]`
