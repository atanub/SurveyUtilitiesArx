// SurvUtilApp.h : header file
//

#ifndef __SURVUTILAPP_H_
#define __SURVUTILAPP_H_

#include "SurvUtilToolbar.h"
#include "ResourceHelper.h"
#include "ProgDlg.h"
#include "SurveyBookReader.h"
#include "NEZProfileBookReader.h"
#include "FieldBookReader.h"
#include "RdPflNEZDataDlg.h"
#include "ParameterDataCache.h"

class CFindChgDlg;
class CSystemRegistryManager;

/////////////////////////////////////////////////////////////////////////////
// CSurvUtilApp 

class CSurvUtilApp
{
// Construction
protected:
	CSurvUtilApp(HINSTANCE hInstance);
	~CSurvUtilApp();

public:
	static CSurvUtilApp*		Instance() { return m_pTheOnlyInstance;}
	static CSurvUtilApp*		Initialize(HINSTANCE hInstance);
	static void					UnInitialize();
	static CParameterDataCache*	GetDataCache();
	static BOOL					HasValidLicense();
	static const char*			GetLicenseInfo();
	static BOOL					GetResetLicenseFlag();
	static BOOL					ResetLicenseInfo();

	//
	static void					RegisterCmd(const char*);
	static void					RegisterCmdGroup(const char*);
	//Utility Func.
	static BOOL		IsValidACADSymName(const char*);
	static BOOL		LoadContourARX(const BOOL& bPopMsg = TRUE); 
	static CString	GetThisAppPath();
	static void		SendCmdToAutoCAD(const CString& strCmd);
	
public:
	static void				DrawFromPIN();
	static void				OpenSurveyBookXLSFile();
	static void				JoinSelectedPts();

	static void				CompileThis();
	static BOOL				BatchConvert(const CMapStringToString&, CStringArray&);

	//'Road Profile' XLS file related *****
	static void OpenRoadProfileXLS();
	static void DrawRoadProfileXLS();
	//'Field Book' XLS file related *****
	static void OpenFieldBookXLS();
	static void DrawFieldBookXLS();
	
	//'Adjust Open Traverse' XLS file related *****
	static void AdjOpenTraverse();
	
	static void DrawRoadPlan();
	static void DrawXLSXSecn();
	static void DrawXLSProfile();

	static void AnnotateChg();
	static void AnnotateChgCore(const double& fChg, const double& fLeftOff, const double& fRiteOff, const double& fTxtSize, const char* pszLyr);

	//Progress Dialog Related....
	static BOOL IsCancelPressed();
	static void ProgDlgNextStep(const char*, const int& = 0);
	static void ShowProgressDlg(const char*, const int& = 0);
	static void HideProgressDlg();

protected:
	BOOL CompileThis(const CString&);
	
	BOOL DrawFromPIN(const BOOL&, const double&, const double&, const CString&, const CString&, const BOOL&, const BOOL&, const BOOL&);
	BOOL DrawRoadPlan(const CString&, const CString&, const CString&, const double&, const double&, const double&, const BOOL&, const BOOL&);
	
	BOOL HasMasterData() const {return m_SurveyBookXLSReader.HasMDFData(); }
	BOOL HasPINData() const {return m_SurveyBookXLSReader.HasPINData(); }

	//
	static BOOL AnnotateChg(const AcGePoint3dArray& ptArray, const double& fChg, const double& fTxtSize, const char* pszLyr, const double* pfLeftOffset, const double* pfRiteOffset);
	static BOOL XtractVertsFr2DPLine(const AcDbObjectId& objId, AcGePoint3dArray& Array, CString* pErrMsg = 0L);

	static void getLoadedApp(CStringArray& arrayOfApps);
	static BOOL IsContourARXLoaded(const CString& strARXApp);
protected:
	void CreateToolBar();

private:
	BOOL _OpenXLS(const CString&, const BOOL&, CStringArray&, CExcelReaderGen*, const BOOL& = TRUE);

// Attributes
private:
	CProgressDlg				m_ProgDlg;
	CSurvUtilToolbar*			m_pToolBar;
	CTBHandler*					m_pToolBarHandler;
	CFindChgDlg*				m_pFindChgDlg;
	CSystemRegistryManager*		m_pSystemRegistryManager;
	HINSTANCE					m_hInstance;
private:
	CSurveyBookReader			m_SurveyBookXLSReader;
	CString						m_strSurveyBookXLSFile;

	CNEZProfileBookReader		m_NEZProfileBookReader;
	CString						m_strNEZProfileXLSFile;
	
	CFieldBookReader			m_FieldBookReader;
	CString						m_strFieldBookXLSFile;
	//
	CParameterDataCache*		m_pTheParamDataCache;

	BOOL						m_bHasValidLicense;
	CString						m_strLicenseInfo;

public:
	static const CString		m_strConstFileOpenFilter;
	static const CString		m_strConstPflFileOpenFilter;
	static const CString		m_strConstCompiledFOpenFilter;
	static const CString		m_strConstDefXtnOfBINOutFile;
	static const CString		m_strConstFBookFOpenFilter;
	
	static const CString		m_strConstContourARXName;
	static const CString		m_strConstThisARXName;
private:
	static CSurvUtilApp*		m_pTheOnlyInstance;
	//
	static CStringArray			m_arrayRegisteredCmds;
	static CStringArray			m_arrayRegisteredCmdGroups;
};
/////////////////////////////////////////////////////////////////////////////
#endif //__SURVUTILAPP_H_

