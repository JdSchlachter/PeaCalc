@echo off
echo.
echo. Building with MinGw ....
echo.
echo. Expecting MinGw in C:\WinTools\mingw64\bin!
echo.

set PATH=C:\WinTools\mingw64\bin\;%PATH%

mkdir ..\build
copy  .\PeaCalc.ini  ..\build /Y
copy ..\PeaCalc.html ..\build /Y
copy ..\LICENSE      ..\build\LICENSE.txt /Y

g++ -Wall -o ..\build\PeaCalc.exe -mwindows -static -static-libgcc -static-libstdc++ PeaCalc.cpp ConfigHandler.cpp CommandHandler.cpp Term.cpp

pause