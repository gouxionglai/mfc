# BAT

​	第一次使用bat，记录下常用的一些语法。

参考：

<https://blog.csdn.net/mycoolx/article/details/50251705>

<https://my.oschina.net/superkangning/blog/528881>

## 命令

### 注释

```bat
@echo off
::echo是用来原样输出的 off是说这个不输出
::一个冒号是定位使用。两个冒号是注释
::这是一个注释
```



### 当前路径

```bat
echo 当前盘符和路径：%~dp0
::%cd%也是代表当前路径
::但是%cd%会随着cmd执行的路径改变而改变。比如在c盘下打开cmd，执行d盘下的bat,路径就变成c:而不是d:
:: 设置变量存储当前路径,用于跳转目录使用
set parentFolder=%~dp0
```

### 跳转目录

```bat
:: 不能写成cd mysql  识别不了
cd /d  MySQL
```



### 隐藏窗口

```bat
::看不见cmd窗口
if "%1" == "h" goto begin 
mshta vbscript:createobject("wscript.shell").run("%~nx0 h",0)(window.close)&&exit 
:: /C 代表  /Close   执行完后关闭cmd
:: /K 代表 /Keep的简写   执行完后保留cmd
:: cmd /c  xxxxxx
:: cmd /k  xxxxxx
```



### 执行命令

```bat
::打开记事本
call notepad
::执行其他bat
::注意 <nul的使用， 因为执行安装mysql服务之后，会停在那，显示"点击任何按键继续"就不会执行后续bat了，
::<nul能实现向控制台在那个时候输入空，使其继续向下执行。
call mysql_serviceinstall.bat <nul

::call会等待执行完成
::start相当于另开一个线程（新窗口）执行，不用等待执行完成
start java -jar "%parentFolder%"/gaois-0.0.1-SNAPSHOT.jar
::以不可改变浏览器地址形式打开地址 --app=xxx
start chrome49.0.2623.112\chrome.exe --app=http://localhost:8004/common/get_version
```



### 输入输出

```text
>nul 表示把命令的执行结果提示信息输出到空设备中，而这个空设备就像黑洞一样，会吞噬所有进入它里面的提示信息，所以，效果就是：提示信息被屏蔽掉了；
<nul 表示从空设备中读入信息，以for /f 语句中的 set /p str=%%i<nul 之类的用法为常见
```



### 修改文件内容

因为不知道用户把exe安装在那个位置，所以需要动态修改指定文件即my.ini中 basedir 和datadir的路径

```bat
@echo off
set "pth=%~dp0"
set "pth=%pth:\=/%"
set "pth=%pth:~,-1%"
(for /f "tokens=1,2* delims=:=" %%a in ('findstr /n .* my.ini') do (
  if "%%c" neq "" (
    if /i not "%%b"=="basedir" (
      if /i not "%%b"=="datadir" (echo,%%b=%%c
      ) else echo,%%b="%pth%/Data/"
    ) else echo,%%b="%pth%"
  ) else echo,%%b
))>$
move $ my.ini
```



## 完整bat

startServer.bat

```bat
@echo off
echo 环境检测中......
echo 当前盘符和路径：%~dp0
:: 设置变量存储当前路径
set parentFolder=%~dp0

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
::15s
echo wscript.sleep 15000 >>%Temp%\Wait.vbs
start /wait %Temp%\Wait.vbs
:: 打开浏览器
cd /d "%parentFolder%"/chrome49.0.2623.112
start chrome.exe --app=http://localhost:8004/common/get_version

```



stopServer.bat

根据指定端口去找pid，关闭进程

netstat -ano|findstr "0.0.0.0:%port%"   正常是这样的，不需要^，放在bat里面就需要，可能是转义之类的吧

```bat
@echo off
@set port=8004
@echo %port%

for /f "tokens=1-5" %%i in ('netstat -ano^|findstr "0.0.0.0:%port%"') do taskkill /pid %%m  -f
```

