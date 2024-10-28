
// Punishment_Heart_AppDlg.h: 头文件
//

#pragma once


// CPunishmentHeartAppDlg 对话框
class CPunishmentHeartAppDlg : public CDialogEx
{
// 构造
public:
	CPunishmentHeartAppDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PUNISHMENT_HEART_APP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	void lcrPrint(CString text);
	CEdit mEdit1;
	CListBox mList1;
};
