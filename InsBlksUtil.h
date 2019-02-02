// InsBlksUtil.h : header file
//
#if !defined(AFX_INSBLKSUTIL_H__7AC34523_D959_11D2_A760_081B04C13107__INCLUDED_)
#define AFX_INSBLKSUTIL_H__7AC34523_D959_11D2_A760_081B04C13107__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
#include "Resource.h"
/////////////////////////////////////////////////////////////////////////////
// CInsBlksUtil dialog


class CInsBlksUtil : public CDialog
{
// Construction
public:
	CInsBlksUtil(const CStringArray&, CWnd* pParent = NULL);   // standard constructor
	void GetAttribs(BOOL&, CString&, CString&, double&, double&, double&, double&, BOOL&);

// Dialog Data
	//{{AFX_DATA(CInsBlksUtil)
	enum { IDD = IDD_INS_BLK_UTIL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInsBlksUtil)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
public:
	static BOOL GetInsBlksParams(const CStringArray&, CString&, CString&, double&, double&, double&, double&, BOOL&);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInsBlksUtil)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	const CStringArray& m_BlkNameArray;
	BOOL	m_bDrawOnCurLyr,
			m_bConsiderZ;
	CString m_strLyr,
			m_strBlk;
	double	m_dX,
			m_dY,
			m_dZ,
			m_dRotAngle;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSBLKSUTIL_H__7AC34523_D959_11D2_A760_081B04C13107__INCLUDED_)
