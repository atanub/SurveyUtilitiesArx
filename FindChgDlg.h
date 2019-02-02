#if !defined(AFX_FINDCHGDLG_H__26530EC2_1C05_11D6_86CD_85F4C2F61E29__INCLUDED_)
#define AFX_FINDCHGDLG_H__26530EC2_1C05_11D6_86CD_85F4C2F61E29__INCLUDED_


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// FindChgDlg.h : header file
//
#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CFindChgDlg dialog

class CFindChgDlg : public CDialog
{
// Construction
public:
	CFindChgDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFindChgDlg)
	enum { IDD = IDD_FIND_POINT_WRT_CHG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindChgDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFindChgDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	afx_msg LONG onAcadKeepFocus ( UINT, LONG );
	afx_msg void OnSysCommand ( UINT nID, LPARAM lParam );

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDCHGDLG_H__26530EC2_1C05_11D6_86CD_85F4C2F61E29__INCLUDED_)
