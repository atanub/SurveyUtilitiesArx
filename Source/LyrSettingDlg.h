#if !defined(AFX_LYRSETTINGDLG_H__FFE850E2_2EAE_11D5_86CD_AF83EE9D9927__INCLUDED_)
#define AFX_LYRSETTINGDLG_H__FFE850E2_2EAE_11D5_86CD_AF83EE9D9927__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// LyrSettingDlg.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CLyrSettingDlg dialog

class CLyrSettingDlg : public CDialog
{
// Construction
public:
	CLyrSettingDlg(const char*, const BOOL&, const int&, const char**, const char**, CWnd* pParent = NULL);   // standard constructor
	BOOL GetValues(const int&, char**) const;

// Dialog Data
	//{{AFX_DATA(CLyrSettingDlg)
	enum { IDD = IDD_DIALOG_LYR_SETTING };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLyrSettingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLyrSettingDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChangeList();
	afx_msg void OnUpdate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	const static CString	m_strLBItemFmt;
protected:
	CStringArray			m_strArrayTags;
	CStringArray			m_strArrayVals;
	const char*				m_pszTitle;
	const BOOL&				m_bIsForACADSymbol;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LYRSETTINGDLG_H__FFE850E2_2EAE_11D5_86CD_AF83EE9D9927__INCLUDED_)
