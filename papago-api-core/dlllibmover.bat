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
CD ../Papago-benchmark/%platform%/%conf%
SET "dest2=%CD%"

CD %home%

COPY /B "%outDir:"=%papago-api-core.dll" "%dest%/" /B
COPY /B "%outDir:"=%papago-api-core.lib" "%dest%/" /B
COPY /B "%outDir:"=%papago-api-core.dll" "%dest2%/" /B
COPY /B "%outDir:"=%papago-api-core.lib" "%dest2%/" /B

EXIT