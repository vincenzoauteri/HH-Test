@echo off
pushd ..\build
set CommonCompilerFlags=-MTd -nologo -Gm -Zi -GR- -EHa- -Od -Oi -WX -W4 -wd4100 -wd4505 -wd4189 -wd4201 -DDEBUG_MODE=1 -DHANDMADE_WIN32=1 -FC 
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib winmm.lib
REM set filename=handmade_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%.pdb
set filename=handmade_%random%.pdb
echo %filename%
del *.pdb >NUL 2>NUL
cl %CommonCompilerFlags% ..\code\handmade.cpp -Fmhandmade.map -LD /link -incremental:no -opt:ref -PDB:%filename% -EXPORT:gameUpdateAndRender -EXPORT:gameGetSoundSamples 
cl %CommonCompilerFlags% ..\code\win32_handmade.cpp -Fmwin32_handmade.map /link %CommonLinkerFlags% 
REM cl -Gm -MT -nologo -GR- -EHa- -Oi -W4 -wd4100 -wd4189 -wd4201 -DDEBUG_MODE=1 -DHANDMADE_WIN32=1 -FC -Z7 ..\code\win32_handmade.cpp /link -subsystem:windows,5.1 user32.lib gdi32.lib
popd
