#if !defined(AFX_PflFromChgZDlg_H__A5B542A0_81C0_11D3_86CC_B76E4FE2BA71__INCLUDED_)
#define AFX_PflFromChgZDlg_H__A5B542A0_81C0_11D3_86CC_B76E4FE2BA71__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PflFromChgZDlg.h : header file
//
#include "Resource.h"
#include "ProfileFromOffZDatFile.h"

/////////////////////////////////////////////////////////////////////////////
// CPflFromChgZDlg dialog
BOOL GetPflFromChgZParam(PFLOFFZDWGPARAM&);

class CPflFromChgZDlg : public CDialog
{
// Construction
public:
	CPflFromChgZDlg(const PFLOFFZDWGPARAM&, CWnd* pParent = NULL);   // standard constructor
	void GetAttribs(PFLOFFZDWGPARAM&);

// Dialog Data
	//{{AFX_DATA(CPflFromChgZDlg)
	enum { IDD = IDD_RD_PROFILE_CHG_Z_FROM_DATAFILE};
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPflFromChgZDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPflFromChgZDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnOpenDataFile();
	afx_msg void OnLyrSetting();
	afx_msg void OnSetAnnTags();
	afx_msg void OnSurfMarker();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	PFLOFFZDWGPARAM	m_Rec;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PflFromChgZDlg_H__A5B542A0_81C0_11D3_86CC_B76E4FE2BA71__INCLUDED_)
