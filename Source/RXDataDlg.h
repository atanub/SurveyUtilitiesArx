#if !defined(AFX_RXDATADLG_H__4990E5E4_CBCE_11D2_A75F_F81A04C13107__INCLUDED_)
#define AFX_RXDATADLG_H__4990E5E4_CBCE_11D2_A75F_F81A04C13107__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// RXDataDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRXDataDlg dialog
#include "Resource.h"


class CRXDataDlg : public CDialog
{
// Construction
public:
	CRXDataDlg(const CStringArray&, CString&, CString&, CString&, double&, double&, double&, BOOL&, BOOL&, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRXDataDlg)
	enum { IDD = IDD_RXD_OUT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRXDataDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
public:
	static BOOL GetRoadPlanData(CString&, CString&, CString&, double&, double&, double&, BOOL&, BOOL&);

// Implementation
protected:
	void GetAttribs(CString&, CString&, CString&, double&, double&, double&, BOOL&, BOOL&) const;

	// Generated message map functions
	//{{AFX_MSG(CRXDataDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	const CStringArray& m_BlkNameArray;
	
	CString m_strBlk;
	CString m_strLyrRXD;
	CString m_strLyrPID;
	double	m_dTxtRotAngle;
	double	m_dTxtSize;
	double	m_dBlkScale;
	BOOL	m_bZFlagRXD;
	BOOL	m_bZFlagPID;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RXDATADLG_H__4990E5E4_CBCE_11D2_A75F_F81A04C13107__INCLUDED_)
