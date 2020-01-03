@echo off
::隐藏cmd窗口
::if "%1" == "h" goto begin 
::mshta vbscript:createobject("wscript.shell").run("%~nx0 h",0)(window.close)&&exit 
:::begin
echo 环境检测中......
echo 当前盘符和路径：%~dp0
:: 设置变量存储当前路径
set parentFolder=%cd%

:: 验证mysql
cd /d  MySQL
call my_ini_edit.bat
:: start cmd /c mysql_servicestart.bat
call mysql_serviceinstall.bat <nul

:: 返回父级
echo =======mysql success
cd /d "%parentFolder%"
:: 验证java
cd /d  jre1.8.0_131/bin
echo 当前盘符和路径：%~dp0
java -version
:: 执行jar
::call java -jar "%parentFolder%"/gaois-0.0.1-SNAPSHOT.jar -n 3000 >nul
start java -jar "%parentFolder%"/gaois-0.0.1-SNAPSHOT.jar
echo ==========success========
:: echo 程序正在启动中......

::等待x秒
echo CreateObject("Scripting.FileSystemObject").DeleteFile(WScript.ScriptFullName) >%Temp%\Wait.vbs
echo wscript.sleep 15000 >>%Temp%\Wait.vbs
start /wait %Temp%\Wait.vbs
:: 打开浏览器
cd /d "%parentFolder%"/chrome49.0.2623.112
start chrome.exe http://localhost:8004/common/get_version



