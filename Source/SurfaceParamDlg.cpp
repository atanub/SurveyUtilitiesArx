// SurfaceParamDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SurfaceParamDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSurfaceParamDlg dialog


CSurfaceParamDlg::CSurfaceParamDlg(const CStringArray& strArrayBlocks, const double& fScale, const double& fMinSpace, CWnd* pParent /*=NULL*/)
	: CDialog(CSurfaceParamDlg::IDD, pParent),m_strArrayBlocks(strArrayBlocks), m_fScale(fScale),	m_fMinSpace(fMinSpace)
{
	//{{AFX_DATA_INIT(CSurfaceParamDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CSurfaceParamDlg::GetAttrib(CString& strSelBlk, double& fScale, double& fMinSpace)
{
	strSelBlk = m_strBlock;
	fScale = m_fScale;
	fMinSpace = m_fMinSpace;
}

void CSurfaceParamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSurfaceParamDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSurfaceParamDlg, CDialog)
	//{{AFX_MSG_MAP(CSurfaceParamDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSurfaceParamDlg message handlers

void CSurfaceParamDlg::OnOK() 
{
	CString strTemp;
	int iIndex;

	iIndex = ((CComboBox*)GetDlgItem(IDC_COMBO_BLK_NAME))->GetCurSel();
	if(iIndex == CB_ERR)
	{
		AfxMessageBox("No block selected", MB_ICONSTOP);
		GetDlgItem(IDC_COMBO_BLK_NAME)->SetFocus();
		return;
	}
	((CComboBox*)GetDlgItem(IDC_COMBO_BLK_NAME))->GetLBText(iIndex, m_strBlock);
	
	//
	GetDlgItem(IDC_EDIT_BLK_SCALE_X)->GetWindowText(strTemp);
	m_fScale = atof((LPCSTR)strTemp);
	if(m_fScale <= 0.0)
	{
		AfxMessageBox("Invalid Block Size specified", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_BLK_SCALE_X)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_BLK_SCALE_X))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_MIN_SPACE)->GetWindowText(strTemp);
	m_fMinSpace = atof((LPCSTR)strTemp);
	if(m_fMinSpace <= 0.0)
	{
		AfxMessageBox("Invalid Spacing specified", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_MIN_SPACE)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_MIN_SPACE))->SetSel(0, -1);
		return;
	}
	if(m_fMinSpace <= m_fScale)
	{
		AfxMessageBox("Invalid Spacing specified, Spacing should be greater than the block scale", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_MIN_SPACE)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_MIN_SPACE))->SetSel(0, -1);
		return;
	}

	CDialog::OnOK();
}

BOOL CSurfaceParamDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	{//Block Name Combo...
		int i;
		for(i = 0; i < m_strArrayBlocks.GetSize(); i++)
		{
			((CComboBox*)GetDlgItem(IDC_COMBO_BLK_NAME))->AddString(m_strArrayBlocks[i]);
		}
	}
	CString strTemp;

	strTemp.Format("%.3f", m_fScale);
	GetDlgItem(IDC_EDIT_BLK_SCALE_X)->SetWindowText(strTemp);
	strTemp.Format("%.3f", m_fMinSpace);
	GetDlgItem(IDC_EDIT_MIN_SPACE)->SetWindowText(strTemp);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
