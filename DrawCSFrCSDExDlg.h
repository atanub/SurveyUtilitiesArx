// DrawCSFrCSDExDlg.h : header file
//
#if !defined(_DRAWCSFRCSDEXDLG_H_)
#define _DRAWCSFRCSDEXDLG_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Resource.h"
#include "Structures.h"


/////////////////////////////////////////////////////////////////////////////
// CDrawCSFrCSDExDlg dialog
BOOL GetCSDExDwgParam(CSDDWGPARAMEX&);

class CDrawCSFrCSDExDlg : public CDialog
{
// Construction
public:
	CDrawCSFrCSDExDlg(CSDDWGPARAMEX&, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDrawCSFrCSDExDlg)
	enum { IDD = IDD_XSEC_FROM_OFF_ELEV_DATA_EX };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDrawCSFrCSDExDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDrawCSFrCSDExDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChangeComboSheet();
	afx_msg void OnOpenCSD();
	afx_msg void OnOpenCSDDsgn();
	afx_msg void OnOpenLogFile();
	afx_msg void OnLyrSetting();
	afx_msg void OnSetAnnTags();
	afx_msg void OnSurfMarker();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	CSDDWGPARAMEX&		m_Rec;
	CStringArray		m_SheetSizeArray;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(_DRAWCSFRCSDEXDLG_H_)
