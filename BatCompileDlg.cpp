// BatCompileDlg.cpp : implementation file
//
#include "stdafx.h"
#include "BatCompileDlg.h"
#include "SurveyBookReader.h"
#include "SurvUtilApp.h"
#include "ResourceHelper.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CSurvUtilApp	theApp;
extern "C" HWND adsw_acadMainWnd();

/////////////////////////////////////////////////////////////////////////////
// CBatCompileDlg dialog
const CString CBatCompileDlg::m_strListCtrlVw = CString("Files for batch compilation");
const CString CBatCompileDlg::m_strEditCtrlVw = CString("Batch compilation report");

CBatCompileDlg::CBatCompileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBatCompileDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBatCompileDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hIconAddToQue = 0L;
	m_hIconAddToQue = AfxGetApp()->LoadIcon(IDI_ADDTO_QUE);
	m_hIconVwList = AfxGetApp()->LoadIcon(IDI_LIST_VW);
	m_hIconOutput	= AfxGetApp()->LoadIcon(IDI_OUTPUT_VW);
	VERIFY(m_hIconAddToQue != 0L);
}


void CBatCompileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBatCompileDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBatCompileDlg, CDialog)
	//{{AFX_MSG_MAP(CBatCompileDlg)
	ON_BN_CLICKED(IDC_BUTTON_RESET, OnButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_VIEW, OnButtonView)
	ON_BN_CLICKED(IDC_CONVERT, OnConvert)
	ON_BN_CLICKED(IDC_BUTTON_ADDTO_QUE, OnButtonAddToQue)
	ON_BN_CLICKED(IDC_SEL_XLS, OnSelXLSFile)
	ON_BN_CLICKED(IDC_SEL_OUT, OnSelOutFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBatCompileDlg message handlers

BOOL CBatCompileDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	{//Initialize List Ctrl
		CRect rect;
		GetDlgItem(IDC_LIST1)->GetWindowRect(rect);
		((CListCtrl*)GetDlgItem(IDC_LIST1))->InsertColumn(0, "Input File");
		((CListCtrl*)GetDlgItem(IDC_LIST1))->InsertColumn(1, "Output File");

		((CListCtrl*)GetDlgItem(IDC_LIST1))->SetColumnWidth(0, rect.Width() / 2);
		((CListCtrl*)GetDlgItem(IDC_LIST1))->SetColumnWidth(1, rect.Width() / 2);

		ScreenToClient(rect);
		GetDlgItem(IDC_EDIT_OUTPUT)->MoveWindow(rect);
	}

	((CButton*)GetDlgItem(IDC_BUTTON_ADDTO_QUE))->SetIcon(m_hIconAddToQue);
	((CButton*)GetDlgItem(IDC_BUTTON_VIEW))->SetIcon(m_hIconVwList);
	GetDlgItem(IDC_STATIC_VIEW)->SetWindowText(m_strListCtrlVw);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CBatCompileDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}
void CBatCompileDlg::OnButtonReset() 
{
	((CListCtrl*)GetDlgItem(IDC_LIST1))->DeleteAllItems();
	m_MapInAndOutFile.RemoveAll();
}
void CBatCompileDlg::OnButtonView() 
{
	BOOL bFlag;
	bFlag = ((CButton*)GetDlgItem(IDC_BUTTON_VIEW))->GetCheck();
	
	GetDlgItem(IDC_EDIT_OUTPUT)->ShowWindow(bFlag ? SW_SHOW:SW_HIDE);
	GetDlgItem(IDC_LIST1)->ShowWindow(bFlag ? SW_HIDE:SW_SHOW);

	((CButton*)GetDlgItem(IDC_BUTTON_VIEW))->SetIcon(bFlag ? m_hIconOutput:m_hIconVwList);
	GetDlgItem(IDC_STATIC_VIEW)->SetWindowText(bFlag ? m_strEditCtrlVw:m_strListCtrlVw);
}

void CBatCompileDlg::OnConvert() 
{
	// TODO: Add your control notification handler code here
	CStringArray strArrayOfErrs;
	CString	strTmp;

	if(m_MapInAndOutFile.GetCount() < 1)
	{
		AfxMessageBox("No file(s) selected for compilation", MB_ICONSTOP);
		return;
	}
	CSurvUtilApp::BatchConvert(m_MapInAndOutFile, strArrayOfErrs);
	strTmp = MakeDispStr(strArrayOfErrs);
	GetDlgItem(IDC_EDIT_OUTPUT)->SetWindowText(strTmp);
}
CString CBatCompileDlg::MakeDispStr(const CStringArray& strArray)
{
	const CString strConstRowSep("\r\n");
	int i;
	CString strToShow;
	for(i = 0; i < strArray.GetSize(); i++)
	{
		if(i == 0)
			strToShow = strArray[i];
		else
			strToShow += strConstRowSep + strArray[i];
	}
	return strToShow;
}
void CBatCompileDlg::OnButtonAddToQue() 
{
	// TODO: Add your control notification handler code here
	CString strTmp, strInpFile, strOutFile;
	CFileFind FileFind;
	int i;

	GetDlgItem(IDC_EDIT1)->GetWindowText(strInpFile);
	if(!FileFind.FindFile(strInpFile))
	{
		strTmp.Format("Invalid file name \"%s\"", strInpFile);
		AfxMessageBox(strTmp, MB_ICONSTOP);
		return;
	}
	strTmp = strInpFile;
	strTmp.MakeLower();
	if(m_MapInAndOutFile.Lookup(strTmp, strTmp))
	{
		strTmp.Format("Invalid file, \"%s\" already selected for batch compilation", strInpFile);
		AfxMessageBox(strTmp, MB_ICONSTOP);
		return;
	}

	GetDlgItem(IDC_EDIT2)->GetWindowText(strOutFile);
	if(strlen((LPCSTR)strOutFile) <= 0)
	{
		strTmp.Format("Invalid file name \"%s\"", strOutFile);
		AfxMessageBox(strTmp, MB_ICONSTOP);
		return;
	}
	strInpFile.MakeLower();
	strOutFile.MakeLower();
	m_MapInAndOutFile.SetAt(strInpFile, strOutFile);
	i = m_MapInAndOutFile.GetCount();
	i = ((CListCtrl*)GetDlgItem(IDC_LIST1))->InsertItem(i, strInpFile);
	((CListCtrl*)GetDlgItem(IDC_LIST1))->SetItemText(i, 1, strOutFile);

	GetDlgItem(IDC_EDIT1)->SetWindowText("");
	GetDlgItem(IDC_EDIT2)->SetWindowText("");
}
void CBatCompileDlg::OnSelXLSFile() 
{
	DWORD dwFlags;
	CString strFile;

	dwFlags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	CFileDialog Dlg(TRUE, 0L, 0L, dwFlags, CSurvUtilApp::m_strConstFileOpenFilter);
	if(Dlg.DoModal() == IDOK)
	{
		strFile = Dlg.GetPathName();
		GetDlgItem(IDC_EDIT1)->SetWindowText(strFile);
	}
}
void CBatCompileDlg::OnSelOutFile() 
{
	DWORD dwFlags;
	CString strFile;

	dwFlags = OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY;
	CFileDialog Dlg(FALSE, CSurvUtilApp::m_strConstDefXtnOfBINOutFile, 0L, dwFlags, CSurvUtilApp::m_strConstCompiledFOpenFilter);
	if(Dlg.DoModal() == IDOK)
	{
		strFile = Dlg.GetPathName();
		GetDlgItem(IDC_EDIT2)->SetWindowText(strFile);
	}
}

void CBatCompileDlg::BatchCompileXLS()
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;
	
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	CBatCompileDlg Dlg(pAcadWnd);
	Dlg.DoModal();
}
