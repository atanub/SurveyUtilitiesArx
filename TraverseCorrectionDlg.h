#if !defined(AFX_TRAVERSECORRECTIONDLG_H__B9145B42_CBC2_11D4_86CD_896C0F95C12F__INCLUDED_)
#define AFX_TRAVERSECORRECTIONDLG_H__B9145B42_CBC2_11D4_86CD_896C0F95C12F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TraverseCorrectionDlg.h : header file
//
#include "Resource.h"
#include "TraverseAdj.h"

/////////////////////////////////////////////////////////////////////////////
// CTraverseCorrectionDlg dialog

BOOL GetTraverseParam(TRAVDWGPARAM&, const BOOL&);

class CTraverseCorrectionDlg : public CDialog
{
// Construction
public:
	CTraverseCorrectionDlg(const TRAVDWGPARAM&, const BOOL&, CWnd* pParent = NULL);   // standard constructor
	void GetAttrib(TRAVDWGPARAM& Rec) const {Rec = m_Rec;}


// Dialog Data
	//{{AFX_DATA(CTraverseCorrectionDlg)
	enum { IDD = IDD_DIALOG_TRAVERSE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTraverseCorrectionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTraverseCorrectionDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonInpFile();
	afx_msg void OnButtonOutFile();
	afx_msg void OnDraw();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	TRAVDWGPARAM	m_Rec;
	const BOOL&		m_bIsInpFromXLS;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRAVERSECORRECTIONDLG_H__B9145B42_CBC2_11D4_86CD_896C0F95C12F__INCLUDED_)
