// InsBlkFrFileDlg.h : header file
//
#if !defined(_INSBLKFRFILEDLG_H_)
#define _INSBLKFRFILEDLG_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
#include "Resource.h"
#include "Structures.h"
/////////////////////////////////////////////////////////////////////////////
// CInsBlkFrFileDlg dialog


BOOL GetInsBlkFrFileParams(const CStringArray&, INSBLKFRFILEPARAMS&);

class CInsBlkFrFileDlg : public CDialog
{
// Construction
public:
	CInsBlkFrFileDlg(const CStringArray&, const INSBLKFRFILEPARAMS&, CWnd* pParent = NULL);   // standard constructor
	void GetAttribs(INSBLKFRFILEPARAMS&);

// Dialog Data
	//{{AFX_DATA(CInsBlkFrFileDlg)
	enum { IDD = IDD_INS_BLK_FROM_TXTFILE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInsBlkFrFileDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInsBlkFrFileDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCheck();
	afx_msg void OnOpenFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	const CStringArray& m_BlkNameArray;
	INSBLKFRFILEPARAMS	m_RecParam;
	BOOL				m_bDrawOnCurLyr;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(_INSBLKFRFILEDLG_H_)
