# libdelphi

Delphi classes for C++.

Settings
-

###### CMake configuration options

In the file `CMakeLists.txt`, specify the phages:

Boolean flag **WITH_POSTGRESQL** can be used to enable PostgreSQL support. The default value is **OFF**.

Boolean flag **WITH_SQLITE3** can be used to enable sqlite3 support. The default value is **OFF**.

Build and installing
-

To build you need:

1. The compiler C ++;
1. [CMake](https://cmake.org);
1. The library [libpq-dev](https://www.postgresql.org/download/) (libraries and headers for C language frontend development);
1. The library [postgresql-server-dev-10](https://www.postgresql.org/download/) (libraries and headers for C language backend development).
1. The library [sqllite3](https://www.sqlite.org/download/) (SQLite 3);

To install the C++ compiler and necessary libraries in Ubuntu, run:
~~~
sudo apt-get install build-essential libssl-dev libcurl4-openssl-dev make cmake gcc g++
~~~

To install PostgreSQL, use the instructions for [this](https://www.postgresql.org/download/) link.

To install SQLite3 run:
~~~
sudo apt-get install sqlite3 libsqlite3-dev
~~~

###### A detailed description of the installation of C++, CMake, IDE, and other components necessary for building the project is not included in this guide. 

To install (without Git) you need:

1. Download [libdelphi](https://github.com/ufocomp/libdelphi/archive/master.zip);
1. Unpack;
1. Build and compile (see below).

To install (with Git) you need:
~~~
git clone https://github.com/ufocomp/libdelphi.git
~~~

###### Build:
~~~
cd libdelphi
cmake -DCMAKE_BUILD_TYPE=Release . -B cmake-build-release
или
cmake -DCMAKE_BUILD_TYPE=Debug . -B cmake-build-debug
~~~

###### Make and install:
~~~
cd cmake-build-release
make
sudo make install
~~~

By default **libdelphi** will be set to:
~~~
/usr/local/lib
~~~
