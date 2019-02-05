// File Name: SurvUtilApp.Cpp
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SurvUtilApp.h"
#include "Resource.h"
#include "ResourceHelper.h"
#include "ResultDlg.h"
#include "JoinSelPtsDlg.h"
#include "QProfile.h"
#include "Line2PLine.h"
#include "DrawUtil.h"
#include "Util.h"
#include "RoadPflChgZDlg.h"
#include "DrawCSFrXLSDlg.h"
#include "FBSheetParamDlg.h"
#include "OTravXLSReader.h"
#include "TraverseCorrectionDlg.h"
#include <dbents.h>
#include "InsPtsDlg.h"
#include "RXDataDlg.h"
#include "FindChgDlg.h"
#include "SystemRegistryManager.h"
#include "EntitySelectors\EntitySelector.h"
#include "QueryEntityProperty.h"


extern "C" HWND adsw_acadMainWnd();
const CString	CSurvUtilApp::m_strConstThisARXName		= CString("SurvUtilArx.Arx");
const CString	CSurvUtilApp::m_strConstContourARXName	= CString("ContourARX.Arx");
const CString	CSurvUtilApp::m_strConstFileOpenFilter	= CString("Survey WorkBooks (*.XLS)|*.XLS|Compiled Survey WorkBooks (*.XLX)|*.XLX||");
const CString	CSurvUtilApp::m_strConstPflFileOpenFilter	= CString("Survey WorkBooks (*.XLS)|*.XLS||");
const CString	CSurvUtilApp::m_strConstFBookFOpenFilter	= CString("Field Book WorkBooks (*.XLS)|*.XLS||");
const CString	CSurvUtilApp::m_strConstCompiledFOpenFilter = CString("Compiled Survey WorkBooks (*.XLX)|*.XLX||");
const CString	CSurvUtilApp::m_strConstDefXtnOfBINOutFile	= CString("XLX");

CStringArray	CSurvUtilApp::m_arrayRegisteredCmds;
CStringArray	CSurvUtilApp::m_arrayRegisteredCmdGroups;

CSurvUtilApp*	CSurvUtilApp::m_pTheOnlyInstance = 0L;

/////////////////////////////////////////////////////////////////////////////
// CSurvUtilApp

CSurvUtilApp::CSurvUtilApp(HINSTANCE hInstance)
{
	m_hInstance = hInstance;
	m_bHasValidLicense = FALSE;
	m_pFindChgDlg = 0L;
	m_pToolBarHandler = 0L;
	m_pTheParamDataCache = new CParameterDataCache();
	m_pSystemRegistryManager = 0L;

	m_pToolBar = 0L;
	CreateToolBar();
}
CSurvUtilApp::~CSurvUtilApp()
{
	if(m_pToolBar != 0L)
		delete m_pToolBar;
	
	if(m_pToolBarHandler != 0L)
		delete m_pToolBarHandler;
	
	if(acdbCurDwg())
	{
		CMDIFrameWnd *pFrame;
		pFrame = acedGetAcadFrame();
		pFrame->RecalcLayout(); // need this step to force some MFC initialization
	}
	delete m_pTheParamDataCache;

	//
	if(m_pFindChgDlg != 0L)
	{
		if(m_pFindChgDlg->GetSafeHwnd() != 0L)
			m_pFindChgDlg->DestroyWindow();
		
		delete m_pFindChgDlg;
	}
	if(m_pSystemRegistryManager != 0L)
		delete m_pSystemRegistryManager;
}
CSurvUtilApp* CSurvUtilApp::Initialize(HINSTANCE hInstance)
{
	CString strLicenseInfo;

	if(0L != m_pTheOnlyInstance)
		return m_pTheOnlyInstance;

	m_pTheOnlyInstance = new CSurvUtilApp(hInstance);
	{//Set up logger for Protection Process debugging
		char szModuleName[_MAX_PATH];
		CFileFind objFileFind;
		CString strLogFile;
	
		::GetModuleFileName(m_pTheOnlyInstance->m_hInstance, szModuleName, _MAX_PATH);
		objFileFind.FindFile(szModuleName);
		objFileFind.FindNextFile();
		strLogFile.Format("%s\\%s", objFileFind.GetRoot(), CApplcationProtector::GetDebugLogFileName(CLicenseManager::AppType::SurveyUtilARX));
		CProtectionDebugging::StartDebugLog(strLogFile, (EXPERT() > 4));	//Set Logging ON
		if(EXPERT() > 4)
		{
			ads_printf("\nApplication log [%s] has been created. Please contact developer with the log file, if you are facing any problem.", strLogFile);
		}
	}
	{//Licensing....
		CApplcationProtector::FaultType eFaultType;

		m_pTheOnlyInstance->m_pSystemRegistryManager = new CSystemRegistryManagerEx(m_arrayRegisteredCmds);
		eFaultType = m_pTheOnlyInstance->m_pSystemRegistryManager->IsCurInstallationOK();
		if(CApplcationProtector::FaultType::Success != eFaultType)
		{
			int iRet;

			ads_printf("... Load failed!\nPossible cause: Invalid license information!\n");
			if(CApplcationProtector::FaultType::UserCancelledActivation != eFaultType)
			{
				iRet = MessageBox(adsw_acadMainWnd(), "Failed to load application\nPossible cause: Invalid license information!\nWould you like to reset License Info ?\nNOTE: Reseting License Info will require new license key for proper functioning.", CSurvUtilApp::m_strConstThisARXName, MB_ICONSTOP|MB_YESNO);
				if(IDYES == iRet)
				{
					CSurvUtilApp::ResetLicenseInfo();
					AfxMessageBox("License info. removed. You will be prompted to provide license key file after restarting AutoCAD.", MB_ICONINFORMATION);
				}
			}
			return 0L;
		}
		if(0L != m_pTheOnlyInstance->m_pSystemRegistryManager->GetLicenseInfo())
		{
			strLicenseInfo = m_pTheOnlyInstance->m_pSystemRegistryManager->GetLicenseInfo();
		}
	}
	m_pTheOnlyInstance->m_bHasValidLicense = TRUE;
	m_pTheOnlyInstance->m_strLicenseInfo = strLicenseInfo;

	return m_pTheOnlyInstance;
}
void CSurvUtilApp::UnInitialize()
{
	delete m_pTheOnlyInstance;
	{//Remove all the cmd groups
		int i;

		for(i = 0; i < m_arrayRegisteredCmdGroups.GetSize(); i++)
		{
			acedRegCmds->removeGroup(m_arrayRegisteredCmdGroups[i]);
		}
	}
	CProtectionDebugging::EndDebugLog();
}
BOOL CSurvUtilApp::HasValidLicense()
{
	return CSurvUtilApp::Instance()->m_bHasValidLicense;
}
const char* CSurvUtilApp::GetLicenseInfo()
{
	return (LPCSTR)(CSurvUtilApp::Instance()->m_strLicenseInfo);
}
BOOL CSurvUtilApp::GetResetLicenseFlag()
{
	CSurvUtilApp* pTheOnlyInstance;

	pTheOnlyInstance = CSurvUtilApp::Instance();
	return pTheOnlyInstance->m_pSystemRegistryManager->GetResetLicenseFlag();
}
BOOL CSurvUtilApp::ResetLicenseInfo()
{
	return CSurvUtilApp::Instance()->m_pSystemRegistryManager->ResetLicenseRelatedData();
}
BOOL CSurvUtilApp::IsValidACADSymName(const char* pszSymName)
{
	int i, iLim;
	BOOL bIsValid;
	iLim = strlen(pszSymName);
	if(iLim <= 0) return FALSE;
	for(i = 0; i < iLim; i++)
	{
		const char chMemb = *(pszSymName + i);
		bIsValid = (((chMemb <= '9') && (chMemb >= '0')) ||
					((chMemb <= 'z') && (chMemb >= 'a')) || ((chMemb <= 'Z') && (chMemb >= 'A')) ||
					((chMemb == '$') || (chMemb == '_') || (chMemb == '-'))
				   );
		if(!bIsValid)
			return FALSE;
	}
	return TRUE;
}


//
//XLS File Related....General
//
BOOL CSurvUtilApp::_OpenXLS(const CString& strFile, const BOOL& bPopErrDlg, CStringArray& strArrayErrs, CExcelReaderGen* pXLReader, const BOOL& bResetReader /*=  TRUE*/) 
{
	CWaitCursor WaitCursor;
	CStringArray StrArrayErrs;
	CString strTmp;
	
	ASSERT(strArrayErrs.GetSize() == 0);

	strTmp.Format("Message : Opening Excel word book : \"%s\"", strFile);
	strArrayErrs.Add(strTmp);

	CSurvUtilApp::ShowProgressDlg("Initializing Excel...", 50);

	pXLReader->UnInitialize(TRUE);//reset data also
	if(!pXLReader->Initialize())
	{
		strTmp.Format("ERROR: Failed to initialize Microsoft Excel 97 Application!");
		strArrayErrs.Add(strTmp);
		CSurvUtilApp::HideProgressDlg();
		return FALSE;
	}
	ProgDlgNextStep("Opening XLS file...");
	if(!pXLReader->Open(strFile, strArrayErrs))
	{
		strTmp.Format("ERROR: Failed to open Microsoft Excel 97 file \"%s\"!", strFile);
		strArrayErrs.Add(strTmp);
		CSurvUtilApp::HideProgressDlg();
		return FALSE;
	}
	ProgDlgNextStep("Excel application opened...", 100);

	//Read Master Data
	ProgDlgNextStep("Reading Data Sheet(s)..", 1);
	if(!pXLReader->ReadAllSheets(&strArrayErrs))
	{
		CSurvUtilApp::HideProgressDlg();
		pXLReader->UnInitialize(TRUE);//reset data also
		if(strArrayErrs.GetSize() > 0)
		{
			if(AfxMessageBox("Errors encountered while reading data sheets...\nWould you like to view errors ?", MB_ICONINFORMATION|MB_YESNO) == IDYES)
			{
				ShowResultDlg(strArrayErrs);
			}
		}
		return FALSE;
	}
	ProgDlgNextStep("Done..", 100);
	CSurvUtilApp::HideProgressDlg();

	if(bPopErrDlg)
	{
		strTmp.Format("File \"%s\" processed successfully...\nWould you like to view log ?", strFile);
		if(AfxMessageBox(strTmp, MB_ICONINFORMATION|MB_YESNO) == IDYES)
		{
			ShowResultDlg(strArrayErrs);
		}
	}
	if(bResetReader)
		pXLReader->UnInitialize(FALSE);//Don't reset data

	return TRUE;
}
//XLS File Related....Survey(Master)
void CSurvUtilApp::OpenSurveyBookXLSFile()
{
	DWORD dwFlags;
	CSurvUtilApp* pApp;
	
	pApp = CSurvUtilApp::Instance();

	dwFlags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	CFileDialog Dlg(TRUE, 0L, 0L, dwFlags, CSurvUtilApp::m_strConstFileOpenFilter);
	if(Dlg.DoModal() == IDOK)
	{
		CStringArray StrArrayErrs;

		pApp->m_strSurveyBookXLSFile = Dlg.GetPathName();
		if((Dlg.GetFileExt().CompareNoCase(m_strConstDefXtnOfBINOutFile) != 0) && (Dlg.GetFileExt().CompareNoCase("XLS") != 0))
		{
			AfxMessageBox("Invalid file name selected, Failed to open file", MB_ICONSTOP);
			return;
		}
		if(Dlg.GetFileExt().CompareNoCase(m_strConstDefXtnOfBINOutFile) == 0)
		{//Open Compiled version...
			pApp->m_SurveyBookXLSReader.GetDataObjPtr()->ReadData(pApp->m_strSurveyBookXLSFile);
			ads_printf("\nOpened Compiled Survey Data file \"%s\"\n", pApp->m_strSurveyBookXLSFile);
			return;
		}

		ads_printf("\nPlease wait...");
		if(!pApp->_OpenXLS(pApp->m_strSurveyBookXLSFile, TRUE, StrArrayErrs, &(pApp->m_SurveyBookXLSReader)))
		{
			if(AfxMessageBox("Error encountered while reading data sheets...\nWould you like to view errors ?", MB_ICONINFORMATION|MB_YESNO) == IDYES)
			{
				ShowResultDlg(StrArrayErrs);
			}
			ads_printf("\nERROR: File not opened !");
			pApp->m_SurveyBookXLSReader.UnInitialize(FALSE);//Don't reset data
			return;
		}
		pApp->m_SurveyBookXLSReader.UnInitialize(FALSE);//Don't reset data
		ads_printf("\nGeneral Survey Data file %s opened\n", pApp->m_strSurveyBookXLSFile);
	}
}
BOOL CSurvUtilApp::BatchConvert(const CMapStringToString& strInNOutFiles, CStringArray& strArrayOfErrs)
{
	POSITION Posn;
	CString strKey, strVal, strTmp;
	CSurveyBookReader XLReader;
	CSurvUtilApp* pApp;
	
	pApp = CSurvUtilApp::Instance();

	for(Posn = strInNOutFiles.GetStartPosition(); Posn != 0L;)
	{
		strInNOutFiles.GetNextAssoc(Posn, strKey, strVal);
		if(pApp->_OpenXLS(strKey, FALSE, strArrayOfErrs, &XLReader, FALSE))
		{
			strTmp.Format("Message: File \"%s\" opened successfully", strKey);
			strArrayOfErrs.Add(strTmp);
			if(XLReader.GetDataObjPtr()->WriteData(strVal))
				strTmp.Format("Message: File \"%s\" converted to file \"%s\" successfully", strKey, strVal);
			else
				strTmp.Format("ERROR: File \"%s\" conversion (output : \"%s\") failed", strKey, strVal);
			strArrayOfErrs.Add(strTmp);
		}
		else
		{
			strTmp.Format("ERROR: File \"%s\" not proccessed properly", strKey);
			strArrayOfErrs.Add(strTmp);
		}
	}
	return TRUE;
}
BOOL CSurvUtilApp::CompileThis(const CString& strOutFile)
{
	if(!m_SurveyBookXLSReader.HasMDFData())
	{
		AfxMessageBox("ERROR: Survey Worksheet not opened yet", MB_ICONSTOP);
		return FALSE;
	}
	m_SurveyBookXLSReader.GetDataObjPtr()->WriteData(strOutFile);
	return TRUE;
}
void CSurvUtilApp::CompileThis()
{
	CSurvUtilApp* pApp;

	pApp = CSurvUtilApp::Instance();
	if(!pApp->HasMasterData())
	{
		AfxMessageBox("Opened Survey Worksheet doesn't contain enough data to compile", MB_ICONSTOP);
		return;
	}
	DWORD dwFlags;
	CString strFile;

	dwFlags = OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY;
	CFileDialog Dlg(FALSE, CSurvUtilApp::m_strConstDefXtnOfBINOutFile, 0L, dwFlags, CSurvUtilApp::m_strConstCompiledFOpenFilter);
	if(Dlg.DoModal() != IDOK)
		return;

	if(pApp->CompileThis(Dlg.GetPathName()))
	{
		ads_printf("\nCompiled File \"%s\" created successfully\n\n", (LPCSTR)(Dlg.GetPathName()));
	}
}

void CSurvUtilApp::JoinSelectedPts()
{
	CSurvUtilApp* pApp;

	pApp = CSurvUtilApp::Instance();
	if(!pApp->m_SurveyBookXLSReader.HasMDFData())
	{
		AfxMessageBox("ERROR: Survey Worksheet not opened yet", MB_ICONSTOP);
		return;
	}
	const CMasterDataRecs* pRecArray = pApp->m_SurveyBookXLSReader.GetDataObjPtr()->GetMasterDataRecPtr();
	if((pRecArray != 0L) && (pRecArray->GetSize() > 0))
	{
		AcGePoint3dArray Array;
		CString strLyr;
		BOOL bZFlag;

		if(!GetJoinSelPtsData(pRecArray, Array, strLyr, bZFlag))
			return;

		//Draw Lines.....
		LAYER(strLyr);
		POLYLINE(Array, strLyr, bZFlag);
		ads_printf("\n%d# of points joined successfully\n", Array.length());
		return;
	}
	AfxMessageBox("Survey Worksheet doesn't contain enough data for drawing", MB_ICONSTOP);
}
BOOL CSurvUtilApp::DrawFromPIN(const BOOL& bZFlag, const double& dTxtSize, const double& dPtAnnTxtAng, const CString& strBlk, const CString& strLyr, const BOOL& bSPLevelFlag, const BOOL& bDescFlag, const BOOL& bShowPtNoFlag)
{
	CSurveyData* pSurvDataPtr;
	if(!m_SurveyBookXLSReader.HasPINData())
		return FALSE;

	const double fTxtShiftFac = 0.25;
	CSurveyPoints PtArray;
	AcGePoint3d pt3dTmp;
	int i, iMasterRecIndex;
	pSurvDataPtr = m_SurveyBookXLSReader.GetDataObjPtr();
	const CMasterDataRecs*	pMasterRecArray = pSurvDataPtr->GetMasterDataRecPtr();
	const CMarkRecs* pMarkRecArray = pSurvDataPtr->GetPINDataRecPtr();

	if(!pSurvDataPtr->CreateSPtArrayFromMarkArray(*pMarkRecArray, PtArray))
	{
		DeallocatePtrArray(PtArray);
		return FALSE;
	}

	ads_printf("\nPlease wait....\n");
	if(bSPLevelFlag)
		LAYER(strLyr);

	for(i = 0; i < PtArray.GetSize(); i++)
	{
		CString strLyrName, strMark;
	
		//Extract Layer from MasterData
		const MARK* pMarkRec = pMarkRecArray->GetAt(i);
		iMasterRecIndex = CSurveyData::FindMark(*pMarkRec, *pMasterRecArray);
		if(iMasterRecIndex < 0)
		{
			ads_printf("\nFATAL ERROR: While proccesing Point #%d\n", i);
			break;
		}
		const MASTERDATA* pMasterRec = pMasterRecArray->GetAt(iMasterRecIndex);
		strLyrName = pMasterRec->szDesc;
		strMark.Format("(%d.%d.%d)", pMarkRec->lFBookID, pMarkRec->lPtSlNo, pMarkRec->lPtSlNoSub);

		const SURVEYPOINT* pSPoint = PtArray[i];
		pt3dTmp.x = pSPoint->fEasting;
		pt3dTmp.y = pSPoint->fNorthing;
		pt3dTmp.z = (bZFlag) ? pSPoint->fElev : 0.0;
		
		//Call Drawing Routines....

		LAYER(pMasterRec->szDesc);

		DRAWPOINT(pt3dTmp, pMasterRec->szDesc);
		//*******Attach XData here......*******
		
		//*******Attach XData here......*******
		
		if(bSPLevelFlag)
		{//Spot Level
			if(!DrawSpotLevel(pt3dTmp, strBlk, strLyr, DTOR(dPtAnnTxtAng), dTxtSize, 1.0))
			{
				ads_printf("\nFATAL ERROR: While proccesing Point #%d\n", i);
				break;
			}
		}
		if(bDescFlag)
		{//Point Desc..
			AcGePoint3d pt3dTmpA;

			ads_polar(asDblArray(pt3dTmp), DTOR(dPtAnnTxtAng), +fTxtShiftFac * dTxtSize, asDblArray(pt3dTmpA));
			TEXTLEFT(pt3dTmpA, pMasterRec->szDesc, DTOR(dPtAnnTxtAng), dTxtSize, pMasterRec->szDesc);
		}
		if(bShowPtNoFlag)
		{//Point #(MARK)
			AcGePoint3d pt3dTmpA;

			ads_polar(asDblArray(pt3dTmp), DTOR(dPtAnnTxtAng), -fTxtShiftFac * dTxtSize, asDblArray(pt3dTmpA));
			TEXTRIGHT(pt3dTmpA, strMark, DTOR(dPtAnnTxtAng), dTxtSize, pMasterRec->szDesc);
		}
		ads_printf("\rTotal %d# of points drawn", i + 1);
	}

	DeallocatePtrArray(PtArray);
	return TRUE;
}
void CSurvUtilApp::DrawRoadPlan()
{
	CString strLyrRXD, strLyrPID, strBlk;
	double dBlkScaleRel, dTxtSize, dTxtRotInRad;
	BOOL bZFlagRXD, bZFlagPID;
	CSurvUtilApp* pApp;
	
	pApp = CSurvUtilApp::Instance();

	if(!CRXDataDlg::GetRoadPlanData(strLyrRXD, strLyrPID, strBlk, dBlkScaleRel, dTxtSize, dTxtRotInRad, bZFlagRXD, bZFlagPID))
		return;

	if(!pApp->DrawRoadPlan(strLyrRXD, strLyrPID, strBlk, dBlkScaleRel, dTxtSize, dTxtRotInRad, bZFlagRXD, bZFlagPID))
	{
	}
}

BOOL CSurvUtilApp::DrawRoadPlan(const CString& strLyrRXD, const CString& strLyrPID, const CString& strBlk, const double& dBlkScaleRel, const double& dTxtSize, const double& dTxtRotInRad, const BOOL& bZFlagRXD, const BOOL& bZFlagPID)
{
	CSurveyData* pSurvDataPtr;
	AcGePoint3d pt3dTmpA, pt3dTmpB;
	
	if(!m_SurveyBookXLSReader.HasMDFData())
		return FALSE;
	if(!m_SurveyBookXLSReader.HasPIDData())
		return FALSE;
	if(!m_SurveyBookXLSReader.HasRXDData())
		return FALSE;

	pSurvDataPtr = m_SurveyBookXLSReader.GetDataObjPtr();

	{//Join Points of Intersection 
		int i;
		CSurveyPoints PtArray;
		const CMarkRecs*		pMarkRecArray	= pSurvDataPtr->GetPIDataRecPtr();

		if(!pSurvDataPtr->CreateSPtArrayFromMarkArray(*pMarkRecArray, PtArray))
		{
			DeallocatePtrArray(PtArray);
			return FALSE;
		}
		LAYER(strLyrPID);

		for(i = 1; i < PtArray.GetSize(); i++)
		{
			if(ads_usrbrk() == 1)
			{
				ads_printf("\nUser break!\n");
				DeallocatePtrArray(PtArray);
				return FALSE;
			}
			ads_printf("\rPlease wait...Joining Point of Intersection #%d && #%d", i, i+1);
			pt3dTmpA.x = (PtArray[i - 1])->fEasting;
			pt3dTmpA.y = (PtArray[i - 1])->fNorthing;
			pt3dTmpA.z = (bZFlagPID) ? (PtArray[i - 1])->fElev : 0.0;

			pt3dTmpB.x = (PtArray[i])->fEasting;
			pt3dTmpB.y = (PtArray[i])->fNorthing;
			pt3dTmpB.z = (bZFlagPID) ? (PtArray[i])->fElev : 0.0;

			LINE(pt3dTmpA, pt3dTmpB, strLyrPID);
			//*******Attach XData here......*******
			
			//*******Attach XData here......*******
		}
		DeallocatePtrArray(PtArray);
	}
	{//Draw XSec Points of 
		const CRXSecDataRecs*	pRXDRecArray	= pSurvDataPtr->GetRXSecDataRecPtr();
		int i, j;

		LAYER(strLyrRXD);

		for(i = 0; i < pRXDRecArray->GetSize(); i++)
		{
			CSurveyPoints PtArray;
			const ROADXSECDATA* pRec = (*pRXDRecArray)[i];

			if(ads_usrbrk() == 1)
			{
				ads_printf("\nUser break!\n");
				DeallocatePtrArray(PtArray);
				return FALSE;
			}
			ads_printf("\rPlease wait...Drawing Cross Section #%d", i+1);
			if(!pSurvDataPtr->CreateSPtArrayFromMarkArray(pRec->MarkArray, PtArray))
			{
				DeallocatePtrArray(PtArray);
				return FALSE;
			}
			for(j = 0; j < PtArray.GetSize(); j++)
			{
				if(ads_usrbrk() == 1)
				{
					ads_printf("\nUser break!\n");
					DeallocatePtrArray(PtArray);
					return FALSE;
				}
				ads_printf("\rPlease wait...Drawing Point #%d", j+1);

				pt3dTmpA.x = (PtArray[j])->fEasting;
				pt3dTmpA.y = (PtArray[j])->fNorthing;
				pt3dTmpA.z = (bZFlagRXD) ? (PtArray[j])->fElev : 0.0;

				DRAWPOINT(pt3dTmpA, strLyrRXD);
				//*******Attach XData here......*******
				
				//*******Attach XData here......*******
				if(!DrawSpotLevel(pt3dTmpA, strBlk, strLyrRXD, dTxtRotInRad, dTxtSize, dBlkScaleRel))
				{
					ads_printf("\nFATAL ERROR: While proccesing Point #%d\n", j);
					DeallocatePtrArray(PtArray);
					return FALSE;
				}
			}
			DeallocatePtrArray(PtArray);
		}
	}
	ads_printf("\nDone !\n");
	return TRUE;
}
void CSurvUtilApp::DrawXLSXSecn()
{
	XLSXSECDWGPARAM objDwgParam;
	CSurveyData* pSurvDataPtr;
	AcGePoint3d pt3dTmpA, pt3dTmpB;
	AcGePoint3d ptIns;
	CSurvUtilApp* pApp;
	
	pApp = CSurvUtilApp::Instance();
	if(!pApp->m_SurveyBookXLSReader.HasMDFData() || !pApp->m_SurveyBookXLSReader.HasXSNData())
	{
		AfxMessageBox("Data Sheet not proccessed yet\nPlease Proccess Survey Work Sheet & then try again", MB_ICONSTOP|MB_ICONERROR);
		return;
	}
	objDwgParam = *(pApp->m_pTheParamDataCache->GetXlsXSecDwgParam());
	if(!GetCSFrXLSDwgParam(objDwgParam))
	{
		return;
	}
	pApp->m_pTheParamDataCache->SetXlsXSecDwgParam(&objDwgParam);
	if(RTNORM != ads_getpoint(0L, "\nPick upper left corner of Drawing Sheet :", asDblArray(ptIns)))
	{
		ads_printf("\nERROR: upper left corner of Drawing Sheet picked");
		return;
	}
	pSurvDataPtr = pApp->m_SurveyBookXLSReader.GetDataObjPtr();
	{//
		CXSecFrXLSDwgGenerator dwgGenerator(objDwgParam);
		CStringArray strArray;

		if(!pSurvDataPtr->GetXSNDataForDwg(dwgGenerator))
		{
			AfxMessageBox("Data proccessing failed, Drawing not possible", MB_ICONSTOP|MB_ICONERROR);
			return;
		}
		if(!dwgGenerator.Draw(ptIns, &strArray))
		{
			AfxMessageBox("Not all the Cross Sections drawn successfully", MB_ICONSTOP|MB_ICONERROR);
			return;
		}
	}
	ads_printf("\nDone !\n");
}
void CSurvUtilApp::DrawXLSProfile()
{
	RDPFLNEZDATA objDwgParam;
	CSurveyData* pSurvDataPtr;
	CSurvUtilApp* pApp;
	
	pApp = CSurvUtilApp::Instance();
	
	if(!pApp->m_SurveyBookXLSReader.HasMDFData() || !pApp->m_SurveyBookXLSReader.HasPRDData())
	{
		AfxMessageBox("Data Sheet not proccessed yet\nPlease Proccess Survey Work Sheet & then try again", MB_ICONSTOP|MB_ICONERROR);
		return;
	}

	if(!CRdPflNEZDataDlg::GetRdPflNEZData(objDwgParam))
	{
		return;
	}
	if(RTNORM != ads_getpoint(0L, "\nPick lower left corner of Profile extent :", asDblArray(objDwgParam.PtOrg)))
	{
		ads_printf("\nERROR: lower left corner of Profile extent picked");
		return;
	}
	pSurvDataPtr = pApp->m_SurveyBookXLSReader.GetDataObjPtr();
	{//
		CProfileFrXLS dwgGenerator;
		CStringArray strArray;

		if(!pSurvDataPtr->GetPFLDataForDwg(dwgGenerator))
		{
			AfxMessageBox("Data proccessing failed, Drawing not possible", MB_ICONSTOP|MB_ICONERROR);
			return;
		}
		if(!dwgGenerator.Draw(objDwgParam.PtOrg, objDwgParam))
		{
			AfxMessageBox("Not all the Cross Sections drawn successfully", MB_ICONSTOP|MB_ICONERROR);
			return;
		}
	}
	ads_printf("\nDone !\n");
}

//
//'Road Profile' XLS file related ************************************************************
//
void CSurvUtilApp::OpenRoadProfileXLS()
{
	DWORD dwFlags;
	CSurvUtilApp* pApp;
	
	pApp = CSurvUtilApp::Instance();
#ifndef _DEBUG //------------ Release Mode ------------ 
	dwFlags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	CFileDialog Dlg(TRUE, 0L, 0L, dwFlags, CSurvUtilApp::m_strConstPflFileOpenFilter);
	if(Dlg.DoModal() != IDOK)
		return;
	if(Dlg.GetFileExt().CompareNoCase("XLS") != 0)
	{
		AfxMessageBox("Invalid file name selected, Failed to open file", MB_ICONSTOP);
		return;
	}
	pApp->m_strNEZProfileXLSFile = Dlg.GetPathName();
#else //------------ Debug Mode ------------ 
	pApp->m_strNEZProfileXLSFile = "d:\\Data\\XLS Sample\\Profile(Design and Existing Level).xls";
#endif	//_DEBUG

	{//
		CStringArray StrArrayErrs;

		ads_printf("\nPlease wait...");
		if(!pApp->_OpenXLS(pApp->m_strNEZProfileXLSFile, TRUE, StrArrayErrs, &(pApp->m_NEZProfileBookReader)))
		{
			ads_printf("\nERROR: File (\"%s\") reading failed !", pApp->m_strNEZProfileXLSFile);
			pApp->m_NEZProfileBookReader.UnInitialize(FALSE);//Don't reset data
			return;
		}
		pApp->m_NEZProfileBookReader.UnInitialize(FALSE);//Don't reset data
		ads_printf("\nRoad Profile Data file \"%s\" opened successfully\n", pApp->m_strNEZProfileXLSFile);
	}
}
void CSurvUtilApp::DrawRoadProfileXLS()
{
	RDPFL_CHG_Z_DWG_PARAM objDwgParam;
	CRoadProfile* pDataPtr;
	AcGePoint3d PtOrg;
	CSurvUtilApp* pApp;
	AcGePoint3dArray ptArrayExistingDesignProfile;
	double fStartChgOnExistingDesignProfile;
	
	pApp = CSurvUtilApp::Instance();
	
	if(!pApp->m_NEZProfileBookReader.HasRoadProfileData())
	{
		AfxMessageBox("Data Sheet not proccessed yet\nPlease Proccess Survey Work Sheet & then try again", MB_ICONSTOP|MB_ICONERROR);
		return;
	}
	objDwgParam = *(pApp->m_pTheParamDataCache->GetRDPflChgZDwgParam());
	if(!GetXLSRoadPflChgZParam(objDwgParam))
	{
		return;
	}
	pApp->m_pTheParamDataCache->SetRDPflChgZDwgParam(&objDwgParam);
	if(RTNORM != ads_getpoint(0L, "\nPick lower left corner of Profile extent :", asDblArray(PtOrg)))
	{
		ads_printf("\nERROR: No point picked");
		return;
	}
	fStartChgOnExistingDesignProfile = 0.0;

	pDataPtr = pApp->m_NEZProfileBookReader.GetRoadPflDataObjPtr();
	{//Select 3DPolyline Entity for "Existing Design Profile"
		CEntitySelectorEx polylineSelector(CEntitySelectorFactory::SelectorType::SinglePolyline3D);
	
		const AcDbObjectIdArray* pObjIDArray = polylineSelector->Select("\nSelect Existing Design Profile as 3D Polyline:");

		if((0l != pObjIDArray) && (pObjIDArray->length() > 0))
		{
			const bool bFlag = CQueryEntityProperty::Extract3dPolyVertices(pObjIDArray->at(0), ptArrayExistingDesignProfile);
			CString strPrompt;
			int iReturnValue;

			//Ask for Start chainage of the existing design Profile....
			ads_initget(RSG_NONEG, 0L);
			ads_real fUserInput;
			fStartChgOnExistingDesignProfile = pDataPtr->GetStartChainage();
			strPrompt.Format("Enter start chainage of the selected 'Existing Design Profile' <%s>:", RTOS(fStartChgOnExistingDesignProfile));
			iReturnValue = ads_getreal(strPrompt, &fUserInput);
			if(RTCAN == iReturnValue)
			{
				fStartChgOnExistingDesignProfile = -1.0;
			}
			if(RTNORM == iReturnValue)
			{
				fStartChgOnExistingDesignProfile = fUserInput;
			}
		}
	}
	{//
		AcGePoint3dArray* pPtArrayExistingDesignProfile;

		pPtArrayExistingDesignProfile = ((ptArrayExistingDesignProfile.length() > 0) && (fStartChgOnExistingDesignProfile >= 0.0)) ? &ptArrayExistingDesignProfile : 0L;
		pDataPtr->Draw(objDwgParam, PtOrg, pPtArrayExistingDesignProfile, ((pPtArrayExistingDesignProfile == 0L) ? 0L:&fStartChgOnExistingDesignProfile));
	}
	ads_printf("\nDone !\n");
}

//
//'Field Book' XLS file related ************************************************************
//"OpenFieldBookXLS ", "DrawFieldBookXLS ";

void CSurvUtilApp::OpenFieldBookXLS()
{
	DWORD dwFlags;
	CSurvUtilApp* pApp;
	
	pApp = CSurvUtilApp::Instance();

	dwFlags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	CFileDialog Dlg(TRUE, 0L, 0L, dwFlags, CSurvUtilApp::m_strConstFBookFOpenFilter);
	if(Dlg.DoModal() == IDOK)
	{
		CStringArray StrArrayErrs;

		pApp->m_strFieldBookXLSFile = Dlg.GetPathName();
		if(Dlg.GetFileExt().CompareNoCase("XLS") != 0)
		{
			AfxMessageBox("Invalid file name selected, Failed to open file", MB_ICONSTOP);
			return;
		}
		ads_printf("\nPlease wait...");
		if(!pApp->_OpenXLS(pApp->m_strFieldBookXLSFile, TRUE, StrArrayErrs, &(pApp->m_FieldBookReader)))
		{
			//if(AfxMessageBox("Error encountered while reading data sheets...\nWould you like to view errors ?", MB_ICONINFORMATION|MB_YESNO) == IDYES)
			//{
			//	ShowResultDlg(StrArrayErrs);
			//}
			ads_printf("\nERROR: File (%s) reading failed !", pApp->m_strFieldBookXLSFile);
			pApp->m_FieldBookReader.UnInitialize(FALSE);//Don't reset data
			return;
		}
		pApp->m_FieldBookReader.UnInitialize(FALSE);//Don't reset data
		ads_printf("\nRoad Profile Data file \"%s\" opened successfully\n", pApp->m_strFieldBookXLSFile);
	}
}
void CSurvUtilApp::DrawFieldBookXLS()
{
	CFBSheetData* pData;
	AcGePoint3d ptBase;
	FBDWGPARAM recDwgParam;
	CSurvUtilApp* pApp;
	
	pApp = CSurvUtilApp::Instance();
	pData = pApp->m_FieldBookReader.GetData();
	if(pData == 0L)
	{
		AfxMessageBox("Data Sheet not proccessed yet\nPlease Proccess Field Book Sheet & then try again", MB_ICONSTOP|MB_ICONERROR);
		return;
	}
	recDwgParam = *(pApp->m_pTheParamDataCache->GetFBookDwgParam());

	if(!GetFBSheetDwgParam(recDwgParam))
		return;

	pApp->m_pTheParamDataCache->SetFBookDwgParam(&recDwgParam);

	if(RTNORM != ads_getpoint(0L, "\nPick Base Point :", asDblArray(ptBase)))
		return;

	if(!pData->Draw(ptBase, recDwgParam))
	{
		AfxMessageBox("Error encountered while drawing !", MB_ICONSTOP|MB_ICONERROR);
		return;
	}
}

void CSurvUtilApp::AdjOpenTraverse()
{
	COTravXLSReader	objXLSReader;
	TRAVDWGPARAM recParam;
	CSurvUtilApp* pApp;
	
	pApp = CSurvUtilApp::Instance();

	//HardCoded Values
	strcpy(recParam.szLyrCalcTable,		"TRAV_CALC_TABLE");
	strcpy(recParam.szLyrTraverseOrg,	"TRAV_DWG_FIELD");
	strcpy(recParam.szLyrTraverseAdj,	"TRAV_DWG_ADJUSTED");
	if(!GetTraverseParam(recParam, TRUE))
		return;

	{//
		CStringArray StrArrayErrs;

		ads_printf("\nPlease wait...");
		if(!pApp->_OpenXLS(recParam.szInFile, FALSE, StrArrayErrs, &objXLSReader))
		{
			if(AfxMessageBox("Error encountered while reading data sheets...\nWould you like to view errors ?", MB_ICONINFORMATION|MB_YESNO) == IDYES)
			{
				ShowResultDlg(StrArrayErrs);
			}
			ads_printf("\nERROR: Failed to open file %s", recParam.szInFile);
			pApp->m_SurveyBookXLSReader.UnInitialize(FALSE);//Don't reset data
			return;
		}
		objXLSReader.UnInitialize(FALSE);//Don't reset data
		ads_printf("\nOpen Traverse Data file %s opened\n", recParam.szInFile);

		//Calculate Error & Adjust Traverse
		objXLSReader.GetData()->Adjust(recParam);
	}
}
void CSurvUtilApp::DrawFromPIN()
{
	CSurvUtilApp* pApp;
	
	pApp = CSurvUtilApp::Instance();
	if(!pApp->HasPINData())
	{
		AfxMessageBox("Opened Survey Worksheet doesn't contain enough data to draw", MB_ICONSTOP);
		return;
	}
	CStringArray BlkNameArray;
	BOOL bZFlag, bSPLevelFlag, bDescFlag, bShowPtNoFlag;
	double dTxtSize, dPtAnnTxtAng;
	CString strBlk, strLyr;

	GetBlockNames(BlkNameArray);

	if(!CInsPtsDlg::GetInsPtsParams(BlkNameArray, bZFlag, dTxtSize, dPtAnnTxtAng, strBlk, strLyr, bSPLevelFlag, bDescFlag, bShowPtNoFlag))
		return;

	pApp->DrawFromPIN(bZFlag, dTxtSize, dPtAnnTxtAng, strBlk, strLyr, bSPLevelFlag, bDescFlag, bShowPtNoFlag);
}
CParameterDataCache* CSurvUtilApp::GetDataCache()
{
	return CSurvUtilApp::Instance()->m_pTheParamDataCache;
}

//Progress Dialog Related....
BOOL CSurvUtilApp::IsCancelPressed()
{
	CSurvUtilApp* pApp;
	
	pApp = CSurvUtilApp::Instance();
	return pApp->m_ProgDlg.CheckCancelButton();
}
void CSurvUtilApp::ShowProgressDlg(const char* pszCaption, const int& iStartPosn /*= 0*/)
{
	CTemporaryResourceOverride ResOverride;
	CSurvUtilApp* pApp;
	
	pApp = CSurvUtilApp::Instance();
	VERIFY(adsw_acadMainWnd() != 0L);
	if(pApp->m_ProgDlg.m_hWnd == 0L)
		pApp->m_ProgDlg.Create(CWnd::FromHandle(adsw_acadMainWnd()));
	pApp->m_ProgDlg.SetStatus(pszCaption);
	pApp->m_ProgDlg.SetPos(iStartPosn);
}
void CSurvUtilApp::HideProgressDlg()
{
	CTemporaryResourceOverride ResOverride;
	CSurvUtilApp* pApp;
	
	pApp = CSurvUtilApp::Instance();
	VERIFY(adsw_acadMainWnd() != 0L);
	pApp->m_ProgDlg.DestroyWindow();
}
void CSurvUtilApp::ProgDlgNextStep(const char* pszCaption /*= 0L*/, const int& iStep /*= 0*/)
{
	CTemporaryResourceOverride ResOverride;
	CSurvUtilApp* pApp;
	
	pApp = CSurvUtilApp::Instance();
	VERIFY(adsw_acadMainWnd() != 0L);
	if(pszCaption != 0L)
		pApp->m_ProgDlg.SetStatus(pszCaption);
	if(iStep > 0)
		pApp->m_ProgDlg.SetPos(iStep);
	pApp->m_ProgDlg.StepIt();
}

//
void CSurvUtilApp::CreateToolBar()
{
	CMDIFrameWnd *pAcadFrame;
	BOOL bFlag;
	CTemporaryResourceOverride Res;
	
	ASSERT(m_pToolBar == 0L);
	m_pToolBar = new  CSurvUtilToolbar();
	if(m_pToolBar == 0L)
	{
		ads_alert("ERROR: Failed to create toolbar object");
		return;
	}
	
	pAcadFrame = acedGetAcadFrame();
	pAcadFrame->EnableDocking(CBRS_ALIGN_ANY);	// Enable AutoCAD's mainframe for docking
	pAcadFrame->RecalcLayout();					// need this step to force some MFC initialization

	bFlag = m_pToolBar->Create(pAcadFrame, WS_CHILD|WS_VISIBLE|CBRS_SIZE_DYNAMIC|CBRS_FLYBY|CBRS_TOP|CBRS_TOOLTIPS);
	if(!bFlag)
	{
		ads_alert("ERROR: Failed to create toolbar object");
		delete m_pToolBar;
		m_pToolBar = NULL;
		return;
	}
	//bFlag = m_pToolBar->LoadToolBar(IDR_TOOLBAR_SURVUTIL);
	bFlag = m_pToolBar->LoadToolBar(IDR_TOOLBAR_SURVUTIL_NEW);
	if(!bFlag)
	{
		ads_alert("ERROR: Failed to load toolbar object");
		delete m_pToolBar;
		m_pToolBar = NULL;
		return;
	}
	// create toolbar handler
	m_pToolBarHandler = new CTBHandler();
	
	if(m_pToolBarHandler == 0L)
	{
		ads_alert("ERROR: Failed to create toolbar object");
		delete m_pToolBar;
		m_pToolBar = NULL;
		return;
	}
	bFlag = m_pToolBarHandler->Create(NULL, NULL, WS_CHILD|WS_MINIMIZE, CRect(0,0,1,1), pAcadFrame, 10);
	if(!bFlag)
	{
		ads_alert("ERROR: Failed to create toolbar object");
		delete m_pToolBarHandler;
		m_pToolBarHandler = 0L;
		delete m_pToolBar;
		m_pToolBar = NULL;
		return;
	}
	m_pToolBar->GetToolBarCtrl().SetOwner(m_pToolBarHandler);
	/////////////////////////////////////////////////////////////////////////	
	m_pToolBar->EnableDocking(CBRS_ALIGN_ANY);
	m_pToolBar->SetWindowText(_T("Survey Utility"));
	pAcadFrame->DockControlBar(m_pToolBar, AFX_IDW_DOCKBAR_BOTTOM);
	pAcadFrame->ShowControlBar(m_pToolBar, TRUE, TRUE);
}

void CSurvUtilApp::AnnotateChg()
{
	CTemporaryResourceOverride ResOverride;
	CSurvUtilApp* pApp;
	
	pApp = CSurvUtilApp::Instance();

	if(pApp->m_pFindChgDlg == 0L)
		pApp->m_pFindChgDlg = new CFindChgDlg(CWnd::FromHandle(acedGetAcadFrame()->m_hWnd));
	if(pApp->m_pFindChgDlg->GetSafeHwnd() == 0L)
	{
		if(!pApp->m_pFindChgDlg->Create((UINT)IDD_FIND_POINT_WRT_CHG, CWnd::FromHandle(acedGetAcadFrame()->m_hWnd)))
		{
			delete pApp->m_pFindChgDlg;
			pApp->m_pFindChgDlg = 0L;
			return;
		}
	}
	pApp->m_pFindChgDlg->ShowWindow(SW_SHOWNORMAL);
	return;
}

BOOL CSurvUtilApp::AnnotateChg(const AcGePoint3dArray& ptArray, const double& fChg, const double& fTxtSize, const char* pszLyr, const double* pfLeftOffset, const double* pfRiteOffset)
{
	const double fXtnLineMF = 5.0;
	const int iLuPrec = LUPREC();
	int i;
	BOOL bFound;
	double fCumChg, fTmp, fAlgnAngle;
	AcGePoint3d ptResult, ptPrev, ptCur, ptTmp;

	fAlgnAngle = 0.0;
	LAYER(pszLyr, 1);
	for(i = 1, fCumChg = 0.0, bFound = FALSE; i < ptArray.length(); i++)
	{
		ptCur = ptArray[i];
		ptPrev = ptArray[i - 1];
		//
		fTmp = Dist2D(ptPrev, ptCur);
		//
		if((fChg >= fCumChg) && (fChg <= (fCumChg + fTmp)))
		{
			fAlgnAngle = Angle2D(ptPrev, ptCur);
			ads_polar(asDblArray(ptPrev), fAlgnAngle, (fChg - fCumChg), asDblArray(ptResult));
			bFound = TRUE;
			break;
		}
		fCumChg += fTmp;
	}
	if(!bFound)
		return FALSE;

	{//for Chg annotation
		AcGePoint3d ptLeft, ptRite;
	
		ads_polar(asDblArray(ptResult), (fAlgnAngle - (PI / 2.0)), (fTxtSize * fXtnLineMF), asDblArray(ptLeft));
		ads_polar(asDblArray(ptResult), (fAlgnAngle + (PI / 2.0)), (fTxtSize * fXtnLineMF), asDblArray(ptRite));
		
		LINE(ptLeft, ptRite, pszLyr);
		DRAWPOINT(ptResult, pszLyr);

		ads_polar(asDblArray(ptResult), (fAlgnAngle - (PI / 2.0)), (fTxtSize * fXtnLineMF) + fTxtSize, asDblArray(ptLeft));
		ads_polar(asDblArray(ptResult), (fAlgnAngle + (PI / 2.0)), (fTxtSize * fXtnLineMF) + fTxtSize, asDblArray(ptRite));

		TEXTMID(ptLeft, RTOS(fChg), fAlgnAngle, fTxtSize, pszLyr);
		TEXTMID(ptRite, RTOS(fChg), fAlgnAngle, fTxtSize, pszLyr);
	}
	//
	//for Offsets...
	//TODO ->> Draw Offsets
	if(pfLeftOffset != 0L)
	{
		CString strTmp;

		fTmp = fabs(*pfLeftOffset);
		ads_polar(asDblArray(ptResult), (fAlgnAngle + (PI / 2.0)), fTmp, asDblArray(ptTmp));
		DRAWPOINT(ptTmp, pszLyr);
		TEXTMID(ptTmp, "L", fAlgnAngle, fTxtSize, pszLyr);
		
		strTmp.Format("L:%s", RTOS(fTmp));
		TEXTMID(ptTmp, strTmp, fAlgnAngle, fTxtSize, pszLyr);
	}
	if(pfRiteOffset != 0L)
	{
		CString strTmp;

		fTmp = fabs(*pfRiteOffset);
		ads_polar(asDblArray(ptResult), (fAlgnAngle - (PI / 2.0)), fTmp, asDblArray(ptTmp));
		DRAWPOINT(ptTmp, pszLyr);
		
		strTmp.Format("R:%s", RTOS(fTmp));
		TEXTMID(ptTmp, strTmp, fAlgnAngle, fTxtSize, pszLyr);
	}
	return TRUE;
}
BOOL CSurvUtilApp::XtractVertsFr2DPLine(const AcDbObjectId& objId, AcGePoint3dArray& Array, CString* pErrMsg/*= 0L*/)
{
	AcGePoint3d pt3D;
	AcDbPolyline* pEnt;
	AcDbObject* pObj;
	Acad::ErrorStatus ErrCode;

	if(Array.length() > 0)
	{
		AfxMessageBox("Invalid parameter passed to 'XtractVertsFr2DPLine()'\nPlease inform developer!", MB_ICONSTOP);
		return FALSE;
	}
	//Extract vertices...
	ErrCode = acdbOpenObject(pObj, objId, AcDb::kForRead);
	if(ErrCode != Acad::eOk)
	{
		return FALSE;
	}
	if(pObj->isA() != AcDbPolyline::desc())
	{
		if(pErrMsg != 0L)
		{
			pErrMsg->Format("Entity selected is \"%s\"...not a 2d polyline\n", pObj->desc()->name());
		}
		else
		{
			ads_printf("\nERROR: Entity selected is \"%s\"...not a 2d polyline\n", pObj->desc()->name());
		}
		pObj->close();
		return FALSE;
	}
	pEnt = (AcDbPolyline*)pObj;
	if(!pEnt->isOnlyLines())
	{
		if(pErrMsg != 0L)
		{
			pErrMsg->Format("Selected 2D Polyline contains curved segments");
		}
		else
		{
			ads_printf("\nERROR: Selected 2D Polyline contains curved segments !\n");
		}
		pObj->close();
		return FALSE;
	}
	{//Extract points...
		int i;
		
		for(i = 0; i < (int)pEnt->numVerts(); i++)
		{
			AcGePoint2d pt2D;

			ErrCode = pEnt->getPointAt(i, pt2D);
			if(ErrCode != Acad::eOk)
			{
				if(pErrMsg != 0L)
				{
					pErrMsg->Format("Error occured while iterating vertices");
				}
				else
				{
					ads_printf("\nERROR: Error occured while iterating vertices!\n");
				}
				return FALSE;
			}
			pt3D.x = pt2D.x;
			pt3D.y = pt2D.y;
			pt3D.z = 0.0;
			Array.append(pt3D);
		}
	}
	pObj->close();
	return TRUE;
}

void CSurvUtilApp::AnnotateChgCore(const double& fChg, const double& fLeftOff, const double& fRiteOff, const double& fTxtSize, const char* pszLyr)
{
	const double* pfLeftOffset = (fLeftOff > 0.0)?(&fLeftOff):(0L);
	const double* pfRiteOffset = (fRiteOff > 0.0)?(&fRiteOff):(0L);
	ads_name selSet;
	ads_name entName;
	AcGePoint3d pt3D;
	AcGePoint3dArray Array;
	AcDbObjectId objId;
	long iNoOfEnt;
	CString strMsg;

	if(ads_ssget("I", 0L, 0L, 0L, selSet) != RTNORM)
	{
		ads_printf("\nERROR: No pre-selected entity found\n");
		AfxMessageBox("No pre-selected entity found", MB_ICONSTOP);
		return;
	}
	if(ads_sslength(selSet, &iNoOfEnt) != RTNORM)
	{
		ads_printf("\nERROR: AutoCAD System error !\n");
		ads_ssfree(selSet);//Free Selset
		return;
	}
	if(iNoOfEnt > 1)
	{
		ads_printf("\nERROR: More than one pre-selected entity(s) found\n");
		AfxMessageBox("More than one pre-selected entity(s) found", MB_ICONSTOP);
		ads_ssfree(selSet);//Free Selset
		return;
	}
	if(iNoOfEnt < 1)
	{
		ads_printf("\nERROR: No pre-selected entity found\n");
		AfxMessageBox("No pre-selected entity found", MB_ICONSTOP);
		ads_ssfree(selSet);//Free Selset
		return;
	}
	if(ads_ssname(selSet, 0, entName) != RTNORM)
	{
		ads_printf("\nERROR: AutoCAD System error !\n");
		ads_ssfree(selSet);//Free Selset
		return;
	}
	if(Acad::eOk != acdbGetObjectId(objId, entName))
	{
		ads_printf("\nERROR: AutoCAD System error !\n");
		ads_ssfree(selSet);//Free Selset
		return;
	}
	ads_ssfree(selSet);//Free Selset
	//
	if(!XtractVertsFr2DPLine(objId, Array, &strMsg))
	{
		ads_printf("\nERROR: %s\n", strMsg);
		AfxMessageBox(strMsg, MB_ICONSTOP);
		return;
	}
	if(!AnnotateChg(Array, fChg, fTxtSize, pszLyr, pfLeftOffset, pfRiteOffset))
	{
		strMsg.Format("Failed to Annotate Chainage %.3f", fChg);
		ads_printf("\nERROR: %s\n", (LPCSTR)strMsg);
		AfxMessageBox(strMsg, MB_ICONSTOP);
		return;
	}
	ads_printf("\nChainage %.3f annotated suceesfully\n", fChg);
}

CString CSurvUtilApp::GetThisAppPath()
{
	CTemporaryResourceOverride ResOverride;
	CString strPath;
	char szPath[_MAX_PATH+1];
	
	if(GetModuleFileName(CTemporaryResourceOverride::m_hInstanceDefault, szPath, _MAX_PATH) > 0)
	{
		strPath = szPath;
		return strPath;
	}
	strPath = "!!!ERROR!!!";
	return strPath;
}
void CSurvUtilApp::getLoadedApp(CStringArray& arrayOfApps)
{
	struct resbuf *pRBArx, *pRBAds, *pRBTemp;
	
	pRBArx = ads_arxloaded();
	pRBAds = ads_loaded();

	for(pRBTemp = pRBArx; pRBTemp != 0L; pRBTemp = pRBTemp->rbnext)
	{
 		arrayOfApps.Add(pRBTemp->resval.rstring);
	}
	for(pRBTemp = pRBAds; pRBTemp != 0L; pRBTemp = pRBTemp->rbnext)
	{
 		arrayOfApps.Add(pRBTemp->resval.rstring);
	}
	ads_relrb(pRBArx);
	ads_relrb(pRBAds);
}
BOOL CSurvUtilApp::IsContourARXLoaded(const CString& strARXApp)
{
	CStringArray arrayOfApps;
	int i;
	
	getLoadedApp(arrayOfApps);
	for(i = 0; i < arrayOfApps.GetSize(); i++)
	{
#ifdef _DEBUG
		ads_printf("\n Loaded ARX file %d = %s", i, arrayOfApps[i]);
#endif// _DEBUG
		if(arrayOfApps[i].CompareNoCase(strARXApp) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}
BOOL CSurvUtilApp::LoadContourARX(const BOOL& bPopMsg/* =TRUE*/) 
{
	CString strPath;
	CFileFind objFileFind;
	const int iPrintStat = 0;
	Adesk::Boolean bRet;
	BOOL bIsAppLoaded;
	
	bIsAppLoaded = IsContourARXLoaded(CSurvUtilApp::m_strConstContourARXName);//Check if the App. already loaded...
	if(bIsAppLoaded)
	{
		return TRUE;//Already loaded!!
	}
	strPath = GetThisAppPath();
	if(objFileFind.FindFile(strPath))
	{
		strPath = objFileFind.GetRoot();
		strPath += CString("\\");
		strPath += CSurvUtilApp::m_strConstContourARXName;
	}
#ifdef _DEBUG
	strPath = "d:/data/ContourDev/ContourArx/Debug/ContourArx.arx";
#endif//_DEBUG
	bRet = acrxDynamicLinker->loadModule(strPath, iPrintStat);
	if(bPopMsg && (bRet != Adesk::kTrue))
	{
		CString strTmp;
		
		strTmp.Format("ERROR: Application ContourARX not installed properly");
#ifdef _DEBUG
		strTmp.Format("ERROR: ContourARX Application file \"%s\" not found!\nLoading failed", strPath);
#endif//_DEBUG
		AfxMessageBox(strTmp, MB_ICONSTOP);
	}
	return(bRet == Adesk::kTrue);
}

void CSurvUtilApp::SendCmdToAutoCAD(const CString& strCmd)
{
	COPYDATASTRUCT cmdMsg;
	CString strCmdTemp(strCmd);

	cmdMsg.dwData = (DWORD)1;
	cmdMsg.cbData = (DWORD)_tcslen(strCmd) + 1;
	cmdMsg.lpData = strCmdTemp.GetBuffer(strCmdTemp.GetLength()+1) ;                   

	SendMessage(adsw_acadMainWnd(), WM_COPYDATA, (WPARAM)adsw_acadMainWnd(), (LPARAM)&cmdMsg);
}

void CSurvUtilApp::RegisterCmd(const char* pszCmd)
{
	int i;

	for(i = 0; i < m_arrayRegisteredCmds.GetSize(); i++)
	{
		if(m_arrayRegisteredCmds[i].CompareNoCase(pszCmd) == 0)
			return;//Already present
	}
	m_arrayRegisteredCmds.Add(pszCmd);
}
void CSurvUtilApp::RegisterCmdGroup(const char* pszCmd)
{
	int i;

	for(i = 0; i < m_arrayRegisteredCmdGroups.GetSize(); i++)
	{
		if(m_arrayRegisteredCmdGroups[i].CompareNoCase(pszCmd) == 0)
			return;//Already present
	}
	m_arrayRegisteredCmdGroups.Add(pszCmd);
}



