@echo off
cls
cd %~dp0

del db.csv

FOR /L %%i IN (1,1,15) DO CALL :collect %%i
echo done!
PAUSE
GOTO :EOF

:collect
cd %1-threads
FOR /F "" %%i IN ('dir /B fps_*.csv') DO CALL :save %1 %%i

cd..
GOTO :EOF


:save
<NUL set /p "=%1 thread(s)">>..\db.csv
FOR /F "skip=1" %%i IN (%2) DO <NUL set /p "=;%%i">>..\db.csv
echo.>>..\db.csv