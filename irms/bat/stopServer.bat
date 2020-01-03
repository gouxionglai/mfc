@echo off
@set port=8004
@echo %port%

for /f "tokens=1-5" %%i in ('netstat -ano^|findstr ":%port%"') do taskkill /pid %%m  -f