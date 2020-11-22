@echo off

if "%1"=="" goto usage1
if "%3"=="" goto usage2
if not exist %1\bin\setenv.bat goto usage3


call %1\BIN\setenv %1 %4

%2
cd %3
build -b -w %5 %6 %7 %8
goto ok

:usage1
echo Error: the first parameter is NULL!
goto exit

:usage2
echo Error: the third parameter is NULL! 
goto exit

:usage3
echo Error: %1\bin\setenv.bat not exist! 
goto exit

:ok
echo MakeDriver %1 %2 %3 %4 
del /q DIRS
pause Press anykey to exit...

:exit