if "%1"=="" goto exit

del /q DIRS
echo DIRS=drv>>DIRS
MakeDriver %DDKROOT% %1 %WORKPATH% FREE

:exit