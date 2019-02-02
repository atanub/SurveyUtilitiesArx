// SurvUtilArx.cpp
/////////////////////////////////////////////////////////

#include "stdafx.h"           // MFC stuff

#include "SurvUtilApp.h"
#include "Commands.h"

#include "ResourceHelper.h"   // temporary resource class
#include "AboutDlg.h"
#include "ConvertFDataDlg.h"
#include "BatCompileDlg.h"
#include "InsBlksUtil.h"
#include "RdPflNEZDataDlg.h"
#include "Line2PLine.h"
#include "RXDataDlg.h"
#include "Purge.h"
#include "RdXSecFrDWGDlg.h"
#include "RdXSecFrDWG.h"
#include "XData.h"
#include "SurveyedStation.h"
#include "TraverseAdj.h"
#include "ProfileFromOffZDatFile.h"
#include "OTravData.h"
#include "ParameterDataCache.h"
#include "Line2PLineEx.h"
#include "SystemRegistryManager.h"
#include "AlignPointsOnSurveyedPath.h"

// helper functions
static BOOL initApp  (void);
static void unloadApp(void);

// MFC support
extern "C" HINSTANCE _hdllInstance;
extern "C" HWND adsw_acadMainWnd();

BOOL InitModule(HINSTANCE, DWORD, LPVOID);

// message handlers
static void OnkPreQuitMsg  (void);
//
static void InitMFC();
static void EndMFC();

/////////////////////////////////////////////////////////////////////
// This function is the entry point for your application.
/////////////////////////////////////////////////////////////////////
extern "C" AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void* pkt);
static AFX_EXTENSION_MODULE arxmfcDLL;

AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void* pkt)
{
  switch (msg)
  {
	case AcRx::kInitAppMsg:
		acrxDynamicLinker->unlockApplication(pkt);
		ads_printf("\nLoading ARX application: %s", CSurvUtilApp::m_strConstThisARXName);
		if(!initApp())
		{
			return AcRx::kRetError;
		}
		break;
	case AcRx::kUnloadAppMsg:
		unloadApp();
		break;
	case AcRx::kLoadDwgMsg:
		break;
	case AcRx::kPreQuitMsg:
		OnkPreQuitMsg();
		break;
	case AcRx::kUnloadDwgMsg:
	case AcRx::kInvkSubrMsg:
	case AcRx::kEndMsg:
	case AcRx::kQuitMsg:
	case AcRx::kSaveMsg:
	case AcRx::kCfgMsg:
	case AcRx::kDependencyMsg:
	case AcRx::kNoDependencyMsg:
	default:
		break;
	}
	return AcRx::kRetOK;
}
static void OnkPreQuitMsg(void)
{
	AcDbDatabase*	pDb;

	pDb = acdbCurDwg();
	if(pDb == 0L)
		return;
	pDb->setSaveproxygraphics(1);
}

static BOOL initApp(void)
{
	InitMFC();
	
	CSurvUtilApp::RegisterCmdGroup("SURVUTIL_ABOUT");
	acedRegCmds->addCommand("SURVUTIL_ABOUT", "AboutSurvUtil",			"AboutSurvUtil",		ACRX_CMD_MODAL|ACRX_CMD_TRANSPARENT|ACRX_CMD_NOMULTIPLE, CAboutDlg::ShowAboutDialog); 					CSurvUtilApp::RegisterCmd("AboutSurvUtil""AboutSurvUtil");

	CSurvUtilApp::RegisterCmdGroup("SURVUTIL_MASTER");
	acedRegCmds->addCommand("SURVUTIL_MASTER", "ConvertFieldData",		"ConvertFieldData",		ACRX_CMD_MODAL, ConvertFieldData);								CSurvUtilApp::RegisterCmd("ConvertFieldData");
	//
	acedRegCmds->addCommand("SURVUTIL_MASTER", "BatchCompileXLS",		"BatchCompileXLS",		ACRX_CMD_MODAL, CBatCompileDlg::BatchCompileXLS);				CSurvUtilApp::RegisterCmd("BatchCompileXLS");
	acedRegCmds->addCommand("SURVUTIL_MASTER", "OpenSurveyBookXLSFile",	"OpenSurveyBookXLSFile",ACRX_CMD_MODAL, CSurvUtilApp::OpenSurveyBookXLSFile);			CSurvUtilApp::RegisterCmd("OpenSurveyBookXLSFile");
	acedRegCmds->addCommand("SURVUTIL_MASTER", "CompileThis",			"CompileThis",			ACRX_CMD_MODAL, CSurvUtilApp::CompileThis);						CSurvUtilApp::RegisterCmd("CompileThis");
	
	acedRegCmds->addCommand("SURVUTIL_MASTER", "DrawRoadPlan",			"DrawRoadPlan",			ACRX_CMD_MODAL, CSurvUtilApp::DrawRoadPlan);					CSurvUtilApp::RegisterCmd("DrawRoadPlan");
	acedRegCmds->addCommand("SURVUTIL_MASTER", "DrawFromPIN",			"DrawFromPIN",			ACRX_CMD_MODAL, CSurvUtilApp::DrawFromPIN);						CSurvUtilApp::RegisterCmd("DrawFromPIN");
	acedRegCmds->addCommand("SURVUTIL_MASTER", "JoinSelectedPts",		"JoinSelectedPts",		ACRX_CMD_MODAL, CSurvUtilApp::JoinSelectedPts);					CSurvUtilApp::RegisterCmd("JoinSelectedPts");

	acedRegCmds->addCommand("SURVUTIL_MASTER", "DrawXLSXSecn",			"DrawXLSXSecn",			ACRX_CMD_MODAL, CSurvUtilApp::DrawXLSXSecn);					CSurvUtilApp::RegisterCmd("DrawXLSXSecn");
	acedRegCmds->addCommand("SURVUTIL_MASTER", "DrawXLSProfile",		"DrawXLSProfile",		ACRX_CMD_MODAL, CSurvUtilApp::DrawXLSProfile);					CSurvUtilApp::RegisterCmd("DrawXLSProfile");

	CSurvUtilApp::RegisterCmdGroup("SURVUTIL_GEN");
	acedRegCmds->addCommand("SURVUTIL_GEN", "InsBlocksFromDatFile",		"InsBlocksFromDatFile",		ACRX_CMD_MODAL, CCommands::InsBlocksFromDatFile);			CSurvUtilApp::RegisterCmd("InsBlocksFromDatFile");
	acedRegCmds->addCommand("SURVUTIL_GEN", "ModifyZValueOfSelEnts",	"ModifyZValueOfSelEnts",	ACRX_CMD_MODAL, CCommands::ModifyZValueOfSelEnts);			CSurvUtilApp::RegisterCmd("ModifyZValueOfSelEnts");
	acedRegCmds->addCommand("SURVUTIL_GEN", "ModifyTxtSizeOfSelEnts",	"ModifyTxtSizeOfSelEnts",	ACRX_CMD_MODAL, CCommands::ModifyTxtSizeOfSelEnts);			CSurvUtilApp::RegisterCmd("ModifyTxtSizeOfSelEnts");
	acedRegCmds->addCommand("SURVUTIL_GEN", "SetZValueOfSelEnts",		"SetZValueOfSelEnts",		ACRX_CMD_MODAL, CCommands::SetZValueOfSelEnts);				CSurvUtilApp::RegisterCmd("SetZValueOfSelEnts");
	acedRegCmds->addCommand("SURVUTIL_GEN", "ExportPoints",				"ExportPoints",				ACRX_CMD_MODAL, CCommands::ExportPoints);					CSurvUtilApp::RegisterCmd("ExportPoints");
	acedRegCmds->addCommand("SURVUTIL_GEN", "ExportPointsEx",			"ExportPointsEx",			ACRX_CMD_MODAL, CCommands::ExportPointsEx);					CSurvUtilApp::RegisterCmd("ExportPointsEx");
	acedRegCmds->addCommand("SURVUTIL_GEN", "JoinPoints",				"JoinPoints",				ACRX_CMD_MODAL, CCommands::JoinPoints);						CSurvUtilApp::RegisterCmd("JoinPoints");
	acedRegCmds->addCommand("SURVUTIL_GEN", "InsBlocks",				"InsBlocks",				ACRX_CMD_MODAL, CCommands::InsBlocks);						CSurvUtilApp::RegisterCmd("InsBlocks");
	acedRegCmds->addCommand("SURVUTIL_GEN", "AlignPointsOnSurveyedPath","AlignPointsOnSurveyedPath",ACRX_CMD_MODAL, CAlignPointsOnSurveyedPath::Align);			CSurvUtilApp::RegisterCmd("AlignPointsOnSurveyedPath");

	acedRegCmds->addCommand("SURVUTIL_GEN", "DrawProfileFrOffZDatFile",		"DrawProfileFrOffZDatFile",		ACRX_CMD_MODAL, DrawProfileFrOffZDatFile);					CSurvUtilApp::RegisterCmd("DrawProfileFrOffZDatFile");
	acedRegCmds->addCommand("SURVUTIL_GEN", "DrawProfileFrOffZDatFileEx",	"DrawProfileFrOffZDatFileEx",	ACRX_CMD_MODAL, DrawProfileFrOffZDatFileEx);				CSurvUtilApp::RegisterCmd("DrawProfileFrOffZDatFileEx");
	acedRegCmds->addCommand("SURVUTIL_GEN", "QProfileLinesOrg",				"QProfileLinesOrg",				ACRX_CMD_MODAL, CCommands::QProfileLinesOrg);				CSurvUtilApp::RegisterCmd("QProfileLinesOrg");
	acedRegCmds->addCommand("SURVUTIL_GEN", "QProfileLines",				"QProfileLines",				ACRX_CMD_MODAL, CCommands::QProfileLines);					CSurvUtilApp::RegisterCmd("QProfileLines");
	acedRegCmds->addCommand("SURVUTIL_GEN", "QProfilePtsOrg",				"QProfilePtsOrg",				ACRX_CMD_MODAL, CCommands::QProfilePtsOrg);					CSurvUtilApp::RegisterCmd("QProfilePtsOrg");
	acedRegCmds->addCommand("SURVUTIL_GEN", "QProfilePts",					"QProfilePts",					ACRX_CMD_MODAL, CCommands::QProfilePts);					CSurvUtilApp::RegisterCmd("QProfilePts");
	acedRegCmds->addCommand("SURVUTIL_GEN", "QProfilePickNAnnotate",		"QProfilePickNAnnotate",		ACRX_CMD_MODAL, CCommands::QProfilePickNAnnotate);			CSurvUtilApp::RegisterCmd("QProfilePickNAnnotate");

	acedRegCmds->addCommand("SURVUTIL_GEN", "DrawXSecFrDwg",				"DrawXSecFrDwg",				ACRX_CMD_MODAL, CCommands::DrawXSecFrDwg);					CSurvUtilApp::RegisterCmd("DrawXSecFrDwg");
	acedRegCmds->addCommand("SURVUTIL_GEN", "DrawXSecFrDwgEx",				"DrawXSecFrDwgEx",				ACRX_CMD_MODAL, CCommands::DrawXSecFrDwgEx);				CSurvUtilApp::RegisterCmd("DrawXSecFrDwgEx");
	acedRegCmds->addCommand("SURVUTIL_GEN", "XSecFrCSD",					"XSecFrCSD",					ACRX_CMD_MODAL, XSecFrCSD);									CSurvUtilApp::RegisterCmd("XSecFrCSD");
	acedRegCmds->addCommand("SURVUTIL_GEN", "XSecFrCSDEx",					"XSecFrCSDEx",					ACRX_CMD_MODAL, XSecFrCSDEx);								CSurvUtilApp::RegisterCmd("XSecFrCSDEx");
	acedRegCmds->addCommand("SURVUTIL_GEN", "L2P",							"L2P",							ACRX_CMD_MODAL, Lines2PLine);								CSurvUtilApp::RegisterCmd("L2P");
	acedRegCmds->addCommand("SURVUTIL_GEN", "L2PEx",						"L2PEx",						ACRX_CMD_MODAL, L2PLayerSpecific);							CSurvUtilApp::RegisterCmd("L2PEx");
	acedRegCmds->addCommand("SURVUTIL_GEN",	"AnnotateChg",					"AnnotateChg",					ACRX_CMD_MODAL, CSurvUtilApp::AnnotateChg);					CSurvUtilApp::RegisterCmd("AnnotateChg");
	
	//'Road Plan' Using STN/PID files ************************************************************
	acedRegCmds->addCommand("SURVUTIL_GEN", "DrawRdPlanOffZFromTxtFile",		"DrawRdPlanOffZFromTxtFile",	ACRX_CMD_MODAL, DrawRdPlanOffZFromTxtFile);					CSurvUtilApp::RegisterCmd("DrawRdPlanOffZFromTxtFile");
	acedRegCmds->addCommand("SURVUTIL_GEN", "ModifyRdPlanOffZTxtFile",			"ModifyRdPlanOffZTxtFile",		ACRX_CMD_MODAL, ModifyRdPlanOffZTxtFile);					CSurvUtilApp::RegisterCmd("ModifyRdPlanOffZTxtFile");
	acedRegCmds->addCommand("SURVUTIL_GEN", "DrawRdPlanNEZFromTxtFile",			"DrawRdPlanNEZFromTxtFile",		ACRX_CMD_MODAL, DrawRdPlanNEZFromTxtFile);					CSurvUtilApp::RegisterCmd("DrawRdPlanNEZFromTxtFile");
	acedRegCmds->addCommand("SURVUTIL_GEN", "ModifyRdPlanNEZTxtFile",			"ModifyRdPlanNEZTxtFile",		ACRX_CMD_MODAL, ModifyRdPlanNEZTxtFile);					CSurvUtilApp::RegisterCmd("ModifyRdPlanNEZTxtFile");

	acedRegCmds->addCommand("SURVUTIL_GEN", "DrawRdPlanOffZFromTxtFileEx",		"DrawRdPlanOffZFromTxtFileEx",	ACRX_CMD_MODAL, DrawRdPlanOffZFromTxtFileEx);				CSurvUtilApp::RegisterCmd("DrawRdPlanOffZFromTxtFileEx");

	acedRegCmds->addCommand("SURVUTIL_GEN", "ModifyPflAndCSecnAnn",	"ModifyPflAndCSecnAnn",	ACRX_CMD_MODAL, ModifyPflAndCSecnAnn);											CSurvUtilApp::RegisterCmd("ModifyPflAndCSecnAnn");

	acedRegCmds->addCommand("SURVUTIL_GEN", "PurgeAll",				"PurgeAll",				ACRX_CMD_MODAL, PurgeAll);														CSurvUtilApp::RegisterCmd("PurgeAll");
	//'Bowditch Rule' Traverse Adjustment ************************************************************
	acedRegCmds->addCommand("SURVUTIL_GEN", "AdjTraverse",			"AdjTraverse",			ACRX_CMD_MODAL, AdjTravUsingBowditchRule);							CSurvUtilApp::RegisterCmd("AdjTraverse");
	acedRegCmds->addCommand("SURVUTIL_GEN", "AdjOpenTraverse",		"AdjOpenTraverse",		ACRX_CMD_MODAL, CSurvUtilApp::AdjOpenTraverse);						CSurvUtilApp::RegisterCmd("AdjOpenTraverse");
	
	acedRegCmds->addCommand("SURVUTIL_GEN", "RevertPLine",			"RevertPLine",			ACRX_CMD_MODAL, CRdXSecFrDWG::RevertPLine);							CSurvUtilApp::RegisterCmd("RevertPLine");

	CSurvUtilApp::RegisterCmdGroup("SURVUTIL_RD_PROFILE");
	//'Road Profile' XLS file related ************************************************************
	acedRegCmds->addCommand("SURVUTIL_RD_PROFILE", "OpenRoadProfileXLS",	"OpenRoadProfileXLS",	ACRX_CMD_MODAL, CSurvUtilApp::OpenRoadProfileXLS);			CSurvUtilApp::RegisterCmd("OpenRoadProfileXLS");
	acedRegCmds->addCommand("SURVUTIL_RD_PROFILE", "DrawRoadProfileXLS",	"DrawRoadProfileXLS",	ACRX_CMD_MODAL, CSurvUtilApp::DrawRoadProfileXLS);			CSurvUtilApp::RegisterCmd("DrawRoadProfileXLS");

	CSurvUtilApp::RegisterCmdGroup("SURVUTIL_FIELD_BOOK");
	//'Field Book' XLS file related ************************************************************
	acedRegCmds->addCommand("SURVUTIL_FIELD_BOOK", "OpenFieldBookXLS",	"OpenFieldBookXLS",	ACRX_CMD_MODAL, CSurvUtilApp::OpenFieldBookXLS);					CSurvUtilApp::RegisterCmd("OpenFieldBookXLS");
	acedRegCmds->addCommand("SURVUTIL_FIELD_BOOK", "DrawFieldBookXLS",	"DrawFieldBookXLS",	ACRX_CMD_MODAL, CSurvUtilApp::DrawFieldBookXLS);					CSurvUtilApp::RegisterCmd("DrawFieldBookXLS");

	CSurvUtilApp::RegisterCmdGroup("SURVUTIL_SYSTEM");
	acedRegCmds->addCommand("SURVUTIL_SYSTEM",		"AB",					"AB",					ACRX_CMD_MODAL, PflAndCSecnXDQuery);						CSurvUtilApp::RegisterCmd("AB");

	// TODO: init your application
    acrxBuildClassHierarchy();
	//
	return (0L != CSurvUtilApp::Initialize(_hdllInstance));
}

static void unloadApp(void)
{
	CSurvUtilApp::UnInitialize();
	EndMFC();
}
// MFC Initialization
void InitMFC()
{
	InitModule(_hdllInstance, DLL_PROCESS_ATTACH, NULL);
	AfxOleInit();
}
void EndMFC()
{
	ASSERT(theApp.pTempResOverride != 0L);

	InitModule(_hdllInstance, DLL_PROCESS_DETACH, NULL);
}


extern BOOL InitModule(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		if(!AfxInitExtensionModule(arxmfcDLL, hInstance))
			return 0;

		CTemporaryResourceOverride::SetDefaultResource(_hdllInstance);
		// TODO: add other initialization tasks here

		new CDynLinkLibrary(arxmfcDLL);
		// end of initialization
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		AfxTermExtensionModule(arxmfcDLL);
		// TODO: perform other cleanup tasks here

		// end of cleanup
	}
	return 1;
}

