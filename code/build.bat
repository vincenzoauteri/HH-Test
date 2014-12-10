@echo off
pushd ..\build
set CommonCompilerFlags=-Gm -MT -nologo -GR- -EHa- -Oi -Od -W4 -wd4100 -wd4189 -wd4201 -DDEBUG_MODE=1 -DHANDMADE_WIN32=1 -FC -Z7
set CommonLinkerFlags=-opt:ref user32.lib gdi32.lib
cl %CommonCompilerFlags% ..\code\win32_handmade.cpp /link %CommonLinkerFlags% 
REM cl -Gm -MT -nologo -GR- -EHa- -Oi -W4 -wd4100 -wd4189 -wd4201 -DDEBUG_MODE=1 -DHANDMADE_WIN32=1 -FC -Z7 ..\code\win32_handmade.cpp /link -subsystem:windows,5.1 user32.lib gdi32.lib
popd

