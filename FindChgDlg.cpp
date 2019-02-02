// FindChgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SurvUtilApp.h"
#include "FindChgDlg.h"
#include "DrawUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFindChgDlg dialog
extern CFindChgDlg* gpFindChgDlg;

//	When AutoCAD asks your dialog if it is willing to give up input
//	focus, return true to keep the focus and false to give it up.
//
#ifndef WM_ACAD_MFC_BASE
#define WM_ACAD_MFC_BASE        (1000)
#endif

#ifndef WM_ACAD_KEEPFOCUS
#define WM_ACAD_KEEPFOCUS       (WM_ACAD_MFC_BASE + 1)
#endif


CFindChgDlg::CFindChgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFindChgDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFindChgDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//
//	ChelloDlg::onAcadKeepFocus()
//
//	This function gets called repeatedly whenever the mouse is moved over
//	the AutoCAD window.
//
afx_msg LONG CFindChgDlg::onAcadKeepFocus( UINT, LONG )
{
	return TRUE;
}


//
//	ChelloDlg::OnSysCommand()
//
//	Handle commands invoked from the dialog's system menu.
//
//
afx_msg void CFindChgDlg::OnSysCommand ( UINT nID, LPARAM lParam )
{
	CDialog::OnSysCommand ( nID, lParam );
}

void CFindChgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindChgDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFindChgDlg, CDialog)
	//{{AFX_MSG_MAP(CFindChgDlg)
	ON_WM_SYSCOMMAND()
	ON_MESSAGE( WM_ACAD_KEEPFOCUS, onAcadKeepFocus )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindChgDlg message handlers

BOOL CFindChgDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CString strTmp;
	
	strTmp.Format("%f", 0.0);
	GetDlgItem(IDC_EDIT_CH)->SetWindowText(strTmp);
	
	strTmp.Format("%f", 0.0);
	GetDlgItem(IDC_EDIT_1)->SetWindowText(strTmp);
	
	strTmp.Format("%f", 0.0);
	GetDlgItem(IDC_EDIT_2)->SetWindowText(strTmp);
	
	strTmp.Format("%f", TEXTSIZE());
	GetDlgItem(IDC_EDIT_TXT_SIZE)->SetWindowText(strTmp);
	
	CLAYER(strTmp);
	GetDlgItem(IDC_EDIT_LYR)->SetWindowText(strTmp);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFindChgDlg::OnOK() 
{
	// TODO: Add extra validation here
	CString strTmp, strLyr;
	double fLeftOffset, fRiteOffset, fChg, fTxtSize;

	GetDlgItem(IDC_EDIT_CH)->GetWindowText(strTmp);
	fChg = atof(strTmp);
	if(fChg < 0.0)
	{
		AfxMessageBox("Invalid chainage specified", MB_ICONSTOP);
		((CEdit*)GetDlgItem(IDC_EDIT_CH))->SetSel(0, -1);
		((CEdit*)GetDlgItem(IDC_EDIT_CH))->SetFocus();
		return;
	}

	GetDlgItem(IDC_EDIT_1)->GetWindowText(strTmp);
	fLeftOffset = atof(strTmp);
	if(fLeftOffset < 0.0)
	{
		AfxMessageBox("Invalid offset specified", MB_ICONSTOP);
		((CEdit*)GetDlgItem(IDC_EDIT_1))->SetSel(0, -1);
		((CEdit*)GetDlgItem(IDC_EDIT_1))->SetFocus();
		return;
	}
	
	GetDlgItem(IDC_EDIT_2)->GetWindowText(strTmp);
	fRiteOffset = atof(strTmp);
	if(fRiteOffset < 0.0)
	{
		AfxMessageBox("Invalid offset specified", MB_ICONSTOP);
		((CEdit*)GetDlgItem(IDC_EDIT_2))->SetSel(0, -1);
		((CEdit*)GetDlgItem(IDC_EDIT_2))->SetFocus();
		return;
	}
	
	GetDlgItem(IDC_EDIT_TXT_SIZE)->GetWindowText(strTmp);
	fTxtSize = atof(strTmp);
	if(fTxtSize <= 0.0)
	{
		AfxMessageBox("Invalid text size specified", MB_ICONSTOP);
		((CEdit*)GetDlgItem(IDC_EDIT_TXT_SIZE))->SetSel(0, -1);
		((CEdit*)GetDlgItem(IDC_EDIT_TXT_SIZE))->SetFocus();
		return;
	}

	GetDlgItem(IDC_EDIT_LYR)->GetWindowText(strLyr);
	if(strlen(strLyr) <= 0)
	{
		AfxMessageBox("Invalid layer name specified", MB_ICONSTOP);
		((CEdit*)GetDlgItem(IDC_EDIT_LYR))->SetSel(0, -1);
		((CEdit*)GetDlgItem(IDC_EDIT_LYR))->SetFocus();
		return;
	}
	//
	CSurvUtilApp::AnnotateChgCore(fChg, fLeftOffset, fRiteOffset, fTxtSize, strLyr);

	//
	acdbCurDwg()->updateExt();
	ads_redraw(0L, 1);
	//
	CWnd::FromHandle(acedGetAcadFrame()->m_hWnd)->SetFocus();
	//CDialog::OnOK();
}
