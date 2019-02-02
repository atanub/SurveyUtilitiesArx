// RdPflNEZDataDlg.h : header file
//
#if !defined(_RDPFLNEZDATADLG_H)
#define _RDPFLNEZDATADLG_H 1

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Resource.h"
#include "Structures.h"


/////////////////////////////////////////////////////////////////////////////
// CRdPflNEZDataDlg dialog

class CRdPflNEZDataDlg : public CDialog
{
// Construction
public:
	CRdPflNEZDataDlg(const RDPFLNEZDATA&, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRdPflNEZDataDlg)
	enum { IDD = IDD_PROFILE_CHG_Z };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRdPflNEZDataDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
public:
	static BOOL GetRdPflNEZData(RDPFLNEZDATA&);

// Implementation
protected:
	void GetAttribs(RDPFLNEZDATA&);
protected:

	// Generated message map functions
	//{{AFX_MSG(CRdPflNEZDataDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnLyrSetting();
	afx_msg void OnSetAnnTags();
	afx_msg void OnSurfMarker();
	afx_msg void OnOutFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	const RDPFLNEZDATA& m_RdPflNEZDataRec; 
	RDPFLNEZDATA		m_RdPflNEZDataRecNew; 
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(_RDPFLNEZDATADLG_H)
