// BatCompileDlg.h : header file
//
#if !defined(AFX_BATCOMPILEDLG_H__0B7C3960_472C_11D3_A1D2_0080C8047C6F__INCLUDED_)
#define AFX_BATCOMPILEDLG_H__0B7C3960_472C_11D3_A1D2_0080C8047C6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Resource.h"


/////////////////////////////////////////////////////////////////////////////
// CBatCompileDlg dialog

class CBatCompileDlg : public CDialog
{
// Construction
public:
	CBatCompileDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBatCompileDlg)
	enum { IDD = IDD_COMPILE_XLS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBatCompileDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
public:
	static CString MakeDispStr(const CStringArray&);
	static void BatchCompileXLS();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBatCompileDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnButtonReset();
	afx_msg void OnButtonView();
	afx_msg void OnConvert();
	afx_msg void OnButtonAddToQue();
	afx_msg void OnSelXLSFile();
	afx_msg void OnSelOutFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HICON m_hIconAddToQue,
		  m_hIconVwList,	
		  m_hIconOutput;

	CMapStringToString m_MapInAndOutFile;
private:
	static const CString	m_strListCtrlVw;
	static const CString	m_strEditCtrlVw;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATCOMPILEDLG_H__0B7C3960_472C_11D3_A1D2_0080C8047C6F__INCLUDED_)
