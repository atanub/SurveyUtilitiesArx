#if !defined(AFX_FBSHEETPARAMDLG_H__DD2B2142_9584_11D4_86CD_A7437D6A8743__INCLUDED_)
#define AFX_FBSHEETPARAMDLG_H__DD2B2142_9584_11D4_86CD_A7437D6A8743__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// FBSheetParamDlg.h : header file
//
#include "FBSheetData.h"
#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CFBSheetParamDlg dialog
BOOL GetFBSheetDwgParam(FBDWGPARAM&);

class CFBSheetParamDlg : public CDialog
{
// Construction
public:
	CFBSheetParamDlg(FBDWGPARAM&, CWnd* pParent = NULL);   // standard constructor
	void GetAttrib(FBDWGPARAM& Rec) const
	{
		Rec = m_RecDwgParam;
	}

// Dialog Data
	//{{AFX_DATA(CFBSheetParamDlg)
	enum { IDD = IDD_FB_SHEET_PARAM };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFBSheetParamDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFBSheetParamDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	FBDWGPARAM&  m_RecDwgParam;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FBSHEETPARAMDLG_H__DD2B2142_9584_11D4_86CD_A7437D6A8743__INCLUDED_)
