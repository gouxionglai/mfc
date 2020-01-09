
// irmsDlg.h: 头文件
//

#pragma once

#define   WM_SYSTEMTRAY WM_USER+100

// CirmsDlg 对话框
class CirmsDlg : public CDialogEx
{
// 构造
public:
	CirmsDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IRMS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
private:
	//托盘
	NOTIFYICONDATA NotifyIcon;

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnSystemtray(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnStart();
	afx_msg void OnAbout();
	afx_msg void OnExit();
	//根据进程id关闭进程
	BOOL KillProcessById(DWORD pID);
	BOOL DestroyWindow();
	void OnCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	POINT Old;
	void resize();
};
