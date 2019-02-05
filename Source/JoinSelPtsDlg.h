#if !defined(AFX_JOINSELPTSDLG_H__73E03581_15D4_11D3_A760_4C4807C1440E__INCLUDED_)
#define AFX_JOINSELPTSDLG_H__73E03581_15D4_11D3_A760_4C4807C1440E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// JoinSelPtsDlg.h : header file
//
#include "Resource.h"
#include "SurveyData.h"
#include <acgi.h>

/////////////////////////////////////////////////////////////////////////////
// CJoinSelPtsDlg dialog

BOOL GetJoinSelPtsData(const CMasterDataRecs*, AcGePoint3dArray&, CString&, BOOL&);

class CJoinSelPtsDlg : public CDialog
{
// Construction
public:
	CJoinSelPtsDlg(const CMasterDataRecs*, CWnd* pParent = NULL);   // standard constructor
	~CJoinSelPtsDlg();
	const CMasterDataRecs* GetSelRecArray() const { return &m_SelectedMDataRecs;}
	void GetAttribs(CString& strLyr, BOOL& bZFlag) const { strLyr = m_strLyr; bZFlag = m_bZFlag;}


// Dialog Data
	//{{AFX_DATA(CJoinSelPtsDlg)
	enum { IDD = IDD_DIALOG_JOIN_SEL_PTS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJoinSelPtsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CJoinSelPtsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnDel();
	afx_msg void OnDelAll();
	afx_msg void OnDblClkListDest();
	afx_msg void OnDblClkListSrc();
	virtual void OnOK();
	afx_msg void OnSelChangeListSrc();
	afx_msg void OnSelChangeListDest();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	static const  CString	m_strConstSPointFmt;
	const CMasterDataRecs*	m_pAvailableMDataRecs;
	CMasterDataRecs			m_SelectedMDataRecs;
	CString					m_strLyr;
	BOOL					m_bZFlag;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JOINSELPTSDLG_H__73E03581_15D4_11D3_A760_4C4807C1440E__INCLUDED_)
