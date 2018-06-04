@ECHO OFF
CLS
CD %~dp0

ECHO deleting old csv files..
DEL *.csv

SET /P "version=cube or skull? "
SET runs=5
SET cubeDim=30
SET dataCount=10
SET threadCount=1

FOR /L %%i IN (1,1,%runs%) DO CALL :record_data %cubeDim% %dataCount% %threadCount% %version% %%i

ECHO done!
PAUSE
GOTO :EOF
:record_data
SET cubeDim=%1
SET dataCount=%2
SET threadCount=%3
SET "output=%4"
SET run=%5
SET "args=-cubeDim %cubeDim% -cubePad 1 -dataCount %dataCount% -threadCount %threadCount% -frameTime -csv"

ECHO Starting run %run%...

Papago-benchmark.exe %args%
FOR /F "delims=_. tokens=2" %%i IN ('DIR /B frameTime_*.csv') DO CALL :mover %%i %output%-%run%

GOTO :EOF
:mover
 SET "dir=data\%2\%1"
 ECHO Creating folder %dir%...
 MKDIR %dir%
 MOVE conf_*.csv %dir%\
 MOVE frameTime_*.csv %dir%\