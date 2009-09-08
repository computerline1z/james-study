// SoundRecorderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SoundRecorder.h"
#include "SoundRecorderDlg.h"
#include ".\soundrecorderdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_TIMER_RECORD 1001

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CSoundRecorderDlg dialog

BEGIN_DHTML_EVENT_MAP(CSoundRecorderDlg)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()


CSoundRecorderDlg::CSoundRecorderDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CSoundRecorderDlg::IDD, CSoundRecorderDlg::IDH, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSoundRecorderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSoundRecorderDlg, CDHtmlDialog)
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_RECORD, OnBnClickedButtonRecord)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnBnClickedButtonPlay)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_OPENFILE, OnBnClickedButtonOpenfile)
END_MESSAGE_MAP()


// CSoundRecorderDlg message handlers

BOOL CSoundRecorderDlg::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CString sFile = _T("recording01.wav");
	GetDlgItem(IDC_EDIT_FILE)->SetWindowText(sFile);

    // Init DirectSound
	HRESULT hr;
    if( FAILED( hr = m_cRecord.InitDirectSound() ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("InitDirectSound"), hr );
        PostQuitMessage( 0 );       
    }

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSoundRecorderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDHtmlDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSoundRecorderDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDHtmlDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSoundRecorderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
HRESULT CSoundRecorderDlg::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT CSoundRecorderDlg::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}

void CSoundRecorderDlg::OnBnClickedButtonRecord()
{
	// TODO: Add your control notification handler code here
	CString sFile;
	GetDlgItem(IDC_EDIT_FILE)->GetWindowText(sFile);

    DWORD dwindex;
	dwindex=1;
	
	/*paul code here

	sam/sec: nSamplesPerSec -
	bit/sam: wBitsPerSample -
	channel: nChannels	    -

	index  sam/sec	 bit/sam channel
 	0		44100		8		1
	1		44100		16		1
	2		44100		8		2
	3		44100		16		2
	4		22050		8		1
	5		22050		16		1
	6		22050		8		2
	7		22050		16		2
	8		11025		8		1
	9		11025		16		1
	10		11025		8		2
	11		11025		16		2
	12		8000		8		1
	13		8000		16		1
	14		8000		8		2
	15		8000		16		2
		
	*/
	
	m_cRecord.Format(dwindex);

	TCHAR szFile[MAX_PATH];
	_tcscpy(szFile, sFile);
	m_cRecord.SetRecordedFile(szFile);
	m_cRecord.StartRecord();
	SetTimer(ID_TIMER_RECORD, 1000, NULL);
	m_iSecond = -1;
	m_iMinute = 0;
	m_iHour = 0;
	OnTimer(ID_TIMER_RECORD);

	GetDlgItem(IDC_BUTTON_RECORD)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(TRUE);
}

void CSoundRecorderDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	m_cRecord.RecordCapturedData();

	incTimer();

	CString s;
	s.Format(_T("%02d:%02d:%02d"), m_iHour, m_iMinute, m_iSecond);
	GetDlgItem(IDC_STATIC_STATUS)->SetWindowText(s);	

	CDHtmlDialog::OnTimer(nIDEvent);
}

void CSoundRecorderDlg::OnBnClickedButtonStop()
{
	// TODO: Add your control notification handler code here
	KillTimer(ID_TIMER_RECORD);
    // Stop the capture and read any data that was not caught by a notification
    m_cRecord.StopRecord(  );

	GetDlgItem(IDC_BUTTON_RECORD)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_PLAY)->EnableWindow(TRUE);
}

void CSoundRecorderDlg::OnBnClickedButtonPlay()
{
	// TODO: Add your control notification handler code here
	CString sFile;
	GetDlgItem(IDC_EDIT_FILE)->GetWindowText(sFile);
	ShellExecute(this->m_hWnd, _T("Open"), sFile, NULL, NULL, SW_SHOW);
}

void CSoundRecorderDlg::OnDestroy()
{
	CDHtmlDialog::OnDestroy();

	// TODO: Add your message handler code here
    // Clean up everything
    m_cRecord.FreeDirectSound();
}

BOOL SaveFileDialog(HWND hWnd, LPTSTR pszName, int cb, TCHAR *szFilter)
{

	TCHAR *l_szFilter;

	if (szFilter == NULL)
		l_szFilter = TEXT("XML File\0*.xml;\0All Files\0*.*\0\0") ;  
	else
		l_szFilter = szFilter;

	// Get real path
	CFile file;
	if (file.Open(pszName, CFile::modeRead))
	{
		_tcscpy(pszName, file.GetFilePath());
		//strcpy(pszName, file.GetFileName());
		file.Close();
	}

	static OPENFILENAME ofn;
	LPTSTR p;
	TCHAR  szFileName[_MAX_PATH];
	TCHAR  szBuffer[_MAX_PATH] ;

	if(pszName == NULL || cb <= 0)
		return FALSE;

	// start with capture file as current file name
	szFileName[0] = 0;
	//lstrcpyn(szFileName, gcap.szCaptureFile, NUMELMS(szFileName));

	// Get just the path info
	// Terminate the full path at the last backslash
	//lstrcpyn(szBuffer, szFileName, NUMELMS(szBuffer));
	lstrcpyn(szBuffer, pszName, MAX_PATH);
	for(p = szBuffer + lstrlen(szBuffer); p > szBuffer; p--)
	{
		if(*p == '\\')
		{
			_tcscpy(szFileName, p+1);
			*(p+1) = '\0';
			break;
		}
	}
	szBuffer[_MAX_PATH-1] = 0;  // Null-terminate

	ZeroMemory(&ofn, sizeof(OPENFILENAME)) ;
	ofn.lStructSize   = sizeof(OPENFILENAME) ;
	ofn.hwndOwner     = hWnd ;
	ofn.lpstrFilter   = l_szFilter;//TEXT("XML File\0*.xml;\0All Files\0*.*\0\0");
	ofn.nFilterIndex  = 0 ;
	ofn.lpstrFile     = szFileName;
	ofn.nMaxFile      = sizeof(szFileName) ;
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrTitle    = TEXT("Save File");
	ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir = szBuffer;
	ofn.Flags = OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST ;

	TCHAR szCurDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szCurDir);
	if(GetSaveFileName((LPOPENFILENAME)&ofn))
	{
		// We have a capture file name
		lstrcpyn(pszName, szFileName, cb);

		//GetCurrentDirectory(MAX_PATH, szCurDir);
		SetCurrentDirectory(szCurDir);

		return TRUE;
	}
	else
	{
		GetCurrentDirectory(MAX_PATH, szCurDir);

		return FALSE;
	}
}

void CSoundRecorderDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
}

void CSoundRecorderDlg::OnBnClickedButtonOpenfile()
{
	// TODO: Add your control notification handler code here
	TCHAR sFile[MAX_PATH];
	CString s;
	GetDlgItem(IDC_EDIT_FILE)->GetWindowText(s);
	_tcscpy(sFile, s);
	if (SaveFileDialog(this->m_hWnd,sFile, _MAX_PATH, 
						TEXT("AVI File\0*.avi;\0All Files\0*.*\0\0") ))
	{
		GetDlgItem(IDC_EDIT_FILE)->SetWindowText(sFile);
	}
}

void CSoundRecorderDlg::incTimer()
{
	m_iSecond++;
	if (m_iSecond >= 60)
	{
		m_iSecond = 0;
		m_iMinute++;
	}
	if (m_iMinute >= 60)
	{
		m_iMinute = 0;
		m_iHour++;
	}
}