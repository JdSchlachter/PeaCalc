@echo off
echo.
echo. Building with MinGw ....
echo.
echo. Expecting MinGw in C:\WinTools\mingw64\bin!
echo.

rem * Prepare the environment:
set PATH=C:\Tools\mingw64\bin\;%PATH%
mkdir  ..\build

rem * Build manual:
del ..\build\PeaCalc.html
echo ^<meta http-equiv="Content-Type" content="text/html; charset=utf-8" /^>                         >  ..\build\PeaCalc.html
echo ^<style title="BodyText" type="text/css"^>                                                     >>  ..\build\PeaCalc.html
echo  body { font-family: Segoe UI,Helvetica,sans-serif; font-weight: normal; font-size: 4mm; }     >>  ..\build\PeaCalc.html
echo  pre { background-color: #e4e4e4; font-family: "Consolas",Consolas,monospace; padding: 10px; } >>  ..\build\PeaCalc.html
echo  em { color: #000099; }                                                                        >>  ..\build\PeaCalc.html
echo ^</style^>                                                                                     >>  ..\build\PeaCalc.html
pandoc ..\README.md >> ..\build\PeaCalc.html

rem * Copy additional files:
copy    .\PeaCalc.ini ..\build /Y
copy   ..\LICENSE     ..\build\LICENSE.txt /Y

rem * ... and build:
g++ -Wall -o ..\build\PeaCalc.exe -mwindows -static -static-libgcc -static-libstdc++ PeaCalc.cpp ConfigHandler.cpp CommandHandler.cpp Term.cpp

pause