@setlocal

echo off
set opt_or_dbg=%1

:: There is no good way to skip a parameter. Substitute it with nothing.
set _input=%*
call set params=%%_input:%1 =%%

:: Make sure built DLLs are accessible
set "PATH=%PATH%;c:\Install\msys64\mingw64\bin;build\%opt_or_dbg%"

echo Running %params%
%params%
