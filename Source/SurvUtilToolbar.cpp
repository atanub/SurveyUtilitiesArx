// File Name: SurvUtilToolbar.cpp

#include "stdafx.h"
#include "SurvUtilToolbar.h"
#include "SurvUtilApp.h"
#include "ResourceHelper.h"
#include "AboutDlg.h"


static BOOL ToolBarProc(WORD, CWnd*);

/////////////////////////////////////////////////////////////////////////////
// CSurvUtilToolbar

CSurvUtilToolbar::CSurvUtilToolbar()
{
}

CSurvUtilToolbar::~CSurvUtilToolbar()
{
}
BEGIN_MESSAGE_MAP(CSurvUtilToolbar, CToolBar)
	//{{AFX_MSG_MAP(CSurvUtilToolbar)
	ON_UPDATE_COMMAND_UI(ID_ABOUT,						OnUpdAlwaysEnabled)

	ON_UPDATE_COMMAND_UI(ID_BUTTON_RD_SURVEY,			OnUpdAlwaysEnabled)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_RD_PROFILE_MENU,		OnUpdAlwaysEnabled)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_FIELDBOOK_MENU,		OnUpdAlwaysEnabled)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_UTIL,				OnUpdAlwaysEnabled)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#ifdef _DEBUG
void CSurvUtilToolbar::AssertValid() const
{
}
void CSurvUtilToolbar::Dump(CDumpContext& /*dc*/) const
{
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CSurvUtilToolbar message handlers
void CSurvUtilToolbar::OnUpdAlwaysEnabled(CCmdUI* /*pCmdUI*/) 
{
}
// End of CSurvUtilToolbar class implementation
//////////////////////////////////////////////////////////////////

// CTBHandler
/////////////////////////////////////////////////////////////////////////////

CTBHandler::CTBHandler()
{
}
CTBHandler::~CTBHandler()
{
}
BEGIN_MESSAGE_MAP(CTBHandler, CWnd)
	//{{AFX_MSG_MAP(CTBHandler)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTBHandler message handlers
static int GetSubMenuID(WORD iIDButtonPressed)
{
	switch(iIDButtonPressed)
	{
	case ID_BUTTON_RD_SURVEY:
		return 0;
	case ID_BUTTON_RD_PROFILE_MENU:
		return 1;
	case ID_BUTTON_FIELDBOOK_MENU:
		return 2;
	case ID_BUTTON_UTIL:
		return 3;
	default:
		return -1;//Invalid
	}
}

static BOOL PopSubCmdMenu(WORD iIDButtonPressed, CWnd* pWndParam)
{//Show Popup menu
	CMenu MenuParent;
	CMenu* pPopMenu;
	POINT Point;
	int iSubMenuID;
	
	iSubMenuID = GetSubMenuID(iIDButtonPressed);
	if(iSubMenuID < 0)
		return FALSE;

	VERIFY(MenuParent.LoadMenu(IDR_MENU_POPUP));
	
	pPopMenu = MenuParent.GetSubMenu(iSubMenuID);
	if(pPopMenu != NULL);
	{
		::GetCursorPos(&Point);
		pPopMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, Point.x, Point.y, pWndParam);
	}
	return TRUE;
}
static BOOL ToolBarProc(WORD iIDButton, CWnd* pWndParam)
{
	CWinApp* pWinApp;
	CString cmdStr;

	pWinApp = acedGetAcadWinApp();

	if(PopSubCmdMenu(iIDButton, pWndParam))
	{
		return TRUE;//Handled
	}
	if(!CSurvUtilApp::HasValidLicense())
	{
		if(iIDButton == ID_ABOUT)
		{
			cmdStr = "AboutSurvUtil ";
		}
		else
		{
			AfxMessageBox("Survey Utility ARX Application not installed properly\nThis software may cause unexpected results\nPlease contact 'atanubanik@gmail.com' for registered copy", MB_ICONSTOP);
			ads_printf("\nSurvey Utility ARX Application not installed properly\nThis software may cause unexpected results\nPlease contact 'atanubanik@gmail.com' for registered copy");
		}
	}
	else
	{//
		switch(iIDButton)
		{
			case ID_ABOUT:
				cmdStr = "AboutSurvUtil ";
				break;
			//For 'Road Survey' (from Master Data) Menu ////////////////////////////////////////////
			case ID_BUTTON_OPENXLS:
				cmdStr = "OpenSurveyBookXLSFile ";
				break;
			case ID_BUTTON_COMPILE_THIS:
				cmdStr = "CompileThis ";
				break;
			case ID_BUTTON_BATCHCOMPILE:
				cmdStr = "BatchCompileXLS ";
				break;
			//
			case ID_BUTTON_ROAD_PLAN:
				cmdStr = "DrawRoadPlan ";
				break;
			case ID_BUTTON_CSEXN:
				cmdStr = "DrawXLSXSecn ";
				break;
			case ID_BUTTON_PROFILE:
				cmdStr = "DrawXLSProfile ";
				break;

			case ID_BUTTON_SEL_N_JOIN_PTS:
			case ID_JOIN_SURVEYED_POINTS:
				cmdStr = "JoinSelectedPts ";
				break;
			case ID_BUTTON_DRAW_SEL_PTS:
			case ID_SURVEY_DRAW_MDF_POINTS:
				cmdStr = "DrawFromPIN ";
				break;
			//For 'Road Profile' Menu ////////////////////////////////////////////
			case ID_ROAD_PROFILE:
				cmdStr = "OpenRoadProfileXLS ";
				break;
			case ID_ROAD_PROFILE_DRAW:
				cmdStr = "DrawRoadProfileXLS ";
				break;
			//For 'Field Book' Menu ////////////////////////////////////////////
			case ID_PROCCESS_FIELDBOOK:
				cmdStr = "OpenFieldBookXLS ";
				break;
			case ID_DRAW_FIELDBOOK_XLS:
				cmdStr = "DrawFieldBookXLS ";
				break;
			//Generic ////////////////////////////////////////////
			case ID_BUTTON_EDIT_PFL_AND_CS_ANN:
				cmdStr = "ModifyPflAndCSecnAnn ";
				break;
			//For Utility Menu ////////////////////////////////////////////
			case ID_BUTTON_CONVERT_FIELD_DATA:
			case ID_SURVEY_CONVERT_RAWSDATA:
				cmdStr = "ConvertFieldData ";
				break;
			case ID_BUTTON_JOIN_POINTS:
			case ID_SURVEY_SEL_AND_JPOINTS:
				cmdStr = "JoinPoints ";
				break;
			case ID_BUTTON_EXPORT_POINTS:
			case ID_SURVEY_XPORT_SPTS:
				cmdStr = "ExportPoints ";
				break;
			case ID_SURVEY_XPORT_SPTS_EX:
				cmdStr = "ExportPointsEx ";
				break;
			case ID_ADJUST_TRAVERSE:
				cmdStr = "AdjTraverse ";
				break;
			case ID_ADJUST_TRAVERSE_OPEN:
				cmdStr = "AdjOpenTraverse ";
				break;
			case ID_BUTTON_PFL_OFFZ_FROM_DATAFILE:
				cmdStr = "DrawProfileFrOffZDatFile ";
				break;
			case ID_BUTTON_PFL_OFFZ_FROM_DATAFILE_EX:
				cmdStr = "DrawProfileFrOffZDatFileEx ";
				break;
			case ID_BUTTON_QUICK_PFL_PTS:
				cmdStr = "QProfilePts ";
				break;
			case ID_QPROFILE_PICKNANNOTATE:
				cmdStr = "QProfilePickNAnnotate ";
				break;
			case ID_BUTTON_QUICK_PFL_PTS_X:
				cmdStr = "QProfilePtsOrg ";
				break;
			case ID_BUTTON_QUICK_PFL_LINES:
				cmdStr = "QProfileLines ";
				break;
			case ID_BUTTON_QUICK_PFL_LINES_X:
				cmdStr = "QProfileLinesOrg ";
				break;
			case ID_BUTTON_QUICK_XSEC:
				cmdStr = "DrawXSecFrDwg ";
				break;
			case ID_BUTTON_QUICK_XSEC_EX:
				cmdStr = "DrawXSecFrDwgEx ";
				break;
			case ID_BUTTON_XSEC_FROM_CSD:
				cmdStr = "XSecFrCSD ";//Doesn't considers Ordinate Interval
				break;
			case ID_BUTTON_XSEC_FROM_CSDEX:
				cmdStr = "XSecFrCSDEx ";//CONSIDERS Ordinate Interval
				break;
			case ID_BUTTON_INS_BLKS:
				cmdStr = "InsBlocks ";
				break;
			case ID_BUTTON_INS_BLKS_FROM_FILE:
				cmdStr = "InsBlocksFromDatFile ";
				break;
			case ID_BUTTON_LINE_2_PLINE:
				cmdStr = "L2P ";
				break;
			case ID_BUTTON_LINE_2_PLINE_EX:
				cmdStr = "L2PEx ";
				break;
			case IDC_RD_PLAN_NEZ_TXTFILE:
				cmdStr = "DrawRdPlanNEZFromTxtFile ";
				break;
			case IDC_RD_PLAN_NEZ_TXTFILE_MODIFY:
				cmdStr = "ModifyRdPlanNEZTxtFile ";
				break;
			case IDC_RD_PLAN_OFFZ_TXTFILE:
				cmdStr = "DrawRdPlanOffZFromTxtFile ";
				break;
			case IDC_RD_PLAN_OFFZ_TXTFILE_MODIFY:
				cmdStr = "ModifyRdPlanOffZTxtFile ";
				break;
			case IDC_RD_PLAN_OFFZ_TXTFILE_EX:
				cmdStr = "DrawRdPlanOffZFromTxtFileEx ";
				break;
			case ID_UTILITY_MODIFY_ZVALUE:
				cmdStr = "ModifyZValueOfSelEnts ";
				break;
			case ID_UTILITY_SETZ_VALUE:
				cmdStr = "SetZValueOfSelEnts ";
				break;
			case ID_UTILITY_MODIFYTEXTSIZE:
				cmdStr = "ModifyTxtSizeOfSelEnts ";
				break;
			case ID_UTILITY_ALIGN_POINTS_ON_SURVEYED_PATH:
				cmdStr = "AlignPointsOnSurveyedPath ";
				break;
			//For ?? Menu ////////////////////////////////////////////
			default:
				return FALSE;//NOT Handled
		}
	}
	if(strlen(cmdStr) > 0)
		CSurvUtilApp::SendCmdToAutoCAD(cmdStr);

	// Force AutoCAD Toolbar to be updated.
	// It's updated only when AutoCAD is on idle.
	pWinApp->OnIdle(0);
	return TRUE;//Handled
}

BOOL CTBHandler::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	UINT iCtrlID, iMsgID;
	
	iCtrlID = LOWORD(wParam);
	iMsgID = HIWORD(wParam);

    if(ToolBarProc((unsigned short)iCtrlID, this))
    {
        return TRUE; // end of msg
    }
	return CWnd::OnCommand(wParam, lParam);
}
