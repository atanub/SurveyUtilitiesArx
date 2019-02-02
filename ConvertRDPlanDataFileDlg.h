// ConvertRDPlanDataFileDlg.h : header file
//
#if !defined(_CONVERTRDPLANDATAFILEDLG_H_)
#define _CONVERTRDPLANDATAFILEDLG_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CConvertRDPlanDataFileDlg dialog
BOOL GetRDPlanDataFileConversionData(CString& strInpFile, CString& strOutFile, double& fValIncr);

class CConvertRDPlanDataFileDlg : public CDialog
{
// Construction
public:
	CConvertRDPlanDataFileDlg(CWnd* pParent = NULL);   // standard constructor
	void GetAttribs(CString& strInpFile, CString& strOutFile, double& fValIncr)
	{
		strInpFile = m_strInpFile;
		strOutFile = m_strOutFile;
		fValIncr = m_fValIncr;
	}

// Dialog Data
	//{{AFX_DATA(CConvertRDPlanDataFileDlg)
	enum { IDD = IDD_RD_PLAN_TXT_FILE_MODIFY };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConvertRDPlanDataFileDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConvertRDPlanDataFileDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnOpenSTN();
	afx_msg void OnOpenOUT();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	CString	m_strInpFile;
	CString	m_strOutFile;
	double	m_fValIncr;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(_CONVERTRDPLANDATAFILEDLG_H_)
