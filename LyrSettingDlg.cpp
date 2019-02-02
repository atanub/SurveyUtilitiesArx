// LyrSettingDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "SurvUtilApp.h"
#include "LyrSettingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLyrSettingDlg dialog
const CString CLyrSettingDlg::m_strLBItemFmt = CString("%-32s : %s");


CLyrSettingDlg::CLyrSettingDlg(const char* pszTitle, const BOOL& bIsForACADSymbol, const int& iSize, const char** pszTags, const char** pszVals, CWnd* pParent /*=NULL*/)
	: CDialog(CLyrSettingDlg::IDD, pParent), m_pszTitle(pszTitle), m_bIsForACADSymbol(bIsForACADSymbol)
{
	int i;

	for(i = 0; i < iSize; i++)
	{
		m_strArrayTags.Add(*(pszTags+i));
		m_strArrayVals.Add(*(pszVals+i));
	}
}


void CLyrSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLyrSettingDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLyrSettingDlg, CDialog)
	//{{AFX_MSG_MAP(CLyrSettingDlg)
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelChangeList)
	ON_BN_CLICKED(IDUPDATE, OnUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLyrSettingDlg message handlers

BOOL CLyrSettingDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	int i;
	CListBox* pSrcListBox;

	pSrcListBox = (CListBox*)GetDlgItem(IDC_LIST1);
	{//Prepare ListBox
		HFONT hFont;
		
		hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
		ASSERT(hFont != 0L);
		pSrcListBox->SendMessage(WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	}

	//Set Title
	SetWindowText(m_pszTitle);

	for(i = 0; i < m_strArrayTags.GetSize(); i++)
	{
		CString strToAdd;
		int iIndex;

		strToAdd.Format(m_strLBItemFmt, m_strArrayTags[i], m_strArrayVals[i]);
		iIndex = pSrcListBox->AddString(strToAdd);
		pSrcListBox->SetItemData(iIndex, i);
	}

	((CEdit*)GetDlgItem(IDC_EDIT1))->LimitText(31);
	GetDlgItem(IDUPDATE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT1)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_LABEL)->EnableWindow(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLyrSettingDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CLyrSettingDlg::OnSelChangeList() 
{
	CListBox* pSrcListBox;
	int iIndex;

	pSrcListBox = (CListBox*)GetDlgItem(IDC_LIST1);
	iIndex = pSrcListBox->GetCurSel();
	
	GetDlgItem(IDUPDATE)->EnableWindow((iIndex != LB_ERR));
	GetDlgItem(IDC_EDIT1)->EnableWindow((iIndex != LB_ERR));
	GetDlgItem(IDC_STATIC_LABEL)->EnableWindow((iIndex != LB_ERR));
	if(iIndex == LB_ERR)
	{
		return;
	}
	iIndex = (int)pSrcListBox->GetItemData(iIndex);
	GetDlgItem(IDC_EDIT1)->SetWindowText(m_strArrayVals[iIndex]);
}

void CLyrSettingDlg::OnUpdate() 
{
	CListBox* pSrcListBox;
	int iIndex, iArrayIndex;
	CString strItem;

	pSrcListBox = (CListBox*)GetDlgItem(IDC_LIST1);
	iIndex = pSrcListBox->GetCurSel();
	
	GetDlgItem(IDUPDATE)->EnableWindow((iIndex != LB_ERR));
	GetDlgItem(IDC_EDIT1)->EnableWindow((iIndex != LB_ERR));
	GetDlgItem(IDC_STATIC_LABEL)->EnableWindow((iIndex != LB_ERR));
	if(iIndex == LB_ERR)
	{
		AfxMessageBox("Nothing selected for update", MB_ICONSTOP);
		return;
	}
	GetDlgItem(IDC_EDIT1)->GetWindowText(strItem);
	if(m_bIsForACADSymbol)
	{
		if(!CSurvUtilApp::IsValidACADSymName(strItem))
		{
			AfxMessageBox("Invalid data specified", MB_ICONSTOP);
			GetDlgItem(IDC_EDIT1)->SetFocus();
			return;
		}
	}
	else
	{
		strItem.TrimLeft();
		strItem.TrimRight();
		if(strlen(strItem) <= 0)
		{
			AfxMessageBox("Invalid data specified", MB_ICONSTOP);
			GetDlgItem(IDC_EDIT1)->SetFocus();
			return;
		}
	}

	iArrayIndex = (int)pSrcListBox->GetItemData(iIndex);
	m_strArrayVals[iArrayIndex] = strItem;
	
	pSrcListBox->DeleteString(iIndex);
	
	strItem.Format(m_strLBItemFmt, m_strArrayTags[iArrayIndex], m_strArrayVals[iArrayIndex]);
	iIndex = pSrcListBox->AddString(strItem);
	pSrcListBox->SetItemData(iIndex, iArrayIndex);
}
BOOL CLyrSettingDlg::GetValues(const int& iSize, char** pszVals) const
{
	int i;

	if(iSize != m_strArrayVals.GetSize())
		return FALSE;
	for(i = 0; i < m_strArrayVals.GetSize(); i++)
	{
		strcpy(*(pszVals+i), m_strArrayVals[i]);
	}
	return TRUE;
}
