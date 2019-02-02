// JoinSelPtsDlg.cpp : implementation file
//

#include "Stdafx.h"
#include "JoinSelPtsDlg.h"
#include "SurvUtilApp.h"
#include "ResourceHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HWND adsw_acadMainWnd();

/////////////////////////////////////////////////////////////////////////////
// CJoinSelPtsDlg dialog

const CString CJoinSelPtsDlg::m_strConstSPointFmt = CString("N: %.4f\r\nE: %.4f\r\nZ: %.4f");

CJoinSelPtsDlg::CJoinSelPtsDlg(const CMasterDataRecs* pRecArray, CWnd* pParent /*=NULL*/)
	: CDialog(CJoinSelPtsDlg::IDD, pParent), m_pAvailableMDataRecs(pRecArray)
{
	ASSERT(m_pAvailableMDataRecs != 0L);
	//{{AFX_DATA_INIT(CJoinSelPtsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}
CJoinSelPtsDlg::~CJoinSelPtsDlg()
{
	DeallocatePtrArray(m_SelectedMDataRecs);
}

void CJoinSelPtsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJoinSelPtsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CJoinSelPtsDlg, CDialog)
	//{{AFX_MSG_MAP(CJoinSelPtsDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DEL, OnDel)
	ON_BN_CLICKED(IDC_DELALL, OnDelAll)
	ON_LBN_DBLCLK(IDC_LIST_DEST, OnDblClkListDest)
	ON_LBN_DBLCLK(IDC_LIST_SRC, OnDblClkListSrc)
	ON_LBN_SELCHANGE(IDC_LIST_SRC, OnSelChangeListSrc)
	ON_LBN_SELCHANGE(IDC_LIST_DEST, OnSelChangeListDest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJoinSelPtsDlg message handlers

BOOL CJoinSelPtsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	HFONT hFont;
	CListBox* pSrcListBox;
	
	pSrcListBox = (CListBox*)GetDlgItem(IDC_LIST_SRC);
	hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	ASSERT(hFont != 0L);
	pSrcListBox->SendMessage(WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	GetDlgItem(IDC_LIST_DEST)->SendMessage(WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	
	{//Fill SRC list box
		int i;

		for(i = 0; i < m_pAvailableMDataRecs->GetSize(); i++)
		{
			MASTERDATA* pRec;
			CString strMark;
			int iIndex;
			
			pRec = m_pAvailableMDataRecs->GetAt(i);
			strMark = CSurveyData::MarkToShortString(pRec->Mark);
			iIndex = pSrcListBox->AddString(strMark);
			pSrcListBox->SetItemData(iIndex, (long)pRec);
		}
	}
	GetDlgItem(IDC_EDIT_DET_DEST)->SetWindowText("Nothing selected");
	GetDlgItem(IDC_EDIT_DET_SRC)->SetWindowText("Nothing selected");
	((CButton*)GetDlgItem(IDC_CHECK))->SetCheck(TRUE);

	return FALSE;
}
void CJoinSelPtsDlg::OnOK() 
{
	// TODO: Add extra validation here
	int i, iCount;
	MASTERDATA* pRec;

	iCount = ((CListBox*)GetDlgItem(IDC_LIST_DEST))->GetCount();
	if(iCount <= 0)
	{
		AfxMessageBox("ERROR: Nothing selected for drawing", MB_ICONSTOP);
		GetDlgItem(IDC_LIST_DEST)->SetFocus();
		return;
	}
	GetDlgItem(IDC_EDIT_LYR)->GetWindowText(m_strLyr);
	if(!CSurvUtilApp::IsValidACADSymName(m_strLyr))
	{
		AfxMessageBox("ERROR: Invalid layer name", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_LYR)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_LYR))->SetSel(0, -1);
		return;
	}
	//Form selected 'MASTERDATA' rec array
	DeallocatePtrArray(m_SelectedMDataRecs);
	for(i = 0; i < iCount; i++)
	{
		MASTERDATA* pNewRec;

		pRec = (MASTERDATA*) (((CListBox*)GetDlgItem(IDC_LIST_DEST))->GetItemData(i));
		pNewRec = new MASTERDATA;
		memcpy(pNewRec, pRec, sizeof(MASTERDATA));
		m_SelectedMDataRecs.Add(pNewRec);
	}
	m_bZFlag = ((CButton*)GetDlgItem(IDC_CHECK))->GetCheck();
	CDialog::OnOK();
}

void CJoinSelPtsDlg::OnAdd() 
{
	int iIndex;

	iIndex = ((CListBox*)GetDlgItem(IDC_LIST_SRC))->GetCurSel();
	if(iIndex == LB_ERR)
	{
		AfxMessageBox("ERROR: Nothing selected to add", MB_ICONSTOP);
		return;
	}
	{//Add to selection(dest) & attach XData
		MASTERDATA* pRec;
		CString strMark;
		
		((CListBox*)GetDlgItem(IDC_LIST_SRC))->GetText(iIndex, strMark);
		pRec = (MASTERDATA*) (((CListBox*)GetDlgItem(IDC_LIST_SRC))->GetItemData(iIndex));
	
		iIndex = ((CListBox*)GetDlgItem(IDC_LIST_DEST))->GetCurSel();
		iIndex = ((CListBox*)GetDlgItem(IDC_LIST_DEST))->InsertString(iIndex, strMark);
		((CListBox*)GetDlgItem(IDC_LIST_DEST))->SetItemData(iIndex, (long)pRec);
	}
	GetDlgItem(IDC_EDIT_DET_DEST)->SetWindowText("Nothing selected");
}
void CJoinSelPtsDlg::OnDel() 
{
	int iIndex;

	iIndex = ((CListBox*)GetDlgItem(IDC_LIST_DEST))->GetCurSel();
	if(iIndex == LB_ERR)
	{
		AfxMessageBox("ERROR: Nothing selected to delete", MB_ICONSTOP);
		return;
	}
	((CListBox*)GetDlgItem(IDC_LIST_DEST))->DeleteString(iIndex);
	GetDlgItem(IDC_EDIT_DET_DEST)->SetWindowText("Nothing selected");
}
void CJoinSelPtsDlg::OnDelAll() 
{
	((CListBox*)GetDlgItem(IDC_LIST_DEST))->ResetContent();
}
void CJoinSelPtsDlg::OnDblClkListDest() 
{
	OnDel();
}
void CJoinSelPtsDlg::OnDblClkListSrc() 
{
	OnAdd();
}

void CJoinSelPtsDlg::OnSelChangeListSrc() 
{
	int iIndex;

	iIndex = ((CListBox*)GetDlgItem(IDC_LIST_SRC))->GetCurSel();
	if(iIndex == LB_ERR)
	{
		GetDlgItem(IDC_EDIT_DET_SRC)->SetWindowText("Nothing selected");
		return;
	}
	{//Add to selection(dest) & attach XData
		MASTERDATA* pRec;
		CString strToShow;

		pRec = (MASTERDATA*) (((CListBox*)GetDlgItem(IDC_LIST_SRC))->GetItemData(iIndex));
		strToShow.Format(CJoinSelPtsDlg::m_strConstSPointFmt, pRec->SPoint.fNorthing, pRec->SPoint.fEasting, pRec->SPoint.fElev);
		GetDlgItem(IDC_EDIT_DET_SRC)->SetWindowText(strToShow);
	}
}

void CJoinSelPtsDlg::OnSelChangeListDest() 
{
	int iIndex;

	iIndex = ((CListBox*)GetDlgItem(IDC_LIST_DEST))->GetCurSel();
	if(iIndex == LB_ERR)
	{
		GetDlgItem(IDC_EDIT_DET_DEST)->SetWindowText("Nothing selected");
		return;
	}
	{//Add to selection(dest) & attach XData
		MASTERDATA* pRec;
		CString strToShow;

		pRec = (MASTERDATA*) (((CListBox*)GetDlgItem(IDC_LIST_DEST))->GetItemData(iIndex));
		strToShow.Format(CJoinSelPtsDlg::m_strConstSPointFmt, pRec->SPoint.fNorthing, pRec->SPoint.fEasting, pRec->SPoint.fElev);
		GetDlgItem(IDC_EDIT_DET_DEST)->SetWindowText(strToShow);
	}
}
//
BOOL GetJoinSelPtsData(const CMasterDataRecs* pRecArrayParam, AcGePoint3dArray& Array, CString& strLyr, BOOL& bZFlag)
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;
	
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	CJoinSelPtsDlg Dlg(pRecArrayParam, pAcadWnd);
	if(Dlg.DoModal() != IDOK)
	{
		return FALSE;
	}
	int i;
	AcGePoint3d ptTmp;

	const CMasterDataRecs* pRecArray = Dlg.GetSelRecArray();
	Dlg.GetAttribs(strLyr, bZFlag);
	for(i = 0; i < pRecArray->GetSize(); i++)
	{
		MASTERDATA* pRec;

		pRec = pRecArray->GetAt(i);
		ptTmp.x = pRec->SPoint.fEasting;
		ptTmp.y = pRec->SPoint.fNorthing;
		ptTmp.z = (bZFlag) ? pRec->SPoint.fElev : 0.0;
		Array.append(ptTmp);
	}

	return TRUE;
}
