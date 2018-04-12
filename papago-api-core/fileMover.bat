@echo off
cd %~dp0

@mkdir bin\Debug\x64\shader\
@mkdir bin\Debug\x64\textures\

for /r "%~dp0\shader" %%f in (*.vert) do @copy "%%f" bin\Debug\x64\shader\
for /r "%~dp0\shader" %%f in (*.frag) do @copy "%%f" bin\Debug\x64\shader\
for /r "%~dp0\textures" %%f in (*.jpg) do @copy "%%f" bin\Debug\x64\textures\

@mkdir bin\Release\x64\shader\
@mkdir bin\Release\x64\textures\

for /r "%~dp0\shader" %%f in (*.vert) do @copy "%%f" bin\Release\x64\shader\
for /r "%~dp0\shader" %%f in (*.frag) do @copy "%%f" bin\Release\x64\shader\
for /r "%~dp0\textures" %%f in (*.jpg) do @copy "%%f" bin\Release\x64\textures\

