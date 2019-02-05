// ChgRangeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ChgRangeDlg.h"
#include "ResourceHelper.h"
#include "DrawUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HWND adsw_acadMainWnd();

/////////////////////////////////////////////////////////////////////////////
// CChgRangeDlg dialog


CChgRangeDlg::CChgRangeDlg(const double& fMax, const double& fMin, double& fValMin, double& fValMax, CWnd* pParent/* = NULL*/)
	: CDialog(CChgRangeDlg::IDD, pParent), m_fMax(fMax), m_fMin(fMin), m_fValMin(fValMin), m_fValMax(fValMax)
{
	//{{AFX_DATA_INIT(CChgRangeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

	
void CChgRangeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChgRangeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChgRangeDlg, CDialog)
	//{{AFX_MSG_MAP(CChgRangeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChgRangeDlg message handlers

BOOL CChgRangeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	GetDlgItem(IDC_EDIT1)->SetWindowText(RTOS(m_fValMin));
	GetDlgItem(IDC_EDIT2)->SetWindowText(RTOS(m_fValMax));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChgRangeDlg::OnOK() 
{
	// TODO: Add extra validation here
	CString strTmp;
	
	GetDlgItem(IDC_EDIT1)->GetWindowText(strTmp);
	m_fValMin = atof(strTmp);

	GetDlgItem(IDC_EDIT2)->GetWindowText(strTmp);
	m_fValMax = atof(strTmp);

	if(m_fValMin > m_fValMax)
	{//Invalid
		AfxMessageBox("Invalid Start Chainage specified", MB_ICONSTOP);
		((CEdit*)GetDlgItem(IDC_EDIT1))->SetSel(0, -1);
		((CEdit*)GetDlgItem(IDC_EDIT1))->SetFocus();
		return;
	}
	if((m_fValMin < m_fMin) || (m_fValMin > m_fMax))
	{//Invalid
		strTmp.Format("Invalid Start Chainage specified\nThe chainage should be in between %s to %s", RTOS(m_fMin), RTOS(m_fMax));
		AfxMessageBox(strTmp, MB_ICONSTOP);
		((CEdit*)GetDlgItem(IDC_EDIT1))->SetSel(0, -1);
		((CEdit*)GetDlgItem(IDC_EDIT1))->SetFocus();
		return;
	}
	if((m_fValMax < m_fMin) || (m_fValMax > m_fMax))
	{//Invalid
		strTmp.Format("Invalid End Chainage specified\nThe chainage should be in between %s to %s", RTOS(m_fMin), RTOS(m_fMax));
		AfxMessageBox(strTmp, MB_ICONSTOP);
		((CEdit*)GetDlgItem(IDC_EDIT2))->SetSel(0, -1);
		((CEdit*)GetDlgItem(IDC_EDIT2))->SetFocus();
		return;
	}
	
	CDialog::OnOK();
}

BOOL GetChgRange(const double& fMax, const double& fMin, double& fMinVal, double& fMaxVal)
{
	CWnd* pAcadWnd;
	double fMinValCache, fMaxValCache;
	CTemporaryResourceOverride ResOverride;
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	
	fMinValCache = fMinVal;
	fMaxValCache = fMaxVal;
	
	
	CChgRangeDlg Dlg(fMax, fMin, fMinValCache, fMaxValCache, pAcadWnd);
	if(Dlg.DoModal() != IDOK)
		return FALSE;

	//
	fMinVal = fMinValCache;
	fMaxVal = fMaxValCache;
	return TRUE;
}

