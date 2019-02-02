// RXDataDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RXDataDlg.h"
#include "ResourceHelper.h"
#include "SurvUtilApp.h"
#include "DrawUtil.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HWND adsw_acadMainWnd();

/////////////////////////////////////////////////////////////////////////////
// CRXDataDlg dialog

CRXDataDlg::CRXDataDlg(const CStringArray& BlkNameArray, CString& strLyrRXD, CString& strLyrPID, CString& strBlk, double& dBlkScaleRel, double& dTxtSize, double& dTxtRotAngleRad, BOOL& bZFlagRXD, BOOL& bZFlagPID, CWnd* pParent /*=NULL*/)
	: CDialog(CRXDataDlg::IDD, pParent), m_strBlk(strBlk), m_strLyrRXD(strLyrRXD), m_strLyrPID(strLyrPID), m_dTxtRotAngle(dTxtRotAngleRad), m_dTxtSize(dTxtSize), m_dBlkScale(dBlkScaleRel), m_bZFlagRXD(bZFlagRXD), m_bZFlagPID(bZFlagPID), m_BlkNameArray(BlkNameArray)
{
	//{{AFX_DATA_INIT(CRXDataDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRXDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRXDataDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRXDataDlg, CDialog)
	//{{AFX_MSG_MAP(CRXDataDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRXDataDlg message handlers

BOOL CRXDataDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CString strTmp;


	((CButton*)GetDlgItem(IDC_CHECK_Z_FLAG_RXD))->SetCheck(m_bZFlagRXD);
	((CButton*)GetDlgItem(IDC_CHECK_Z_FLAG_PID))->SetCheck(m_bZFlagPID);
	
	GetDlgItem(IDC_EDIT_LYR_RXD)->SetWindowText(m_strLyrRXD);
	GetDlgItem(IDC_EDIT_LYR_PID)->SetWindowText(m_strLyrPID);
	{//Block Name Combo...
		int i;
		for(i = 0; i < m_BlkNameArray.GetSize(); i++)
		{
			((CComboBox*)GetDlgItem(IDC_COMBO_BLOCK))->AddString(m_BlkNameArray[i]);
		}
	}
	strTmp.Format("%f", m_dTxtSize);	GetDlgItem(IDC_EDIT_TXT_SIZE)->SetWindowText(strTmp);
	strTmp.Format("%f", m_dBlkScale);	GetDlgItem(IDC_EDIT_BLK_SCALE)->SetWindowText(strTmp);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CRXDataDlg::OnOK() 
{
	// TODO: Add extra validation here
	int iIndex;
	CString strTemp;

	//RXD Related 
	GetDlgItem(IDC_EDIT_LYR_RXD)->GetWindowText(m_strLyrRXD);
	if(!CSurvUtilApp::IsValidACADSymName(m_strLyrRXD))
	{
		AfxMessageBox("Invalid Layer name", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_LYR_RXD)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_LYR_RXD))->SetSel(0, -1);
		return;
	}
	//
	iIndex = ((CComboBox*)GetDlgItem(IDC_COMBO_BLOCK))->GetCurSel();
	if(iIndex == CB_ERR)
	{
		AfxMessageBox("No block selected", MB_ICONSTOP);
		GetDlgItem(IDC_COMBO_BLOCK)->SetFocus();
		return;
	}
	((CComboBox*)GetDlgItem(IDC_COMBO_BLOCK))->GetLBText(iIndex, m_strBlk);

	GetDlgItem(IDC_EDIT_TXT_SIZE)->GetWindowText(strTemp);
	m_dTxtSize = atof((LPCSTR)strTemp);
	if(m_dTxtSize <= 0.0)
	{
		AfxMessageBox("Invalid Text Size", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_TXT_SIZE)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_TXT_SIZE))->SetSel(0, -1);
		return;
	}
	GetDlgItem(IDC_EDIT_BLK_SCALE)->GetWindowText(strTemp);
	m_dBlkScale = atof((LPCSTR)strTemp);
	if((m_dBlkScale <= 0.0) || (m_dBlkScale > 1.0))
	{
		AfxMessageBox("Invalid relative block scale", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_BLK_SCALE)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_BLK_SCALE))->SetSel(0, -1);
		return;
	}
	
	m_bZFlagRXD = ((CButton*)GetDlgItem(IDC_CHECK_Z_FLAG_RXD))->GetCheck();

	//PID Related 
	GetDlgItem(IDC_EDIT_LYR_PID)->GetWindowText(m_strLyrPID);
	if(!CSurvUtilApp::IsValidACADSymName(m_strLyrPID))
	{
		AfxMessageBox("Invalid Layer name", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_LYR_PID)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_LYR_PID))->SetSel(0, -1);
		return;
	}
	m_bZFlagPID = ((CButton*)GetDlgItem(IDC_CHECK_Z_FLAG_PID))->GetCheck();
	
	CDialog::OnOK();
}
void CRXDataDlg::GetAttribs(CString& strLyrRXD, CString& strLyrPID, CString& strBlk, double& dBlkScale, double& dTxtSize, double& dTxtRotInRad, BOOL& bZFlagRXD, BOOL& bZFlagPID) const
{
	strLyrRXD		= m_strLyrRXD;
	strBlk			= m_strBlk;
	dTxtRotInRad	= m_dTxtRotAngle;
	dTxtSize		= m_dTxtSize;
	dBlkScale		= m_dBlkScale;
	bZFlagRXD		= m_bZFlagRXD;
	bZFlagPID		= m_bZFlagPID;
	strLyrPID		= m_strLyrPID;
}

//////////////////////////////////////////////////
BOOL CRXDataDlg::GetRoadPlanData(CString& strLyrRXD, CString& strLyrPID, CString& strBlk, double& dBlkScaleRel, double& dTxtSize, double& dTxtRotInRad, BOOL& bZFlagRXD, BOOL& bZFlagPID)
{
	CWnd* pAcadWnd;
	CStringArray BlkNameArray;

	GetBlockNames(BlkNameArray);

	CTemporaryResourceOverride ResOverride;
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	{//Initialize default data...
		strLyrRXD = "ROAD_X_SECN_PTS";
		strLyrPID = "POINT_OF_INTERSECTION";
		strBlk = "";
		dTxtSize = 2.0;
		dBlkScaleRel = 0.5;
		dTxtRotInRad = DTOR(45.0);
		bZFlagRXD = TRUE;
		bZFlagPID = TRUE;
	}
	CRXDataDlg Dlg(BlkNameArray, strLyrRXD, strLyrPID, strBlk, dBlkScaleRel, dTxtSize, dTxtRotInRad, bZFlagRXD, bZFlagPID, pAcadWnd);
	if(Dlg.DoModal() != IDOK)
		return FALSE;

	Dlg.GetAttribs(strLyrRXD, strLyrPID, strBlk, dBlkScaleRel, dTxtSize, dTxtRotInRad, bZFlagRXD, bZFlagPID);
	return TRUE;
}

