#if !defined(AFX_CHGRANGEDLG_H__63C92FC2_1F37_11D6_86CD_A3D46E098A1D__INCLUDED_)
#define AFX_CHGRANGEDLG_H__63C92FC2_1F37_11D6_86CD_A3D46E098A1D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ChgRangeDlg.h : header file
//
#include "Resource.h"
/////////////////////////////////////////////////////////////////////////////
// CChgRangeDlg dialog

BOOL GetChgRange(const double&, const double&, double&, double&);
class CChgRangeDlg : public CDialog
{
// Construction
public:
	CChgRangeDlg(const double&, const double&, double&, double&, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChgRangeDlg)
	enum { IDD = IDD_CHG_RANGE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChgRangeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChgRangeDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	const double& m_fMax;
	const double& m_fMin;
	double& m_fValMin;
	double& m_fValMax;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHGRANGEDLG_H__63C92FC2_1F37_11D6_86CD_A3D46E098A1D__INCLUDED_)
