// InsBlksUtil.cpp : implementation file
//

#include "stdafx.h"
#include "InsBlksUtil.h"
#include "SurvUtilApp.h"
#include "ResourceHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HWND adsw_acadMainWnd();

/////////////////////////////////////////////////////////////////////////////
// CInsBlksUtil dialog


CInsBlksUtil::CInsBlksUtil(const CStringArray& BlkNameArray, CWnd* pParent /*=NULL*/)
	: CDialog(CInsBlksUtil::IDD, pParent), m_BlkNameArray(BlkNameArray)
{
	//{{AFX_DATA_INIT(CInsBlksUtil)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CInsBlksUtil::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInsBlksUtil)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInsBlksUtil, CDialog)
	//{{AFX_MSG_MAP(CInsBlksUtil)
	ON_BN_CLICKED(IDC_CHECK, OnCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInsBlksUtil message handlers

BOOL CInsBlksUtil::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_bDrawOnCurLyr = TRUE;
	((CButton*)GetDlgItem(IDC_CHECK))->SetCheck(1);
	m_bConsiderZ = TRUE;
	((CButton*)GetDlgItem(IDC_CHECK_ZFLAG))->SetCheck(1);

	GetDlgItem(IDC_EDIT_LYR)->EnableWindow(!m_bDrawOnCurLyr);
	GetDlgItem(IDC_EDIT_BLK_SCALE_X)->SetWindowText("1.0");
	GetDlgItem(IDC_EDIT_BLK_SCALE_Y)->SetWindowText("1.0");
	GetDlgItem(IDC_EDIT_BLK_SCALE_Z)->SetWindowText("1.0");
	GetDlgItem(IDC_EDIT_BLK_ROT_ANGLE)->SetWindowText("0.0");
	
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

void CInsBlksUtil::OnOK() 
{
	// TODO: Add extra validation here
	int iIndex;
	CString strTemp;

	m_bDrawOnCurLyr = ((CButton*)GetDlgItem(IDC_CHECK))->GetCheck();
	m_bConsiderZ = ((CButton*)GetDlgItem(IDC_CHECK_ZFLAG))->GetCheck();

	GetDlgItem(IDC_EDIT_LYR)->GetWindowText(m_strLyr);
	if(!m_bDrawOnCurLyr)
	{
		if(!CSurvUtilApp::IsValidACADSymName(m_strLyr))
		{
			AfxMessageBox("Invalid Layer name", MB_ICONSTOP);
			GetDlgItem(IDC_EDIT_LYR)->SetFocus();
			((CEdit*)GetDlgItem(IDC_EDIT_LYR))->SetSel(0, -1);
			return;
		}
	}
	iIndex = ((CComboBox*)GetDlgItem(IDC_COMBO_BLK_NAME))->GetCurSel();
	if(iIndex == CB_ERR)
	{
		AfxMessageBox("No block selected", MB_ICONSTOP);
		GetDlgItem(IDC_COMBO_BLK_NAME)->SetFocus();
		return;
	}
	((CComboBox*)GetDlgItem(IDC_COMBO_BLK_NAME))->GetLBText(iIndex, m_strBlk);

	GetDlgItem(IDC_EDIT_BLK_SCALE_X)->GetWindowText(strTemp);
	m_dX = atof((LPCSTR)strTemp);
	if(m_dX <= 0.0)
	{
		AfxMessageBox("Invalid Block Scale", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_BLK_SCALE_X)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_BLK_SCALE_X))->SetSel(0, -1);
		return;
	}
	
	GetDlgItem(IDC_EDIT_BLK_SCALE_Y)->GetWindowText(strTemp);
	m_dY = atof((LPCSTR)strTemp);
	if(m_dY <= 0.0)
	{
		AfxMessageBox("Invalid Block Scale", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_BLK_SCALE_Y)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_BLK_SCALE_Y))->SetSel(0, -1);
		return;
	}

	GetDlgItem(IDC_EDIT_BLK_SCALE_Z)->GetWindowText(strTemp);
	m_dZ = atof((LPCSTR)strTemp);
	if(m_dZ <= 0.0)
	{
		AfxMessageBox("Invalid Block Scale", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_BLK_SCALE_Z)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_BLK_SCALE_Z))->SetSel(0, -1);
		return;
	}

	GetDlgItem(IDC_EDIT_BLK_ROT_ANGLE)->GetWindowText(strTemp);
	m_dRotAngle = atof((LPCSTR)strTemp);
	if((m_dRotAngle < -360.0) || (m_dRotAngle > 360.0))
	{
		AfxMessageBox("Invalid Block Rotation Angle", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_BLK_ROT_ANGLE)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_BLK_ROT_ANGLE))->SetSel(0, -1);
		return;
	}

	CDialog::OnOK();
}
void CInsBlksUtil::OnCheck() 
{
	m_bDrawOnCurLyr = !m_bDrawOnCurLyr;
	GetDlgItem(IDC_EDIT_LYR)->EnableWindow(!m_bDrawOnCurLyr);
}
void CInsBlksUtil::GetAttribs(BOOL& bDrOnCurLyr, CString& strLyr, CString& strBlk, double& dX, double& dY, double& dZ, double& dRotAngle, BOOL& bConsiderZ)
{
	bDrOnCurLyr = m_bDrawOnCurLyr;
	bConsiderZ = m_bConsiderZ;
	strLyr = m_strLyr;
	strBlk = m_strBlk;
	dX = m_dX;
	dY = m_dY;
	dZ = m_dZ;
	dRotAngle = m_dRotAngle;
}


BOOL CInsBlksUtil::GetInsBlksParams(const CStringArray& BlkNameArray, CString& strLyr, CString& strBlk, double& dX, double& dY, double& dZ, double& dRotAngle, BOOL& bConsiderZ)
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;
	BOOL bDrOnCurLyr;
	
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	CInsBlksUtil Dlg(BlkNameArray, pAcadWnd);
	if(Dlg.DoModal() != IDOK)
		return FALSE;

	Dlg.GetAttribs(bDrOnCurLyr, strLyr, strBlk, dX, dY, dZ, dRotAngle, bConsiderZ);
	if(bDrOnCurLyr)
		strLyr = "";

	return TRUE;
}
