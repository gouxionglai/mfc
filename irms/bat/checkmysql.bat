@echo off
:: —È÷§mysql
cd /d  MySQL
call my_ini_edit.bat
:: start cmd /c mysql_servicestart.bat
call mysql_serviceinstall.bat <nul