// SoundRecorderDlg.h : header file
//

#pragma once

#include "record.h"


// CSoundRecorderDlg dialog
class CSoundRecorderDlg : public CDHtmlDialog
{
// Construction
public:
	void incTimer();
	CSoundRecorderDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SOUNDRECORDER_DIALOG, IDH = IDR_HTML_SOUNDRECORDER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
public:
	afx_msg void OnBnClickedButtonRecord();
private:
	int m_iHour;
	int m_iMinute;
	int m_iSecond;

	Record m_cRecord;
public:
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButtonOpenfile();
};
