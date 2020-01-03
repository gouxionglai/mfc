#pragma once


#include "CSplashDlg.h"
// CSplashThread
#define	WM_USERMESSAGE WM_USER+100
class CSplashThread : public CWinThread
{
	DECLARE_DYNCREATE(CSplashThread)

protected:
	CSplashThread();           // 动态创建所使用的受保护的构造函数
	virtual ~CSplashThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	void HideSplash();

	void log(CString str);

protected:
	DECLARE_MESSAGE_MAP();
	CSplashDlg* m_pSplashDlg;
};


