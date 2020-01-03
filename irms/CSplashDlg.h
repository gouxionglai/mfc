#pragma once


// CSplashDlg 对话框

class CSplashDlg : public CDialog
{
	DECLARE_DYNAMIC(CSplashDlg)

public:
	CSplashDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CSplashDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SPLASH};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	void RefreshControl(UINT uCtlID);

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	LRESULT OnProcName(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	CStatic m_StaticDeviceStat;
	CString transData;
};
