@echo off
::����cmd����
::if "%1" == "h" goto begin 
::mshta vbscript:createobject("wscript.shell").run("%~nx0 h",0)(window.close)&&exit 
:::begin
echo ���������......
echo ��ǰ�̷���·����%~dp0
:: ���ñ����洢��ǰ·��
set parentFolder=%cd%

:: ��֤mysql
cd /d  MySQL
call my_ini_edit.bat
:: start cmd /c mysql_servicestart.bat
call mysql_serviceinstall.bat <nul

:: ���ظ���
echo =======mysql success
cd /d "%parentFolder%"
:: ��֤java
cd /d  jre1.8.0_131/bin
echo ��ǰ�̷���·����%~dp0
java -version
:: ִ��jar
::call java -jar "%parentFolder%"/gaois-0.0.1-SNAPSHOT.jar -n 3000 >nul
start java -jar "%parentFolder%"/gaois-0.0.1-SNAPSHOT.jar
echo ==========success========
:: echo ��������������......

::�ȴ�x��
echo CreateObject("Scripting.FileSystemObject").DeleteFile(WScript.ScriptFullName) >%Temp%\Wait.vbs
echo wscript.sleep 15000 >>%Temp%\Wait.vbs
start /wait %Temp%\Wait.vbs
:: �������
cd /d "%parentFolder%"/chrome49.0.2623.112
start chrome.exe http://localhost:8004/common/get_version



