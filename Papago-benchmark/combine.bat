@echo off
mkdir frame_data
FOR /F %%i IN ('dir /b /a:d') DO CALL :copy %%i
echo Moved data, combning files.
cd frame_data
FOR /F "" %%i IN ('dir /B fps*.csv') DO CALL :combine %%i
echo Done!
PAUSE
GOTO :EOF

:copy
cd %1
FOR /F "" %%i IN ('dir /B fps*.csv') DO COPY "%%i" "../frame_data/%%i"
cd ..
GOTO :EOF

:combine
FOR /F "skip=1" %%i IN (%1) DO <NUL set /p "=;%%i">>results.csv
echo.>>results.csv
GOTO :EOF