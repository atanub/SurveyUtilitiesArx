// ConvertFDataDlg.h : header file
//
#if !defined(AFX_CONVERTFDATADLG_H__50D23F81_EA13_11D2_A760_081B04C13107__INCLUDED_)
#define AFX_CONVERTFDATADLG_H__50D23F81_EA13_11D2_A760_081B04C13107__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Resource.h"
/////////////////////////////////////////////////////////////////////////////
// CConvertFDataDlg dialog

void ConvertFieldData();

class CConvertFDataDlg : public CDialog
{
// Construction
public:
	CConvertFDataDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConvertFDataDlg)
	enum { IDD = IDD_CONVERT_FIELD_DATA };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConvertFDataDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConvertFDataDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnButtonInpFile();
	afx_msg void OnButtonOutFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CString m_strOutFile,
			m_strInpFile;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONVERTFDATADLG_H__50D23F81_EA13_11D2_A760_081B04C13107__INCLUDED_)
