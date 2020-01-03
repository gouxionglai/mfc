// CSplashDlg.cpp: 实现文件
//

#include "pch.h"
#include "irms.h"
#include "CSplashDlg.h"
#include "afxdialogex.h"


// CSplashDlg 对话框

IMPLEMENT_DYNAMIC(CSplashDlg, CDialog)

CSplashDlg::CSplashDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_SPLASH, pParent)
{

}

CSplashDlg::~CSplashDlg()
{
}

void CSplashDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INFO, m_StaticDeviceStat);
}


BEGIN_MESSAGE_MAP(CSplashDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_USERMESSAGE, OnProcName)
END_MESSAGE_MAP()


// CSplashDlg 消息处理程序


void CSplashDlg::RefreshControl(UINT uCtlID)
{
	CRect rc;
	GetDlgItem(uCtlID)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	InvalidateRect(rc);
}


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


LRESULT CSplashDlg::OnProcName(WPARAM wParam, LPARAM lParam)
{
	// TODO: 处理用户自定义消息
	CString* str = (CString*)lParam;
	//SetDlgItemText(IDC_INFO, *str);
	//GetDlgItem(IDC_INFO)->SetWindowText("Hello Linux");
	GetDlgItem(IDC_INFO)->SetWindowText(*str);
	RefreshControl(IDC_INFO);

	return 0;

}

typedef struct {
	CString srcString;
	CString DesString;
	CWnd* hander;
}Param;