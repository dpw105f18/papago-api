@ECHO OFF
CLS
CD %~dp0

ECHO deleting old csv files..
DEL *.csv

SET /P "version=cube or skull? "
SET runs=5
SET /P "dataCount=Data entries: "
SET /P "threadCount=Thread count: "
SET /P "output=Folder (in data\): "

FOR /L %%j IN (1,1,%runs%) DO (
	FOR /L %%i IN (5,5,50) DO CALL :record_data %%i %dataCount% %threadCount% %version% %%j %output%
)

ECHO done!
PAUSE
GOTO :EOF
:record_data
SET cubeDim=%1
SET dataCount=%2
SET threadCount=%3
SET "output=%4"
SET run=%5
SET "folder=%6"
SET "args=-cubeDim %cubeDim% -cubePad 1 -dataCount %dataCount% -threadCount %threadCount% -frameTime -csv"

IF /I %run% GTR 1 (
	ECHO Cooling down...
	TIMEOUT /T 15
)
ECHO Starting run %run% (%cubeDim%x%cubeDim%x%cubeDim%)...

Papago-benchmark.exe %args%
FOR /F "delims=_. tokens=2" %%i IN ('DIR /B frameTime_*.csv') DO CALL :mover %%i %6\%output%-%run%

GOTO :EOF
:mover
 SET "dir=data\%2\%1"
 ECHO Creating folder %dir%...
 MKDIR %dir%
 MOVE conf_*.csv %dir%\
 MOVE frameTime_*.csv %dir%\
 COPY combine.bat data\%2\