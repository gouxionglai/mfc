# MFC

​	记录下遇到的一些功能点及其实现方式

## 启动动画

参考：

<https://blog.csdn.net/qq_33012981/article/details/80716876>

<http://blog.sina.com.cn/s/blog_63a881060102w01h.html>

两种实现方式：

1. 新建MFC工程，将里面默认的窗口设置为启动动画的窗口。
   1. 缺点：如果要在上面增加字体控件，并且动态修改，而且想已启动就加载。会导致图片在最后加载UI线程，而其他的检验已经做完了。所以一开始看到的是空白窗口。
2. 新建MFC工程，新建一个窗口，将其作为启动窗口。
   1. 优势：新开线程先显示启动动画窗口（已经加载完毕）
   2. 主线程执行一些校验，并且把步骤动态反馈到启动动画（局部修改）
   3. 启动完成后，隐藏启动动画，隐藏主窗口，加载系统托盘。



```c++
//另开线程加载启动界面
BOOL CSplashThread::InitInstance()
{
	// TODO:    在此执行任意逐线程初始化
    ::AttachThreadInput(m_nThreadID, AfxGetApp()->m_nThreadID, TRUE);

    //:通常系统内的每个线程都有自己的输入队列。本函数允许线程和进程共享输入队列。连接了线程后，输入焦点、窗口激活、鼠标捕获、键盘状态以及输入队列状态都会进入共享状态 . (这个函数可以不用)
    m_pSplashDlg = new CSplashDlg;
    //m_pSplashDlg->SetEnable(true);
    m_pSplashDlg->Create(IDD_SPLASH);
    m_pSplashDlg->ShowWindow(SW_SHOW);

    //原本想在这里执行检查java,mysql,启动jar包等流程，但是不能实现动态修改启动动画的进度内容
 //   CString str1 = _T("cmd /c jre1.8.0_131\\bin\\java -version");
 //   CString temp = ExecuteCmd2(str1, NULL);
	//::SendMessage(m_pSplashDlg->m_hWnd, WM_USERMESSAGE, 0, (LPARAM)&temp);
	return TRUE;
}
```



## 隐藏窗口

```c++
//因为当前是在主线程，想要获取其他窗口实例 使用这个办法
if (((CirmsApp*)AfxGetApp())->pSplashThread != NULL)
	((CirmsApp*)AfxGetApp())->pSplashThread->HideSplash();	//调用pSplashThread的HideSplash方法  其本质是
//m_pSplashDlg->SendMessage(WM_CLOSE);
```



## 禁止重复启动

```c++
//主线程的InitInstance中设置
BOOL CirmsApp::InitInstance()
{
	//防止重复启动
	HANDLE hObject = CreateMutex(NULL, FALSE, _T("CReadOracleDBApp"));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hObject);
		return FALSE;
	}

	//启动动画 就是调用上面的启动动画线程
	pSplashThread = (CSplashThread*)AfxBeginThread(
		RUNTIME_CLASS(CSplashThread),
		THREAD_PRIORITY_NORMAL,
		0, CREATE_SUSPENDED);
	ASSERT(pSplashThread->IsKindOf(RUNTIME_CLASS(CSplashThread)));
	pSplashThread->ResumeThread();
	//Sleep(3000);
    //以下是新建项目系统自动生成的一些东西
    ....
}
```



## 执行程序

执行程序有两种CreateProcess 和 ShellExecute

### CreateProcess 

可以建立通道，将cmd执行的结果返回回来

这里我犯过一个错误，私以为像java一样，传递参数即可，因为CreateProcess的倒数第三个参数代表要执行的目录，我用CString ExecuteCmd2(CString cmdline,CString path)这样传递进去，结果反而执行不了。

但是思想应该是对的，估计是语法不对吧。

具体自行搜索CreateProcess 使用

```c++
//执行命令，返回执行结果
//核心是CreateProcess
CString ExecuteCmd2(CString cmdline)
{
	SECURITY_ATTRIBUTES sa;
	HANDLE hRead, hWrite;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		return NULL;
	}
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;
	si.wShowWindow = SW_HIDE;	//很多可选值，比如最大化显示，正常显示，不显示之类的
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	//转换成LPTSTR
	if (!CreateProcess(NULL, (LPTSTR)(LPCTSTR)cmdline, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		return NULL;
	}
	CloseHandle(hWrite);

	char buffer[4096];
	memset(buffer, 0, 4096);
	CString output;
	DWORD byteRead;
	while (true)
	{
		if (ReadFile(hRead, buffer, 4095, &byteRead, NULL) == NULL)
		{
			break;
		}
		output += buffer;
	}
	return output;
}
```

建立通道有个好处，可以获取端口是否已启动的状态

```c++
	//启动30秒如果还是没启动 则抛错
	while (i<10)
	{
		res = ExecuteCmd2(str4);
		DWORD le0 = res.GetLength();
		if (le0 > 0)
		{
			break;
		}
		//1s请求一次
		Sleep(3000);
		i++;
	}
```



### ShellExecute

```c++
//执行exe 并且带参数执行
ShellExecute(NULL, NULL, _T("chrome.exe"), _T("--app=http://localhost:9009/api"), _T("chrome49.0.2623.112"), SW_SHOWMAXIMIZED);
//执行bat
ShellExecute(NULL, NULL, _T("checkmysql.bat"), NULL, NULL, SW_HIDE);
```





## 消息机制

参考：

<https://blog.csdn.net/xiaoding133/article/details/8718332>

能够实现窗口A的动作，发送消息给窗口B，使其做一些事，比如我这里主线程让启动动画窗口改变文本内容。

//窗口不能跨越执行，就好像java里面的封装性质，一个窗口不能直接操作另一个窗口的东西。

```c++
//核心是 SendMessage + WM_USERMESSAGE
// 有 SendMessage + PostMessage 
// WM_USERMESSAGE充当了一个传递介质的功能
::SendMessage(m_pSplashDlg->m_hWnd, WM_USERMESSAGE, 0, (LPARAM)&str);
```





## 文本控件内容堆叠

参考：

<http://blog.sina.com.cn/s/blog_5eb73de10100dt5l.html>

比如使用GetDlgItem(IDC_INFO)->SetWindowText(*str); 目的是改变文本框内容

多次调用后，是累计的状态，而不是更新。显示效果就是之前的内容和最新的内容重叠在一起了。

```c++
//动态改变文本字体 颜色 背景等
HBRUSH CSplashDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (pWnd->GetDlgCtrlID() == IDC_INFO)
	{
		pDC->SetTextColor(RGB(255, 255, 255));//设置控件字体颜色
		pDC->SetBkMode(TRANSPARENT);//设置透明属性
		return (HBRUSH)GetStockObject(NULL_BRUSH);//返回空画刷
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}
```

```c++
//解决重叠问题，更新字体内容
void CSplashDlg::RefreshControl(UINT uCtlID)
{
	CRect rc;
	GetDlgItem(uCtlID)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	InvalidateRect(rc);
}
```

```c++
//调用
GetDlgItem(IDC_INFO)->SetWindowText(*str); 
RefreshControl(IDC_STATIC); //改成自己控件的id
```



## 程序托盘

参考：

<https://blog.csdn.net/u013051748/article/details/45621937>

需要自己在视图里面添加menu，然后给menu绑定事件

```c++
//点击菜单事件
afx_msg LRESULT CirmsDlg::OnSystemtray(WPARAM wParam, LPARAM lParam)
{
	//wParam接收的是图标的ID，而lParam接收的是鼠标的行为
//  if(wParam!=IDR_MAINFRAME)     
//      return    1;     
	switch (lParam)
	{
		case  WM_RBUTTONDOWN://右键起来时弹出快捷菜单
		{
			CMenu menu;
			menu.LoadMenu(IDR_MENU1);
			CMenu* pPopUp = menu.GetSubMenu(0);
			CPoint pt;
			GetCursorPos(&pt);
			SetForegroundWindow();		//顶层显示主窗口
			pPopUp->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);//确定弹出式菜单的位置		
			HMENU hmenu = menu.Detach();	//资源回收	
			menu.DestroyMenu();
		}
		break;
		//case  WM_LBUTTONDOWN://左键单击的处理     
		//{
		//	ModifyStyleEx(0, WS_EX_TOPMOST);   //可以改变窗口的显示风格
		//	ShowWindow(SW_SHOWNORMAL);
		//}
		//break;
	}
	return 0;
}
```

加载托盘

```c++
//
BOOL CirmsDlg::OnInitDialog()
{
    //do something
    
	CDialogEx::OnInitDialog();
	//隐藏主窗口
	SetWindowPos(&CWnd::wndNoTopMost, 0, 0, 0, 0, SWP_HIDEWINDOW);
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);

	//系统托盘
	NotifyIcon.cbSize = sizeof(NOTIFYICONDATA);
	//NotifyIcon.hIcon=AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	NotifyIcon.hIcon = m_hIcon;  //上面那句也可以
	NotifyIcon.hWnd = m_hWnd;
	lstrcpy(NotifyIcon.szTip, _T("irms"));
	NotifyIcon.uCallbackMessage = WM_SYSTEMTRAY;
	NotifyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	Shell_NotifyIcon(NIM_ADD, &NotifyIcon);   //添加系统托盘
  //do something
}
```



## 关闭进程

思想：根据pid或者进程名关闭进程。  //进程名范围太大了，比如我启动了两个server,  在控制台看到的都是java.exe，  如果关闭就全部关了。但是两个java.exe的pid肯定不一样。



```c++
//根据name去关闭进程
void CloseProgram(CString strProgram)
{
	HANDLE handle; //定义CreateToolhelp32Snapshot系统快照句柄
	HANDLE handle1; //定义要结束进程句柄
	handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//获得系统快照句柄
	PROCESSENTRY32* info; //定义PROCESSENTRY32结构字指
	//PROCESSENTRY32 结构的 dwSize 成员设置成 sizeof(PROCESSENTRY32)

	info = new PROCESSENTRY32;
	info->dwSize = sizeof(PROCESSENTRY32);
	//调用一次 Process32First 函数，从快照中获取进程列表
	Process32First(handle, info);
	//重复调用 Process32Next，直到函数返回 FALSE 为止
	while (Process32Next(handle, info) != FALSE)
	{
		CString strTmp = info->szExeFile;     //指向进程名字  
		if (strProgram.CompareNoCase(info->szExeFile) == 0)
		{
			//PROCESS_TERMINATE表示为结束操作打开,FALSE=可继承,info->th32ProcessID=进程ID   
			handle1 = OpenProcess(PROCESS_TERMINATE, FALSE, info->th32ProcessID);
			//结束进程   
			TerminateProcess(handle1, 0);
		}
	}
	delete info;

	CloseHandle(handle);
}
```



获取pid 关闭进程

```c++
//-------------------------获取pid----------------------
CString getPid(CString p)
{
	// TODO: 在此添加控件通知处理程序代码
    //调用前面的ExecuteCmd2  获取pid
	CString str = ExecuteCmd2(p);//_T("i am a student");
	//CString str = _T("  TCP 0.0.0.0:8008           0.0.0.0:0              LISTENING       22040        ");
	str.TrimRight();//去掉右边的空格
	str.TrimLeft();//去掉左边的空格
	CStringArray strArr;
	//空格替换成其他分隔符
	CString displit=",";
	str.Replace(" ", displit);
	//CString转CStringArray
	if (DivStr(str, strArr, displit) <= 0)
	{
		//AfxMessageBox(_T("数组为空!"));
		return 0;
	}
	//CStringArray转CString
	int index = strArr.GetSize();
	//AfxMessageBox(strArr[strArr.GetSize() - 1]);
	return strArr[strArr.GetSize() - 1];
}

//根据pid关闭进程
BOOL CirmsDlg::KillProcessById(DWORD pID)
{//由进程的ID，结束进程的函数
	HANDLE Hwnd;
	bool ret = FALSE;
	Hwnd = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, 0, pID);//得到句柄
	if (Hwnd)
	{
		if (TerminateProcess(Hwnd, 0))
		{
			ret = true;
		}
	}
	return ret;
}

//调用
	//CloseProgram("java.exe");
	CString str = _T("cmd /c netstat -ano | findstr 0.0.0.0:9009");
	//CString str = _T("cmd.exe %s inconfig %s");
	//LPTSTR szCmdline = _tcsdup(TEXT(str));
	//TCHAR* p = (LPTSTR)(LPCTSTR)str;
	CString pid = getPid(str);

	KillProcessById(_ttoi(pid));
```



