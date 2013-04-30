@echo off

REM re-compile
echo Recompiling... Please wait
devenv "./CIS687-Pr4.sln" /rebuild debug

REM for local machine test
REM "C:/Program Files (x86)/Microsoft Visual Studio 11.0/Common7/IDE/devenv.exe" "./CIS687-Pr4.sln" /rebuild debug

echo Solution Compiled.  Enjoy!