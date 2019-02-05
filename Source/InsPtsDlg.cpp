// InsPtsDlg.cpp : implementation file
//

#include "Stdafx.h"
#include "InsPtsDlg.h"
#include "SurvUtilApp.h"
#include "ResourceHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HWND adsw_acadMainWnd();
/////////////////////////////////////////////////////////////////////////////
// CInsPtsDlg dialog


CInsPtsDlg::CInsPtsDlg(const CStringArray& BlkNameArray, CWnd* pParent /*=NULL*/)
	: CDialog(CInsPtsDlg::IDD, pParent), m_BlkNameArray(BlkNameArray)
{
	//{{AFX_DATA_INIT(CInsPtsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CInsPtsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInsPtsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInsPtsDlg, CDialog)
	//{{AFX_MSG_MAP(CInsPtsDlg)
	ON_BN_CLICKED(IDC_CHECK_SPLEVEL_FLAG, OnCheckSplevelFlag)
	ON_BN_CLICKED(IDC_CHECK_SHOW_PTNO, OnClickShowPtNo)
	ON_BN_CLICKED(IDC_CHECK_DESC_FLAG, OnClickShowDescFlag)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInsPtsDlg message handlers

BOOL CInsPtsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_dPtAnnTxtAngle = 45;//Default
	m_bDescFlag = TRUE;
	m_bSPLevelFlag = FALSE;
	m_bShowPtNoFlag = FALSE;
	m_bZFlag = TRUE;
	
	((CButton*)GetDlgItem(IDC_CHECK_DESC_FLAG))->SetCheck(m_bDescFlag);
	((CButton*)GetDlgItem(IDC_CHECK_SPLEVEL_FLAG))->SetCheck(m_bSPLevelFlag);
	((CButton*)GetDlgItem(IDC_CHECK_SHOW_PTNO))->SetCheck(m_bShowPtNoFlag);
	
	((CButton*)GetDlgItem(IDC_CHECK_Z_FLAG_PIN))->SetCheck(m_bZFlag);

	GetDlgItem(IDC_EDIT_SPLEVEL_LYR)->SetWindowText("SP_LEVEL_LYR");
	GetDlgItem(IDC_EDIT_NUM_TXT_SIZE)->SetWindowText("2.0");
	GetDlgItem(IDC_EDIT_PT_TXT_ANG)->SetWindowText("45.0");
	
	GetDlgItem(IDC_COMBO_BLK_NAME)->EnableWindow(m_bSPLevelFlag);
	GetDlgItem(IDC_EDIT_SPLEVEL_LYR)->EnableWindow(m_bSPLevelFlag);

	m_dPtAnnTxtAngle = 45;//Default
	GetDlgItem(IDC_EDIT_PT_TXT_ANG)->SetWindowText("45.0");

	{//Block Name Combo...
		int i;
		for(i = 0; i < m_BlkNameArray.GetSize(); i++)
		{
			((CComboBox*)GetDlgItem(IDC_COMBO_BLK_NAME))->AddString(m_BlkNameArray[i]);
		}
	}


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CInsPtsDlg::OnCheckSplevelFlag() 
{
	m_bSPLevelFlag = ((CButton*)GetDlgItem(IDC_CHECK_SPLEVEL_FLAG))->GetCheck();
	GetDlgItem(IDC_COMBO_BLK_NAME)->EnableWindow(m_bSPLevelFlag);
	GetDlgItem(IDC_EDIT_SPLEVEL_LYR)->EnableWindow(m_bSPLevelFlag);
}
void CInsPtsDlg::OnClickShowPtNo() 
{
	GetDlgItem(IDC_COMBO_BLK_NAME)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_SPLEVEL_LYR)->EnableWindow(FALSE);
}

void CInsPtsDlg::OnClickShowDescFlag() 
{
	GetDlgItem(IDC_COMBO_BLK_NAME)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_SPLEVEL_LYR)->EnableWindow(FALSE);
}

void CInsPtsDlg::OnOK() 
{
	// TODO: Add extra validation here
	CString strTemp;
	int iIndex;

	m_bZFlag = ((CButton*)GetDlgItem(IDC_CHECK_Z_FLAG_PIN))->GetCheck();
	m_bDescFlag = ((CButton*)GetDlgItem(IDC_CHECK_DESC_FLAG))->GetCheck();
	m_bSPLevelFlag = ((CButton*)GetDlgItem(IDC_CHECK_SPLEVEL_FLAG))->GetCheck();
	m_bShowPtNoFlag = ((CButton*)GetDlgItem(IDC_CHECK_SHOW_PTNO))->GetCheck();

	if(m_bSPLevelFlag)
	{
		iIndex = ((CComboBox*)GetDlgItem(IDC_COMBO_BLK_NAME))->GetCurSel();
		if(iIndex == CB_ERR)
		{
			AfxMessageBox("No block selected", MB_ICONSTOP);
			GetDlgItem(IDC_COMBO_BLK_NAME)->SetFocus();
			return;
		}
		((CComboBox*)GetDlgItem(IDC_COMBO_BLK_NAME))->GetLBText(iIndex, m_strBlk);
	}
	GetDlgItem(IDC_EDIT_PT_TXT_ANG)->GetWindowText(strTemp);
	m_dPtAnnTxtAngle = atof((LPCSTR)strTemp);
	if((m_dPtAnnTxtAngle < -360.0) || (m_dPtAnnTxtAngle > 360.0))
	{
		AfxMessageBox("Invalid rotation angle", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_PT_TXT_ANG)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_PT_TXT_ANG))->SetSel(0, -1);
		return;
	}
	
	GetDlgItem(IDC_EDIT_SPLEVEL_LYR)->GetWindowText(m_strLyr);
	if(!CSurvUtilApp::IsValidACADSymName(m_strLyr))
	{
		AfxMessageBox("Invalid Layer name", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_SPLEVEL_LYR)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_SPLEVEL_LYR))->SetSel(0, -1);
		return;
	}
	
	GetDlgItem(IDC_EDIT_NUM_TXT_SIZE)->GetWindowText(strTemp);
	m_dTxtSize = atof((LPCSTR)strTemp);
	if(m_dTxtSize <= 0.0)
	{
		AfxMessageBox("Invalid Text Size", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_NUM_TXT_SIZE)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_NUM_TXT_SIZE))->SetSel(0, -1);
		return;
	}


	CDialog::OnOK();
}
void CInsPtsDlg::GetAttribs(BOOL& bZFlag, double& dTxtSize, double& dPtAnnTxtAng, CString& strBlk, CString& strLyr, BOOL& bSPLevelFlag, BOOL& bDescFlag, BOOL& bShowPtNoFlag)
{
	dTxtSize = m_dTxtSize,
	dPtAnnTxtAng = m_dPtAnnTxtAngle;

    strBlk = m_strBlk;
	strLyr = m_strLyr;
	bZFlag = m_bZFlag;

	bDescFlag		= m_bDescFlag;
	bSPLevelFlag	= m_bSPLevelFlag;
	bShowPtNoFlag	= m_bShowPtNoFlag;
}
BOOL CInsPtsDlg::GetInsPtsParams(const CStringArray& BlkNameArray, BOOL& bZFlag, double& dTxtSize, double& dPtAnnTxtAng, CString& strBlk, CString& strLyr, BOOL& bSPLevelFlag, BOOL& bDescFlag, BOOL& bShowPtNoFlag)
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;
	
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	CInsPtsDlg Dlg(BlkNameArray, pAcadWnd);
	if(Dlg.DoModal() != IDOK)
		return FALSE;

	Dlg.GetAttribs(bZFlag, dTxtSize, dPtAnnTxtAng, strBlk, strLyr, bSPLevelFlag, bDescFlag, bShowPtNoFlag);
	return TRUE;
}
