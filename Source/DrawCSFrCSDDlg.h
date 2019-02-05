// DrawCSFrCSDDlg.h : header file
//
#if !defined(AFX_DRAWCSFRCSDDLG_H__850D34C1_F800_11D3_86CC_C97D650903E4__INCLUDED_)
#define AFX_DRAWCSFRCSDDLG_H__850D34C1_F800_11D3_86CC_C97D650903E4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Resource.h"
#include "Structures.h"

BOOL GetCSDDwgParam(CSDDWGPARAM&);

/////////////////////////////////////////////////////////////////////////////
// CDrawCSFrCSDDlg dialog

class CDrawCSFrCSDDlg : public CDialog
{
// Construction
public:
	CDrawCSFrCSDDlg(CSDDWGPARAM&, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDrawCSFrCSDDlg)
	enum { IDD = IDD_XSEC_FROM_OFF_ELEV_DATA };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDrawCSFrCSDDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDrawCSFrCSDDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChangeComboSheet();
	afx_msg void OnOpenCSD();
	afx_msg void OnLyrSetting();
	afx_msg void OnSetAnnTags();
	afx_msg void OnSurfMarker();
	afx_msg void OnCheckAutoDatum();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	CSDDWGPARAM& m_Rec;
	CStringArray m_SheetSizeArray;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DRAWCSFRCSDDLG_H__850D34C1_F800_11D3_86CC_C97D650903E4__INCLUDED_)
