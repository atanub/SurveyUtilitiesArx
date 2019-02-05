// DrawCSFrXLSDlg.h : header file
//
#if !defined(_DRAWCSFRXLSDLG_H_)
#define _DRAWCSFRXLSDLG_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Resource.h"
#include "Structures.h"

BOOL GetCSFrXLSDwgParam(XLSXSECDWGPARAM&);

/////////////////////////////////////////////////////////////////////////////
// CDrawCSFrXLSDlg dialog

class CDrawCSFrXLSDlg : public CDialog
{
// Construction
public:
	CDrawCSFrXLSDlg(XLSXSECDWGPARAM&, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDrawCSFrXLSDlg)
	enum { IDD = IDD_XSEC_XLS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDrawCSFrXLSDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDrawCSFrXLSDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChangeComboSheet();
	afx_msg void OnLyrSetting();
	afx_msg void OnSetAnnTags();
	afx_msg void OnSurfMarker();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	XLSXSECDWGPARAM&		m_Rec;
	CStringArray			m_SheetSizeArray;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(_DRAWCSFRXLSDLG_H_)
