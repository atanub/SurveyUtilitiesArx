// File Name: Commands.Cpp
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Commands.h"
#include "SurvUtilApp.h"
#include "GeneralUtilFuncs.h"
#include "InsBlksUtil.h"
#include "RdXSecFrDWG.h"
#include "ExportPoints.h"


extern "C" HWND adsw_acadMainWnd();

/////////////////////////////////////////////////////////////////////////////
// CCommands

CCommands::CCommands()
{
}
CCommands::~CCommands()
{
}

void CCommands::InsBlocks()
{
	CStringArray BlkNameArray;
	CString strLyr, strBlk;
	double dX, dY, dZ, dRotAngle;
	BOOL bConsiderZ;

	GetBlockNames(BlkNameArray);
	if(BlkNameArray.GetSize() <= 0)
	{
		AfxMessageBox("No block definition found in current drawing\nCreate block first & then try again", MB_ICONSTOP);
		return;
	}

	if(!CInsBlksUtil::GetInsBlksParams(BlkNameArray, strLyr, strBlk, dX, dY, dZ, dRotAngle, bConsiderZ))
		return;

	CGeneralUtilFuncs::InsBlocks(strLyr, strBlk, dX, dY, dZ, dRotAngle, bConsiderZ);
}
void CCommands::JoinPoints()
{
	CString strLCurLyr;

	CLAYER(strLCurLyr);
	if(!CGeneralUtilFuncs::JoinPoints(strLCurLyr))
	{

	}
}
void CCommands::InsBlocksFromDatFile()
{
	CGeneralUtilFuncs::InsBlocksFromDatFile();
}
void CCommands::ModifyZValueOfSelEnts()
{
	CGeneralUtilFuncs::ModifyZVal();
}
void CCommands::ModifyTxtSizeOfSelEnts()
{
	CGeneralUtilFuncs::ModifyTxtSize();
}
void CCommands::SetZValueOfSelEnts()
{
	CGeneralUtilFuncs::SetZValueOfSelEnts();
}
void CCommands::ExportPoints()
{
	CExportPointsFactory::Export(CExportPointsFactory::ExportType::General);
}
void CCommands::ExportPointsEx()
{
	CExportPointsFactory::Export(CExportPointsFactory::ExportType::MXRoads);
}
void CCommands::QProfileLines()
{
	RDPFLNEZDATA Rec;
	
	Rec = *(CSurvUtilApp::GetDataCache()->GetRdPflNEZDataRec());
	if(Rec.dOrdInterval <= 0.0)
		Rec.dOrdInterval = 5.0;//-Ve means the feature "Equal Interval Ordinate" is not public

	if(!CRdPflNEZDataDlg::GetRdPflNEZData(Rec))
		return;

	CSurvUtilApp::GetDataCache()->SetRdPflNEZDataRec(&Rec);
	if(RTNORM != ads_getpoint(0L, "\nPick lower left corner of profile :", asDblArray(Rec.PtOrg)))
		return;

	CGeneralUtilFuncs::QProfileLines(Rec);
}
void CCommands::QProfileLinesOrg()
{
	RDPFLNEZDATA Rec;
	
	Rec = *(CSurvUtilApp::GetDataCache()->GetRdPflNEZDataRec());
	Rec.dOrdInterval = -1.0;//-Ve means the feature "Equal Interval Ordinate" is not public
	strcpy(Rec.szOutFile ,"");

	if(!CRdPflNEZDataDlg::GetRdPflNEZData(Rec))
		return;
	CSurvUtilApp::GetDataCache()->SetRdPflNEZDataRec(&Rec);

	if(RTNORM != ads_getpoint(0L, "\nPick lower left corner of profile :", asDblArray(Rec.PtOrg)))
		return;

	CGeneralUtilFuncs::QProfileLines(Rec);
}
void CCommands::QProfilePts()
{
	RDPFLNEZDATA Rec;
	
	Rec = *(CSurvUtilApp::GetDataCache()->GetRdPflNEZDataRec());
	if(Rec.dOrdInterval <= 0.0)
		Rec.dOrdInterval = 5.0;//-Ve means the feature "Equal Interval Ordinate" is not public

	if(!CRdPflNEZDataDlg::GetRdPflNEZData(Rec))
		return;
	CSurvUtilApp::GetDataCache()->SetRdPflNEZDataRec(&Rec);
	if(RTNORM != ads_getpoint(0L, "\nPick lower left corner of profile :", asDblArray(Rec.PtOrg)))
		return;

	CGeneralUtilFuncs::QProfilePts(Rec);
}
void CCommands::QProfilePtsOrg()
{
	RDPFLNEZDATA Rec;
	
	Rec = *(CSurvUtilApp::GetDataCache()->GetRdPflNEZDataRec());
	Rec.dOrdInterval = -1.0;//-Ve means the feature "Equal Interval Ordinate" is not public
	strcpy(Rec.szOutFile ,"");

	if(!CRdPflNEZDataDlg::GetRdPflNEZData(Rec))
		return;
	CSurvUtilApp::GetDataCache()->SetRdPflNEZDataRec(&Rec);
	if(RTNORM != ads_getpoint(0L, "\nPick lower left corner of profile :", asDblArray(Rec.PtOrg)))
		return;

	CGeneralUtilFuncs::QProfilePts(Rec);
}
void CCommands::QProfilePickNAnnotate()
{
	RDPFLNEZDATA Rec;
	
	Rec = *(CSurvUtilApp::GetDataCache()->GetRdPflNEZDataRec());
	if(!CRdPflNEZDataDlg::GetRdPflNEZData(Rec))
		return;

	CSurvUtilApp::GetDataCache()->SetRdPflNEZDataRec(&Rec);
	CGeneralUtilFuncs::QProfilePickNAnnotate(Rec);
}


void CCommands::DrawXSecFrDwg()
{
	RDXSECOFFELEVDATA Rec;

	Rec = *(CSurvUtilApp::GetDataCache()->GetRdXSecOffElevDataParam());
	Rec.fOrdInterval = -1.0;
	if(!CRdXSecFrDWGDlg::GetRoadXSecFrDwgData(Rec))
		return;
	
	CSurvUtilApp::GetDataCache()->SetRdXSecOffElevDataParam(&Rec);
	CRdXSecFrDWG::RdXSecFrDWG(Rec);
}
void CCommands::DrawXSecFrDwgEx()
{
	RDXSECOFFELEVDATA Rec;

	Rec = *(CSurvUtilApp::GetDataCache()->GetRdXSecOffElevDataParam());
	if(Rec.fOrdInterval <= 0.0)
	{
		Rec.fOrdInterval = 5.0;
	}
	if(!CRdXSecFrDWGDlg::GetRoadXSecFrDwgData(Rec))
		return;
	
	CSurvUtilApp::GetDataCache()->SetRdXSecOffElevDataParam(&Rec);
	CRdXSecFrDWG::RdXSecFrDWG(Rec);
}

