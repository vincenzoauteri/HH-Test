@echo off
subst z: "c:\users\vincenzo-auteri\documents\github\"
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
set path=z:\hh-test\misc:%path%
