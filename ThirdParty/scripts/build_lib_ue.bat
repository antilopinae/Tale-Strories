@echo off
set ROOT=%~dp0
set BUILD=%ROOT%\build\Win64

cmake -S %ROOT% -B %BUILD% -G "Visual Studio 17 2022"
cmake --build %BUILD% --config Debug
cmake --build %BUILD% --config Release
