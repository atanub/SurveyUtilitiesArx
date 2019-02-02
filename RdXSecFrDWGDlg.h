#if !defined(AFX_RDXSECFRDWGDLG_H__143076E1_F45E_11D3_86CC_B92A2A4E01FB__INCLUDED_)
#define AFX_RDXSECFRDWGDLG_H__143076E1_F45E_11D3_86CC_B92A2A4E01FB__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// RdXSecFrDWGDlg.h : header file
//
#include "Resource.h"
#include "Structures.h"

/////////////////////////////////////////////////////////////////////////////
// CRdXSecFrDWGDlg dialog



class CRdXSecFrDWGDlg : public CDialog
{
// Construction
public:
	CRdXSecFrDWGDlg(RDXSECOFFELEVDATA&, AcGePoint3dArray&, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRdXSecFrDWGDlg)
	enum { IDD = IDD_XSEC_FROM_DWG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRdXSecFrDWGDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	static BOOL		GetRoadXSecFrDwgData(RDXSECOFFELEVDATA&);
	static void		FillCombo(CComboBox*, const CStringArray&);
	static void		InitSheetArray(CStringArray&);
	static void		ParseSheetSizeDataRec(const CString&, double&, double&);
protected:
	static CString	ConvertInternalFmt2GUIFmt(const CString&);
protected:
	const static char	m_chShAttrSeparetor;
protected:
	void _IntializeDatumRange();
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRdXSecFrDWGDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCheckAutoDatum();
	afx_msg void OnButtonAddRange();
	afx_msg void OnButtonDelRange();
	afx_msg void OnDblClkListLevRange();
	afx_msg void OnSelChangeListLevRange();
	afx_msg void OnSelChangeComboSheet();
	afx_msg void OnLyrSetting();
	afx_msg void OnSetAnnTags();
	afx_msg void OnSurfMarker();
	afx_msg void OnOpenCSDOutFile();
	afx_msg void OnOpenPIDOutFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	RDXSECOFFELEVDATA&		m_Rec;
	AcGePoint3dArray&		m_DatumRangeArray;
	CStringArray			m_SheetSizeArray;
protected:
	static const char*		m_pszArrayOfPIDPathNames[];
	static const int		m_iArrayOfPIDPathVals[];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RDXSECFRDWGDLG_H__143076E1_F45E_11D3_86CC_B92A2A4E01FB__INCLUDED_)
