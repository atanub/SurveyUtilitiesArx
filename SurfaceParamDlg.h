#if !defined(AFX_SURFACEPARAMDLG_H__A68199C3_6BBD_11D5_A1F4_0080C8047C6F__INCLUDED_)
#define AFX_SURFACEPARAMDLG_H__A68199C3_6BBD_11D5_A1F4_0080C8047C6F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Resource.h"

// SurfaceParamDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSurfaceParamDlg dialog

class CSurfaceParamDlg : public CDialog
{
// Construction
public:
	CSurfaceParamDlg(const CStringArray& strArrayBlocks, const double& fScale,	const double& fMinSpace, CWnd* pParent = NULL);   // standard constructor
	void GetAttrib(CString& strSelBlk, double& fScale, double& fMinSpace);

// Dialog Data
	//{{AFX_DATA(CSurfaceParamDlg)
	enum { IDD = IDD_PROFILE_SURF_PARAM };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSurfaceParamDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSurfaceParamDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	const CStringArray& m_strArrayBlocks;
	CString m_strBlock;
	double  m_fScale;
	double  m_fMinSpace;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SURFACEPARAMDLG_H__A68199C3_6BBD_11D5_A1F4_0080C8047C6F__INCLUDED_)

