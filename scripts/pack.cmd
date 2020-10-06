@ECHO OFF
powershell.exe -NoLogo -NoProfile -ExecutionPolicy ByPass -Command "& """%~dp0build.ps1""" -pack %*"
EXIT /B %ERRORLEVEL%