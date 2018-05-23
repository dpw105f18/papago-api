@ECHO OFF
CLS
CD ~dp0

REM %1 = platform
REM %2 = configuration

ECHO %1/%2/shaders/
ECHO %1/%2/textures/

COPY /Y shaders\* %1\%2\shaders /B
COPY /Y textures\* %1\%2\textures /B