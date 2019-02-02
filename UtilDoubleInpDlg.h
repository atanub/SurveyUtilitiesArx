#if !defined(AFX_UTILDOUBLEINPDLG_H__A9F6FFE2_3DA6_11D5_86CD_D2A734C83526__INCLUDED_)
#define AFX_UTILDOUBLEINPDLG_H__A9F6FFE2_3DA6_11D5_86CD_D2A734C83526__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// UtilDoubleInpDlg.h : header file
//
#include "Resource.h"


/////////////////////////////////////////////////////////////////////////////
// CUtilDoubleInpDlg dialog

class CUtilDoubleInpDlg : public CDialog
{
// Construction
public:
	typedef enum ValidationType {Any = 0, ZeroNotAllowed = 1, NegNotAllowed = 2};
	CUtilDoubleInpDlg(const double& fVal, const ValidationType, const char* pszPmt, CWnd* pParent = NULL);   // standard constructor
	void GetAttribs(double& fVal) const { fVal = m_fVal;}

// Dialog Data
	//{{AFX_DATA(CUtilDoubleInpDlg)
	enum { IDD = IDD_UTIL_DOUBLE_INPUT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUtilDoubleInpDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUtilDoubleInpDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	double						m_fVal;
	const char*					m_pszPmt;
	const ValidationType		m_iValidationType;
};

BOOL GetPositiveDoubleVal(const char* pszPmt, const CUtilDoubleInpDlg::ValidationType, double& fVal);

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UTILDOUBLEINPDLG_H__A9F6FFE2_3DA6_11D5_86CD_D2A734C83526__INCLUDED_)
