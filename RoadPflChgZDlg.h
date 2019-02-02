#if !defined(AFX_ROADPFLCHGZDLG_H__A5B542A0_81C0_11D3_86CC_B76E4FE2BA71__INCLUDED_)
#define AFX_ROADPFLCHGZDLG_H__A5B542A0_81C0_11D3_86CC_B76E4FE2BA71__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// RoadPflChgZDlg.h : header file
//
#include "RoadProfile.h"
#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CRoadPflChgZDlg dialog
BOOL GetXLSRoadPflChgZParam(RDPFL_CHG_Z_DWG_PARAM&);

class CRoadPflChgZDlg : public CDialog
{
// Construction
public:
	CRoadPflChgZDlg(const RDPFL_CHG_Z_DWG_PARAM&, CWnd* pParent = NULL);   // standard constructor
	void GetAttribs(RDPFL_CHG_Z_DWG_PARAM&);

// Dialog Data
	//{{AFX_DATA(CRoadPflChgZDlg)
	enum { IDD = IDD_RD_PROFILE_CHG_Z_FROM_XLS};
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRoadPflChgZDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRoadPflChgZDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnLyrSetting();
	afx_msg void OnSetAnnTags();
	afx_msg void OnSurfMarker();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	RDPFL_CHG_Z_DWG_PARAM	m_Rec;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROADPFLCHGZDLG_H__A5B542A0_81C0_11D3_86CC_B76E4FE2BA71__INCLUDED_)
