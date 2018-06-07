@ECHO off
CD %~dp0

ECHO deleting old csv files..
DEL *.csv

SET /P "dataCount=Data entries: "
SET cubeDim=30
SET testCount=5
SET maxThreadCount=15
SET /P "output=Output Folder (in data/): "

SET exename=Papago-benchmark
SET "drawArg=-cubeDim %cubeDim% -cubePad 1 -csv -frameTime -dataCount %dataCount%"

DEL *.csv

FOR /L %%i IN (1,1,%testCount%) DO (
	FOR /L %%j IN (1,1,%maxThreadCount%) DO CALL :run %%j %%i
)
ECHO Tests Complete!
PAUSE
EXIT

:run
IF /I %1 GTR 1 (
	ECHO Cooling down...
	TIMEOUT /T 15
)

START "Test #%1 Threads" /WAIT %exename%.exe %drawArg% -threadCount %1
SET "folder=data\%output%\run-%2\
FOR /F "delims=_. tokens=2" %%i IN ('dir /B frameTime_*.csv') DO CALL :mover %%i %folder% %1
SET "folder="
ECHO Test %2.%1 done!

GOTO :EOF

:mover
 MKDIR %2\%3-threads\
 MOVE frameTime_*.csv %2\%3-threads\
 MOVE conf_*.csv %2\%3-threads\
 COPY createDB.bat %2\%3-threads\
 GOTO :EOF