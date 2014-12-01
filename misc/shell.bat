@echo off
subst z: "c:\users\vince\workspace"
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
set path=z:\handmade\misc:%path%
