// ConvertFDataDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConvertFDataDlg.h"
#include "ConvertFData.h"
#include "ResourceHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HWND adsw_acadMainWnd();
/////////////////////////////////////////////////////////////////////////////
// CConvertFDataDlg dialog


CConvertFDataDlg::CConvertFDataDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConvertFDataDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConvertFDataDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CConvertFDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConvertFDataDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConvertFDataDlg, CDialog)
	//{{AFX_MSG_MAP(CConvertFDataDlg)
	ON_BN_CLICKED(IDC_BUTTON_INP_FILE, OnButtonInpFile)
	ON_BN_CLICKED(IDC_BUTTON_OUT_FILE, OnButtonOutFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConvertFDataDlg message handlers

BOOL CConvertFDataDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	GetDlgItem(IDC_EDIT_INP_FILE)->SetWindowText("No file selected yet");
	GetDlgItem(IDC_EDIT_OUT_FILE)->SetWindowText("No file selected yet");
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConvertFDataDlg::OnOK() 
{
	// TODO: Add extra validation here
	if(m_strInpFile.IsEmpty() || m_strOutFile.IsEmpty())
	{
		AfxMessageBox("ERROR: Data files not specified", MB_ICONSTOP);
		return;
	}
	if(m_strInpFile.CompareNoCase(m_strOutFile) == 0)
	{
		AfxMessageBox("ERROR: Input & Output files are both same", MB_ICONSTOP);
		return;
	}
	
	{//Start Conversion.......................
		CWaitCursor WaitCursor;
		CString strMsg;
		
		if(ConvertRawFData(m_strInpFile, m_strOutFile))
		{
			strMsg.Format("Output file \"%s\" created successfully.", (LPCSTR)m_strOutFile);
			AfxMessageBox(strMsg, MB_ICONINFORMATION);
		}
	}
	//CDialog::OnOK();
}

void CConvertFDataDlg::OnButtonInpFile() 
{
	DWORD dwFlags;
	const CString strConstFOpenFilter = CString("Field Data Files (*.TXT)|*.TXT||");

	dwFlags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	CFileDialog Dlg(TRUE, 0L, 0L, dwFlags, strConstFOpenFilter);
	if(Dlg.DoModal() == IDOK)
	{
		m_strInpFile = Dlg.GetPathName();
		GetDlgItem(IDC_EDIT_INP_FILE)->SetWindowText(m_strInpFile);
	}
}
void CConvertFDataDlg::OnButtonOutFile() 
{
	DWORD dwFlags;
	const CString strConstFOpenFilter = CString("Output Data Files (*.TXT)|*.TXT||");

	dwFlags = OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY;
	CFileDialog Dlg(FALSE, 0L, 0L, dwFlags, strConstFOpenFilter);
	if(Dlg.DoModal() == IDOK)
	{
		m_strOutFile = Dlg.GetPathName();
		GetDlgItem(IDC_EDIT_OUT_FILE)->SetWindowText(m_strOutFile);
	}
}

void ConvertFieldData()
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;
	
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	CConvertFDataDlg Dlg(pAcadWnd);
	Dlg.DoModal();
}
