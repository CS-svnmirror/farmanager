@echo off
setlocal
:: This batch file cleares the plugins cache

call :clear "hkcu\software\far2\pluginscache"

goto :eof

:clear
reg query %1 >nul 2>^&1
if not errorlevel 1 (
echo.
echo deleting %1...
reg delete %1 /f
)
