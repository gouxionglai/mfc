@echo off
@set port=8008
@echo hhhhh
netstat -ano|findstr "0.0.0.0:%port%"