#if !defined(AFX_INSPTSDLG_H__B54AEBE1_D921_11D2_A760_081B04C13107__INCLUDED_)
#define AFX_INSPTSDLG_H__B54AEBE1_D921_11D2_A760_081B04C13107__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// InsPtsDlg.h : header file
//

#include "Resource.h"
/////////////////////////////////////////////////////////////////////////////
// CInsPtsDlg dialog


class CInsPtsDlg : public CDialog
{
// Construction
public:
	static BOOL GetInsPtsParams(const CStringArray&, BOOL&, double&, double&, CString&, CString&, BOOL&, BOOL&, BOOL&);
public:
	CInsPtsDlg(const CStringArray&, CWnd* pParent = NULL);   // standard constructor
	void GetAttribs(BOOL&, double&, double&, CString&, CString&, BOOL&, BOOL&, BOOL&);

// Dialog Data
	//{{AFX_DATA(CInsPtsDlg)
	enum { IDD = IDD_PIN_OUT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInsPtsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInsPtsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCheckSplevelFlag();
	afx_msg void OnClickShowPtNo();
	afx_msg void OnClickShowDescFlag();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	const CStringArray& m_BlkNameArray;
private:
	double	m_dTxtSize,
			m_dPtAnnTxtAngle;
    CString	m_strBlk,
			m_strLyr;
	BOOL	m_bDescFlag,
			m_bSPLevelFlag,
			m_bShowPtNoFlag,
			m_bZFlag;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSPTSDLG_H__B54AEBE1_D921_11D2_A760_081B04C13107__INCLUDED_)
