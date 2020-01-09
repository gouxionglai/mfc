
// irmsDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "irms.h"
#include "irmsDlg.h"
#include "afxdialogex.h"
#include "tlhelp32.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CirmsDlg 对话框



CirmsDlg::CirmsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_IRMS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CirmsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CirmsDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SYSTEMTRAY, &CirmsDlg::OnSystemtray)
	ON_COMMAND(ID_32771, &CirmsDlg::OnStart)
	ON_COMMAND(ID_32772, &CirmsDlg::OnAbout)
	ON_COMMAND(ID_32773, &CirmsDlg::OnExit)
	ON_WM_SIZE()
END_MESSAGE_MAP()


//获取系统版本
CString getOSName()
{
	CString osName = _T("");
	int a = 0, b = 0, i = 0, j = 0;
	_asm
	{
		pushad
		mov ebx, fs: [0x18] ; get self pointer from TEB
		mov eax, fs: [0x30] ; get pointer to PEB / database
		mov ebx, [eax + 0A8h]; get OSMinorVersion
		mov eax, [eax + 0A4h]; get OSMajorVersion
		mov j, ebx
		mov i, eax
		popad
	}

	if ((i == 5) && (j == 0))
	{
		osName = _T("Windows2000");
	}
	else if ((i == 5) && (j == 1))
	{
		osName = _T("WindowsXP");
	}
	else if ((i == 5) && (j == 2))
	{
		osName = _T("Windows2003");
	}
	else if ((i == 6) && (j == 0))
	{
		osName = _T("WindowsVista");
	}
	else if ((i == 6) && (j == 1))
	{
		osName = _T("Windows7");
	}
	else if ((i == 6) && (j == 2))
	{
		osName = _T("Windows8");
	}
	else if ((i == 6) && (j == 3))
	{
		osName = _T("Windows8.1");
	}
	else if ((i == 10) && (j == 0))
	{
		osName = _T("Windows10");
	}
	else
	{
		osName = _T("当前系统低于Windows2000，或者高于Windows10，或者未知系统版本");
	}
	return osName;
}

//执行路径 + 执行参数， 返回进程pid
DWORD ExecuteCmd(CString exe, CString commond)
{
	STARTUPINFO   StartupInfo;//创建进程所需的信息结构变量    
	GetStartupInfo(&StartupInfo);
	StartupInfo.lpReserved = NULL;
	StartupInfo.lpDesktop = NULL;
	StartupInfo.lpTitle = NULL;
	StartupInfo.dwX = 0;
	StartupInfo.dwY = 0;
	StartupInfo.dwXSize = 0;
	StartupInfo.dwYSize = 0;
	StartupInfo.dwXCountChars = 500;
	StartupInfo.dwYCountChars = 500;
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow = SW_HIDE;
	//说明进程将以隐藏的方式在后台执行    
	StartupInfo.cbReserved2 = 0;
	StartupInfo.lpReserved2 = NULL;
	StartupInfo.hStdInput = stdin;
	StartupInfo.hStdOutput = stdout;
	StartupInfo.hStdError = stderr;
	PROCESS_INFORMATION   piProcess;
	BOOL   bRet;
	bRet = CreateProcess((LPCTSTR)exe, (LPTSTR)(LPCTSTR)commond, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &StartupInfo, &piProcess);
	if (bRet)
		CloseHandle(piProcess.hThread);
	CloseHandle(piProcess.hProcess);
	return piProcess.dwProcessId;
}


//执行命令，返回执行结果
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
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

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



// 普通的创建程序 和上面ExecuteCmd差别就在于不返回执行后内容
HANDLE StartProcess(LPCTSTR program, LPCTSTR args)
{
	HANDLE hProcess = NULL;
	PROCESS_INFORMATION processInfo;
	STARTUPINFO startupInfo;
	::ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	if (::CreateProcess(program, (LPTSTR)args,
		NULL,  // process security
		NULL,  // thread security
		FALSE, // no inheritance
		0,     // no startup flags
		NULL,  // no special environment
		NULL,  // default startup directory
		&startupInfo,
		&processInfo))
		return hProcess;
}

//隐藏任务栏图标  未使用
BOOL ShowInTaskbar(HWND hWnd, BOOL bShow)
{
	HRESULT hr;
	ITaskbarList* pTaskbarList;
	hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER,
		IID_ITaskbarList, (void**)&pTaskbarList);

	if (SUCCEEDED(hr))
	{
		pTaskbarList->HrInit();
		if (bShow)
			pTaskbarList->AddTab(hWnd);
		else
			pTaskbarList->DeleteTab(hWnd);

		pTaskbarList->Release();
		return TRUE;
	}

	return FALSE;
}




typedef struct tagWNDINFO
{
	DWORD dwProcessId;
	HWND hWnd;
} WNDINFO, * LPWNDINFO;

BOOL CALLBACK YourEnumProc(HWND hWnd, LPARAM lParam)
{
	DWORD dwProcessId;
	GetWindowThreadProcessId(hWnd, &dwProcessId);
	LPWNDINFO pInfo = (LPWNDINFO)lParam;

	if (dwProcessId == pInfo->dwProcessId)
	{

		bool isWindowVisible = IsWindowVisible(hWnd);

		if (isWindowVisible == true)

		{

			pInfo->hWnd = hWnd;

			return FALSE;

		}

	}
	return TRUE;
}

HWND GetProcessMainWnd(DWORD dwProcessId)
{
	WNDINFO wi;
	wi.dwProcessId = dwProcessId;
	wi.hWnd = NULL;
	EnumWindows(YourEnumProc, (LPARAM)&wi);
	return wi.hWnd;
}




// CirmsDlg 消息处理程序
BOOL CirmsDlg::OnInitDialog()
{
	//执行命令

	//1.检查java
	((CirmsApp*)AfxGetApp())->pSplashThread->log("检查运行环境中......");
	CString str1 = _T("cmd /c java -version");
	CString temp = ExecuteCmd2(str1);

	//MessageBox(temp);
	//ShellExecute(NULL, NULL, _T("cmd.exe"), _T("jre1.8.0_131\\bin java -version"), NULL, SW_NORMAL);
	//2.检查mysql
	//bin\mysqladmin --no-defaults -u root shutdown -proot
	//GetDlgItem(IDC_INFO)->SetWindowText("检测mysql环境中...");
	((CirmsApp*)AfxGetApp())->pSplashThread->log("检查DB环境中......");
	//ShellExecute(NULL, NULL, _T("test.bat"), NULL, NULL, SW_SHOWMAXIMIZED);
	ShellExecute(NULL, NULL, _T("checkmysql.bat"), NULL, NULL, SW_HIDE);
	//CString str2 = _T("test.bat");
	//temp = ExecuteCmd(str2, NULL);
	/*CString str2 = _T("cmd /c MySQL\\bin\\mysqladmin --no-defaults -u root shutdown -proot");
	temp = ExecuteCmd(str2, NULL);

	str2 = _T("cmd /c bin\\mysqld-nt --defaults-file=my.ini");
	temp = ExecuteCmd(str2, "MySQL");*/
	//MessageBox(temp);
	//3.启动jar
	//GetDlgItem(IDC_INFO)->SetWindowText("启动服务中...");
	//((CirmsApp*)AfxGetApp())->pSplashThread->log("启动服务中......");
	//CString str3 = _T("cmd /c jre1.8.0_131\\bin\\java -jar irms-0.0.1-SNAPSHOT.jar");
	ShellExecute(NULL, NULL, _T("calljar.bat"), NULL, NULL, SW_HIDE);
	//temp = ExecuteCmd(str3, NULL);
	//4.检测jar是否启动成功
	((CirmsApp*)AfxGetApp())->pSplashThread->log("检查服务是否已启动......");
	CString str4 = _T("cmd /c netstat -ano | findstr 0.0.0.0:9009");
	////MessageBox(temp);
	CString res;
	int i = 0;
	////启动30秒如果还是没启动 则抛错
	while (i<50)
	{
		res = ExecuteCmd2(str4);
		DWORD le0 = res.GetLength();
		if (le0 > 0)
		{
			break;
		}
		//1s请求一次
		if (i%3==0) 
		{
			((CirmsApp*)AfxGetApp())->pSplashThread->log("启动服务中");
		}
		else if (i%3==1) 
		{
			((CirmsApp*)AfxGetApp())->pSplashThread->log("启动服务中...");
		}
		else 
		{
			((CirmsApp*)AfxGetApp())->pSplashThread->log("启动服务中......");
		}
		Sleep(1000);
		i++;
	}

	//Sleep(3000);
	CDialogEx::OnInitDialog();
	//隐藏主窗口
	//SetWindowPos(&CWnd::wndNoTopMost, 0, 0, 0, 0, SWP_HIDEWINDOW);
	////(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	//ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);

	//系统托盘
	NotifyIcon.cbSize = sizeof(NOTIFYICONDATA);
	//NotifyIcon.hIcon=AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	NotifyIcon.hIcon = m_hIcon;  //上面那句也可以
	NotifyIcon.hWnd = m_hWnd;
	lstrcpy(NotifyIcon.szTip, _T("irms"));
	NotifyIcon.uCallbackMessage = WM_SYSTEMTRAY;
	NotifyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	Shell_NotifyIcon(NIM_ADD, &NotifyIcon);   //添加系统托盘

	if (i >=50) 
	{
		AfxMessageBox(_T("启动程序失败，请退出程序重试..."));
		((CirmsApp*)AfxGetApp())->pSplashThread->log("启动程序失败, 请退出程序重试.");
		//return false;
	}
	else 
	{
		((CirmsApp*)AfxGetApp())->pSplashThread->log("服务启动成功......");
		//出现主窗口的时候就隐藏启动界面
		if (((CirmsApp*)AfxGetApp())->pSplashThread != NULL)
			((CirmsApp*)AfxGetApp())->pSplashThread->HideSplash();
		//5.打开浏览器
		OnStart();
		ShowWindow(SW_MAXIMIZE);
	}


	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	CRect rect;
	GetClientRect(&rect);//取客户区大小
	Old.x = rect.right - rect.left;
	Old.y = rect.bottom - rect.top;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CirmsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CirmsDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CirmsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

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
		case  WM_LBUTTONDOWN://左键单击的处理     
		{
			ModifyStyleEx(0, WS_EX_TOPMOST);   //可以改变窗口的显示风格
			ShowWindow(SW_SHOWNORMAL);
		}
		break;
	}
	return 0;
}



//     ---------------成拆分数组-------

UINT DivStr(CString str, CStringArray& Arr, CString strDiv)
{
	int nFindposi = str.Find(strDiv);
	if (nFindposi < 0)
		return 0;

	while (nFindposi > 0)
	{
		Arr.Add(str.Left(nFindposi));
		str = str.Right(str.GetLength() - nFindposi - 1);
		str.TrimLeft(strDiv);    //warning

		nFindposi = str.Find(strDiv);
	}

	if (!str.IsEmpty())
		Arr.Add(str);

	return Arr.GetSize();
}

//-------------------------获取pid----------------------
CString getPid(CString p)
{
	// TODO: 在此添加控件通知处理程序代码
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


void CirmsDlg::OnStart()
{
	// TODO: 在此添加命令处理程序代码
	//打开浏览器
	CString osName = getOSName();
	//CString param = "--user-data-dir=tmp --no-sandbox --disk-cache-dir=cache --disable-translate --start-maximized --user-agent=irmswkwkwk --app=http://127.0.0.1:9009/api/index.html";
	CString param = "--user-data-dir=tmp --disk-cache-dir=cache --disable-translate --start-maximized --user-agent=irms_";
	CString param2 = " --kiosk http://127.0.0.1:9009/api/index.html";
	CString finalString = param + osName + param2;
	//ShellExecute(NULL, NULL, _T("chrome.exe"), _T(finalString), _T("chrome49.0.2623.112"), SW_SHOWMAXIMIZED);

	//PROCESS_INFORMATION processInfo;
	//STARTUPINFO startupInfo;
	//::ZeroMemory(&startupInfo, sizeof(startupInfo));
	//startupInfo.cb = sizeof(startupInfo);
	//startupInfo.wShowWindow = SW_HIDE;
	//startupInfo.dwFlags = STARTF_USESHOWWINDOW;
	//startupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	//CreateProcess((LPCTSTR)"D:\\CRT\\SecureCRT.exe", (LPTSTR)(LPCTSTR)finalString,
	//	NULL,  // process security
	//	NULL,  // thread security
	//	FALSE, // no inheritance
	//	0,     // no startup flags
	//	NULL,  // no special environment
	//	NULL,  // default startup directory
	//	&startupInfo,
	//	&processInfo);
	DWORD dword = ExecuteCmd("chrome49.0.2623.112\\chrome.exe", finalString);
	ModifyStyleEx(dword, WS_EX_APPWINDOW, 0);
	//StartProcess(NULL, "notepad");

	UpdateData(TRUE);
	HWND hwnd = NULL;
	CRect rc;
	//GetDlgItem(IDC_CRT)->GetClientRect(&rc);
	GetDlgItem(IDC_STATIC_SHOW)->GetWindowRect(&rc);
	//GetClientRect(&rc);
	int nWidth = rc.Width();
	int nHeight = rc.Height();
	ScreenToClient(rc);

	while (!hwnd)
	{
		hwnd = GetProcessMainWnd(dword);
		//hwnd = GetProcessMainWnd(processInfo.dwProcessId);
		//hwnd = ::FindWindow(NULL, "无标题 - 记事本");
		Sleep(500);
	}
	if (hwnd)
	{
		LONG style = GetWindowLong(hwnd, GWL_STYLE);// 14CF 0000

		style &= ~WS_CAPTION;
		style &= ~WS_THICKFRAME;
		//style |=WS_CHILD;
		SetWindowLong(hwnd, GWL_STYLE, style);

		::SetParent(hwnd, this->m_hWnd);
		//::MoveWindow(hwnd, rc.left, rc.top, nWidth, nHeight, true); //将外部程序移到自自身窗口里		
		::SetWindowPos(hwnd, HWND_TOP, rc.left, rc.top, nWidth, nHeight, SWP_SHOWWINDOW | SWP_HIDEWINDOW);
		//::BringWindowToTop(hwnd);
		//Invalidate();
		//::UpdateWindow(hwnd);
		::ShowWindow(hwnd, SW_SHOW);
	}
}

//关于
void CirmsDlg::OnAbout()
{
	// TODO: 在此添加命令处理程序代码
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

//根据name去关闭进程
//void CloseProgram(CString strProgram)
//{
//	HANDLE handle; //定义CreateToolhelp32Snapshot系统快照句柄
//	HANDLE handle1; //定义要结束进程句柄
//	handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//获得系统快照句柄
//	PROCESSENTRY32* info; //定义PROCESSENTRY32结构字指
//	//PROCESSENTRY32 结构的 dwSize 成员设置成 sizeof(PROCESSENTRY32)
//
//	info = new PROCESSENTRY32;
//	info->dwSize = sizeof(PROCESSENTRY32);
//	//调用一次 Process32First 函数，从快照中获取进程列表
//	Process32First(handle, info);
//	//重复调用 Process32Next，直到函数返回 FALSE 为止
//	while (Process32Next(handle, info) != FALSE)
//	{
//		CString strTmp = info->szExeFile;     //指向进程名字  
//		if (strProgram.CompareNoCase(info->szExeFile) == 0)
//		{
//			//PROCESS_TERMINATE表示为结束操作打开,FALSE=可继承,info->th32ProcessID=进程ID   
//			handle1 = OpenProcess(PROCESS_TERMINATE, FALSE, info->th32ProcessID);
//			//结束进程   
//			TerminateProcess(handle1, 0);
//		}
//	}
//	delete info;
//
//	CloseHandle(handle);
//}

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

//重新关闭
BOOL CirmsDlg::DestroyWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	Shell_NotifyIcon(NIM_DELETE, &NotifyIcon);//消除托盘图标
	return CDialogEx::DestroyWindow();
}

void CirmsDlg::OnCancel()  //点击X 按钮，最小化到系统托盘
{
	// TODO: 在此添加专用代码和/或调用基类
	this->ShowWindow(HIDE_WINDOW);
	//CDialogEx::OnCancel();
}


void CirmsDlg::OnExit()
{
	// TODO: 在此添加命令处理程序代码
	// 先关闭jar
	//CloseProgram("java.exe");
	CString str = _T("cmd /c netstat -ano | findstr 0.0.0.0:9009");
	//CString str = _T("cmd.exe %s inconfig %s");
	//LPTSTR szCmdline = _tcsdup(TEXT(str));
	//TCHAR* p = (LPTSTR)(LPCTSTR)str;
	CString pid = getPid(str);

	//杀进程
	KillProcessById(_ttoi(pid));
	//关闭窗口
	DestroyWindow();
}


void CirmsDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED) {
		//调整所有控件
		resize();
	}
}


void CirmsDlg::resize()
{
	float fsp[2];
	POINT Newp; //获取现在对话框的大小
	CRect recta;
	GetClientRect(&recta);     //取客户区大小  
	Newp.x = recta.right - recta.left;
	Newp.y = recta.bottom - recta.top;
	fsp[0] = (float)Newp.x / Old.x;
	fsp[1] = (float)Newp.y / Old.y;
	CRect Rect;
	int woc;
	CPoint OldTLPoint, TLPoint; //左上角
	CPoint OldBRPoint, BRPoint; //右下角
	HWND  hwndChild = ::GetWindow(m_hWnd, GW_CHILD);  //列出所有控件  
	while (hwndChild)
	{
		woc = ::GetDlgCtrlID(hwndChild);//取得ID
		GetDlgItem(woc)->GetWindowRect(Rect);
		ScreenToClient(Rect);
		OldTLPoint = Rect.TopLeft();
		TLPoint.x = long(OldTLPoint.x * fsp[0]);
		TLPoint.y = long(OldTLPoint.y * fsp[1]);
		OldBRPoint = Rect.BottomRight();
		BRPoint.x = long(OldBRPoint.x * fsp[0]);
		BRPoint.y = long(OldBRPoint.y * fsp[1]);
		Rect.SetRect(TLPoint, BRPoint);
		GetDlgItem(woc)->MoveWindow(Rect, TRUE);
		hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
	}
	Old = Newp;

}