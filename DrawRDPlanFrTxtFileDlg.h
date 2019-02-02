// DrawRDPlanFrTxtFileDlg.h : header file
//
#if !defined(_DRAWRDPLANFRTXTFILEDLG_H_)
#define _DRAWRDPLANFRTXTFILEDLG_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Resource.h"
#include "Structures.h"

/////////////////////////////////////////////////////////////////////////////
// CDrawRDPlanFrTxtFileDlg dialog
BOOL GetDrawParamRDPlanFrTxtFile(const BOOL&, DRAWPARAMRDPLANFRTXTFILE&);

class CDrawRDPlanFrTxtFileDlg : public CDialog
{
// Construction
public:
	CDrawRDPlanFrTxtFileDlg(const BOOL& bIsNEZ, const CStringArray&, DRAWPARAMRDPLANFRTXTFILE&, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDrawRDPlanFrTxtFileDlg)
	enum { IDD = IDD_RD_PLAN_FROM_TXT_FILE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDrawRDPlanFrTxtFileDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDrawRDPlanFrTxtFileDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnOpenSTN();
	afx_msg void OnOpenPID();
	afx_msg void OnOpenOUT();
	afx_msg void OnLyrSetting();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	DRAWPARAMRDPLANFRTXTFILE&		m_Rec;
	const CStringArray&				m_BlkNameArray;
	const BOOL&						m_bIsNEZ;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(_DRAWRDPLANFRTXTFILEDLG_H_)
