@ECHO off
CLS
SET "home=%~dp0"
CD %home%

SET "outDir=%1"
SET "platform=%2"
SET "conf=%3"

CD ../papago-user-test/%platform%/%conf%
SET "dest=%CD%"

CD %home%
ECHO %dest%
COPY /B "%outDir:"=%papago-api-core.dll" "%dest%/" /B
COPY /B "%outDir:"=%papago-api-core.lib" "%dest%/" /B


EXIT