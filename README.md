## Todo List
This is a simple program aimed to let programers remember their todo lists easily. It is written with C++ language and uses
sqlite3 library which is written by C language to work with local database file. To compile a program written by C++ and C
first from the C sourc code the object file generated using gcc compiler `gcc sqlite3.c -c`, then Compile C++ source code and link with sqlite3.o(object file)
and specify the include path for sqlite3.h(header file) `g++ main.cpp sqlite3.o -I <path\to\sqlite3.h>` (with -I it knows where to find the sqlite3.h file).

[SQlite3 Source Code](https://www.sqlite.org/download.html)
