@echo off

rem * Prepare the environment:
set MINGWPATH=C:\Tools\mingw64\bin
set PATH=%MINGWPATH%\;%PATH%
mkdir  ..\build

rem * Give out user-information:
echo.
echo. Building with MinGw
echo.
echo. Expecting MinGw in %MINGWPATH%...
echo.

rem * Build manual:
del ..\build\PeaCalc.html
echo ^<meta http-equiv="Content-Type" content="text/html; charset=utf-8" /^>                         >  ..\build\PeaCalc.html
echo ^<style title="BodyText" type="text/css"^>                                                     >>  ..\build\PeaCalc.html
echo  body { font-family: Segoe UI,Helvetica,sans-serif; font-weight: normal; padding-left: 20px; } >>  ..\build\PeaCalc.html
echo  pre { background-color: #e4e4e4; font-family: "Consolas",Consolas,monospace; padding: 20px; } >>  ..\build\PeaCalc.html
echo  em { color: #000099; }                                                                        >>  ..\build\PeaCalc.html
echo ^</style^>                                                                                     >>  ..\build\PeaCalc.html
pandoc ..\README.md >> ..\build\PeaCalc.html

rem * Copy additional files:
copy    .\PeaCalc.ini ..\build /Y
copy   ..\LICENSE     ..\build\LICENSE.txt /Y

rem * ... and build:
windres PeaCalc.rc -O coff -o PeaCalc.res
g++ -O3 -s -o ..\build\PeaCalc.exe -mwindows -static-libgcc -static-libstdc++ PeaCalc.cpp ConfigHandler.cpp CommandHandler.cpp Term.cpp PeaCalc.res -lversion
del *.res

pause