@ECHO OFF
CLS
CD ~dp0

REM %1 = platform
REM %2 = configuration

MKDIR %1\%2\shaders
MKDIR %1\%2\textures

COPY /Y shaders\* %1\%2\shaders\
COPY /Y textures\* %1\%2\textures\