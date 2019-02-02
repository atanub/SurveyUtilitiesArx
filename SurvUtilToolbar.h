// CSurvUtilToolbar.h : header file
//

#ifndef __SURVUTILTOOLBAR_H_
#define __SURVUTILTOOLBAR_H_


/////////////////////////////////////////////////////////////////////////////
// CSurvUtilToolbar 

class CSurvUtilToolbar : public CToolBar
{
// Construction
public:
	CSurvUtilToolbar();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSurvUtilToolbar)
	//}}AFX_VIRTUAL
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Implementation
public:
	virtual ~CSurvUtilToolbar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSurvUtilToolbar)
	afx_msg void OnUpdAlwaysEnabled(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CTBHandler window

class CTBHandler : public CWnd
{
// Construction
public:
	CTBHandler();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTBHandler)
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTBHandler();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTBHandler)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
void CreateToolBar();
/////////////////////////////////////////////////////////////////////////////
#endif //__SURVUTILTOOLBAR_H_
