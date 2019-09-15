# libdelphi
Delphi classes for C++

НАСТРОЙКА
-

###### Параметры конфигурации CMake

В файле `CMakeLists.txt` укажите фаги:

Логический флаг **WITH_POSTGESQL** можно использовать, чтобы включить поддержку PostgreSQL. По умолчанию установлено в ON.

Логический флаг **WITH_SQLITE3** можно использовать, чтобы включить поддержку sqlite3. По умолчанию установлено в OFF.

СБОРКА И УСТАНОВКА
-
Для сборки библиотеки Вам потребуется:

1. Компилятор C++;
1. [CMake](https://cmake.org) или интегрированная среда разработки (IDE) с поддержкой [CMake](https://cmake.org);
1. Библиотека [libpq-dev](https://www.postgresql.org/download/) (libraries and headers for C language frontend development);
1. Библиотека [postgresql-server-dev-10](https://www.postgresql.org/download/) (libraries and headers for C language backend development).
1. Библиотека [sqllite3](https://www.sqlite.org/download/) (SQLite 3);

Для того чтобы установить компилятор C++ и необходимые библиотеки на Ubuntu выполните:
~~~
sudo apt-get install build-essential libssl-dev libcurl4-openssl-dev make cmake gcc g++
~~~

Для того чтобы установить SQLite3 выполните:
~~~
sudo apt-get install sqlite3 libsqlite3-dev
~~~

Для того чтобы установить PostgreSQL воспользуйтесь инструкцией по [этой](https://www.postgresql.org/download/) ссылке.

###### Подробное описание установки C++, CMake, IDE и иных компонентов необходимых для сборки проекта не входит в данное руководство. 

Для сборки **libdelphi**, необходимо:

1. Скачать **libdelphi** по [ссылке](https://github.com/ufocomp/libdelphi/archive/master.zip);
1. Распаковать;
1. Скомпилировать (см. ниже).

Для сборки **libdelphi**, с помощью Git выполните:
~~~
git clone https://github.com/ufocomp/libdelphi.git
~~~

###### Сборка:
~~~
cd libdelphi
cmake -DCMAKE_BUILD_TYPE=Release . -B cmake-build-release
или
cmake -DCMAKE_BUILD_TYPE=Debug . -B cmake-build-debug
~~~

###### Компиляция и установка:
~~~
cd cmake-build-release
make
sudo make install
~~~

По умолчанию **libdelphi** будет установлена в:
~~~
/usr/local/lib
~~~
