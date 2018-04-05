@echo off
echo compiling!
cd %~dp0

C:/VulkanSDK/1.0.65.0/Bin32/glslangValidator.exe -V shader.vert
C:/VulkanSDK/1.0.65.0/Bin32/glslangValidator.exe -V shader.frag

xcopy /Y .\vert.spv ..\bin\Debug\x64\shader\vert.spv*
xcopy /Y .\frag.spv ..\bin\Debug\x64\shaders\frag.spv*
xcopy /Y .\vert.spv ..\bin\Release\x64\shaders\vert.spv*
xcopy /Y .\frag.spv ..\bin\Release\x64\shaders\frag.spv*
