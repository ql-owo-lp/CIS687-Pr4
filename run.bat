@echo off

REM copy executive files to current directory just to make working directory right
copy /A .\users.database.xml .\Debug /Y
copy /B .\src\Client-UI\bin\Debug\ClientGUI.exe .\Debug /Y
copy /B .\src\Client-UI\bin\Debug\MahApps.Metro.dll .\Debug /Y
copy /B .\src\Client-UI\bin\Debug\System.Windows.Interactivity.dll .\Debug /Y

mklink /j "./Debug/ClientRepo" "./ClientRepo"
mklink /j "./Debug/ServerRepo" "./ServerRepo"

cd .\Debug
start Server.exe

REM sleep for 1 sec, so that the randomly generated port will not be same for same rand seed
echo Wait one second to open the client...
ping -n 2 127.0.0.1 > nul

start ClientGUI.exe

REM Goback, doesn't matter actually
cd ../