// TraverseAdj.cpp : implementation file
//
#include "stdafx.h"
#include "SurvUtilApp.h"
#include "TraverseAdj.h"
#include "TraverseCorrectionDlg.h"
#include "FBSheetData.h"
#include "ResultDlg.h"
#include "DrawTable.h"

#include "Util.h"
#include "DrawUtil.h"
#include "ParameterDataCache.h"

static BOOL ReadInpFile(const CString&, CTravVertexArray&);
static void FindClosingError(const CTravVertexArray&, double&, double&, double&);

static double CalcErrorAtStn(const CTravVertexArray&, const int&, const double&, const double&);
static BOOL CalcVertexes(CTravVertexArray&, const TRAVDWGPARAM&);
static BOOL AdjustTraverse(const CString&, CStdioFile&, const TRAVDWGPARAM&);
static void AdjustError(const CTravVertexArray&, const double&, const double&, const double&);
static void CalculateCorrIncAngle(const CTravVertexArray&, const int&, double&);

static void DrawTraverse(const CTravVertexArray&, const TRAVDWGPARAM&);

static void BreakAngle(const double&, double&, double&, double&, BOOL&);

static void AnnotateStn(const AcGePoint3d&, const double&, const char*, const char*, const BOOL&);

//Input File Format
//[Stn. A], [Inc. Angle D-A-B],		[Length of line joining D-A]
//[Stn. B], [Inc. Angle A-B-C],		[Length of line joining A-B], [Bearing of line joining A-B]
//[Stn. C], [Inc. Angle B-C-D],		[Length of line joining B-C]
//[Stn. D], [Inc. Angle C-D-A],		[Length of line joining C-D]

void AdjTravUsingBowditchRule()
{
	CStdioFile fileOut;
	TRAVDWGPARAM recParam;

	recParam = *(CSurvUtilApp::GetDataCache()->GetTravDwgParam());
	if(!GetTraverseParam(recParam, FALSE))
		return;
	CSurvUtilApp::GetDataCache()->SetTravDwgParam(&recParam);

	//HardCoded Values
	strcpy(recParam.szLyrCalcTable,		"TRAV_CALC_TABLE");
	strcpy(recParam.szLyrTraverseOrg,	"TRAV_DWG_FIELD");
	strcpy(recParam.szLyrTraverseAdj,	"TRAV_DWG_ADJUSTED");

	//Open Out file
	if(!fileOut.Open(recParam.szOutFile, CFile::modeCreate|CFile::modeWrite|CFile::typeText))
	{
		ads_printf("\nERROR: Unable to open output file %s", (LPCSTR)recParam.szOutFile);
		return;
	}
	if(RTNORM != ads_getpoint(0L, "\nPick base point of traverse :", asDblArray(recParam.ptTravBase)))
	{
		ads_printf("\nERROR: Invalid base point picked");
		return;
	}

	///Process
	if(!AdjustTraverse(recParam.szInFile, fileOut, recParam))
	{//Error
	}
	fileOut.Close();
	ShowResultDlg(recParam.szOutFile);
}
static BOOL AdjustTraverse(const CString& strInpFile, CStdioFile& fileOut, const TRAVDWGPARAM& recParam)
{
	CTravVertexArray arrayTravVertex;
	CString strTmp;
	const BOOL bDraw = (recParam.fStnMarkTxtSize > 0.0);

	if(!ReadInpFile(strInpFile, arrayTravVertex))
		return FALSE;

	if(arrayTravVertex.GetSize() < 3)
	{
		ads_printf("\nERROR: Traversed stations doesn't form closed boundary, at least 3 stations needed!");
		DeallocatePtrArray(arrayTravVertex);
		return FALSE;
	}
	//Core Calculations .....
	CalcVertexes(arrayTravVertex, recParam);

	{//Dump Results.....
		CPtrArray arrayOfPointers;
		double fCumIncAngle, fCumIncAngAdj;
		
		fCumIncAngle = 0.0;
		fCumIncAngAdj = 0.0;
		for(int i = 0; i < arrayTravVertex.GetSize(); i++)
		{
			CStringArray* pStrArray;
			CString	strTmp, strTmpA;
			TRAVVERTEX* pRec;

			
			pRec = arrayTravVertex[i];
			if(i == 0)
			{//Captions
				pStrArray = new CStringArray();
				
				strTmp.Format("%s", "Station");				pStrArray->Add(strTmp);
				strTmp.Format("%s", "Bearing");				pStrArray->Add(strTmp);
				strTmp.Format("%s", "Inc. Angle");			pStrArray->Add(strTmp);
				strTmp.Format("%s", "Inc.Angle Error");		pStrArray->Add(strTmp);
				strTmp.Format("%s", "Corrected Inc.Angle");	pStrArray->Add(strTmp);
				strTmp.Format("%s", "Dist from Prev Stn");	pStrArray->Add(strTmp);
				strTmp.Format("%s", "Northing");			pStrArray->Add(strTmp);
				strTmp.Format("%s", "N-Correction");		pStrArray->Add(strTmp);
				strTmp.Format("%s", "Corrected-N");			pStrArray->Add(strTmp);
				strTmp.Format("%s", "Easting");				pStrArray->Add(strTmp);
				strTmp.Format("%s", "E-Correction");		pStrArray->Add(strTmp);
				strTmp.Format("%s", "Corrected-E");			pStrArray->Add(strTmp);
				//
				arrayOfPointers.Add(pStrArray);
			}
			pStrArray = new CStringArray();
			
			{//Station Name...#1
				strTmp = pRec->szStn;
				pStrArray->Add(strTmp);
			}
			{//Bearing...#2
				strTmp = FormatAngle(ConvertAcadAngleToBearing(pRec->fBearing));
#ifdef _DEBUG
				strTmp = FormatAngle(pRec->fBearing);//In AutoCAD system for the sake of debugging ease
#endif//_DEBUG
				pStrArray->Add(strTmp);
			}
			{//Inc Angle...#3
				strTmp = FormatAngle(pRec->fIncAngle);
#ifdef _DEBUG
				strTmpA.Format("[%f]", RTOD(pRec->fIncAngle));
				strTmp += strTmpA;
#endif//_DEBUG
				pStrArray->Add(strTmp);
			}
			{//Correction in Included angle #4
				strTmp = FormatAngle(pRec->fIncAngleAdj);
#ifdef _DEBUG
				strTmpA.Format("[%f]", RTOD(pRec->fIncAngleAdj));
				strTmp += strTmpA;
#endif//_DEBUG
				pStrArray->Add(strTmp);
			}
			{//Corrected Inc.Angle #5
				strTmp = FormatAngle(pRec->fIncAngle + pRec->fIncAngleAdj);
#ifdef _DEBUG
				strTmpA.Format("[%f]", RTOD(pRec->fIncAngle + pRec->fIncAngleAdj));
				strTmp += strTmpA;
#endif//_DEBUG
				pStrArray->Add(strTmp);
			}
			{//Distance to Next Station...#6
				strTmp.Format("%f", pRec->fDistance);
				pStrArray->Add(strTmp);
			}
			//Northing #7
			strTmp.Format("%f", pRec->fNorthing);
			pStrArray->Add(strTmp);
			//Northing Correction #8
			strTmp.Format("%f", pRec->fNorthingAdj);
			pStrArray->Add(strTmp);
			//Corrected-N #9
			strTmp.Format("%f", (pRec->fNorthing + pRec->fNorthingAdj));
			pStrArray->Add(strTmp);
			//Easting #10
			strTmp.Format("%f", pRec->fEasting);
			pStrArray->Add(strTmp);
			//Easting Correction #11
			strTmp.Format("%f", pRec->fEastingAdj);
			pStrArray->Add(strTmp);
			//Corrected-E #12
			strTmp.Format("%f", (pRec->fEasting + pRec->fEastingAdj));
			pStrArray->Add(strTmp);
		
			//
			arrayOfPointers.Add(pStrArray);
			//
			fCumIncAngle += pRec->fIncAngle;
			fCumIncAngAdj += pRec->fIncAngleAdj;
			if(i == arrayTravVertex.GetSize() - 1)
			{//Captions
				pStrArray = new CStringArray();
				
				strTmp.Format("%s", "");						pStrArray->Add(strTmp);
				strTmp.Format("%s", "");						pStrArray->Add(strTmp);
				{//Total Inc Angle
					strTmp = FormatAngle(fCumIncAngle);
#ifdef _DEBUG
					strTmp.Format("%s[%f]", FormatAngle(fCumIncAngle), RTOD(fCumIncAngle));
#endif// _DEBUG
					pStrArray->Add(strTmp);
				}
				{//Total Inc Angle Error
					strTmp = FormatAngle(fCumIncAngAdj);
#ifdef _DEBUG
					strTmp.Format("%s[%f]", FormatAngle(fCumIncAngAdj), RTOD(fCumIncAngAdj));
#endif// _DEBUG
					pStrArray->Add(strTmp);
				}
				strTmp.Format("%s", "");						pStrArray->Add(strTmp);
				strTmp.Format("%s", "");						pStrArray->Add(strTmp);
				strTmp.Format("%s", "");						pStrArray->Add(strTmp);
				strTmp.Format("%s", "");						pStrArray->Add(strTmp);
				strTmp.Format("%s", "");						pStrArray->Add(strTmp);
				strTmp.Format("%s", "");						pStrArray->Add(strTmp);
				strTmp.Format("%s", "");						pStrArray->Add(strTmp);
				strTmp.Format("%s", "");						pStrArray->Add(strTmp);
				//
				arrayOfPointers.Add(pStrArray);
			}
		}
		{//Dump in file
			for(int i = 0; i < arrayOfPointers.GetSize(); i++)
			{
				CStringArray* pStrArray;
				CString	strLine;
				
				pStrArray = (CStringArray*)arrayOfPointers[i];
				strLine = "";
				for(int j = 0; j < pStrArray->GetSize(); j++)
				{
					if(j == 0)
					{
						strLine = pStrArray->GetAt(j);
					}
					else
					{
						strLine += (CString("\t\t\t") + pStrArray->GetAt(j));
					}
				}
				fileOut.WriteString(strLine);
				fileOut.WriteString(CString("\n"));
			}
		}
		if(bDraw)
		{//Draw in dwg file
			if(RTNORM == ads_getpoint(0L, "\nPick upper right corner of Calculation Table :", asDblArray(recParam.ptTableBase)))
			{
				const BOOL bDrawGrid = TRUE;
				const AcGePoint3d ptBase(0,0,0);
				CDrawTable objDrawTable(&arrayOfPointers, recParam.fTableTxtSize, recParam.fTableMargin, bDrawGrid);
				
				LAYER(recParam.szLyrCalcTable, 1);
				objDrawTable.Draw(recParam.ptTableBase, recParam.szLyrCalcTable);
			}
		}
		{//Deallocate Memory
			for(int i = 0; i < arrayOfPointers.GetSize(); i++)
			{
				CStringArray* pStrArray;
				
				pStrArray = (CStringArray*)arrayOfPointers[i];
				delete pStrArray;
			}
			if(arrayOfPointers.GetSize() > 0)
			{
				arrayOfPointers.RemoveAll();
			}
		}
	}
	DeallocatePtrArray(arrayTravVertex);
	return TRUE;
}
//XYZ format
static BOOL CalcVertexes(CTravVertexArray& arrayTravVertex, const TRAVDWGPARAM& recParam)
{
	const BOOL bDraw = (recParam.fStnMarkTxtSize > 0.0);
	TRAVVERTEX* pRecNext;
	TRAVVERTEX* pRec;
	int i;
	double fBakBearing;

	if(arrayTravVertex.GetSize() < 3)
	{
		ads_printf("\nERROR: Failed to calculate vertex coordinates !\nMinimum 3# of stations required for closed traverse");
		return FALSE;
	}
	//NOTE : Bearing in AutoCAD Angle System(Anti Clockwise)
	for(i = 1; i < arrayTravVertex.GetSize(); i++)
	{
		pRec = arrayTravVertex[i];
		if(i == (arrayTravVertex.GetSize() - 1))
			pRecNext = arrayTravVertex[0];
		else
			pRecNext = arrayTravVertex[i + 1];
		
		//Calculate Bearing ...
		fBakBearing = pRec->fBearing + PI;
		if(fBakBearing > (PI * 2.0))
			fBakBearing = fBakBearing - (PI * 2.0);

		pRecNext->fBearing = fBakBearing - pRec->fIncAngle;
		if(pRecNext->fBearing < 0.0)
			pRecNext->fBearing = (2 * PI) + pRecNext->fBearing;
#ifdef _DEBUG
		fBakBearing = RTOD(pRec->fBearing);
		ads_printf("\nRec#%d[%s]->>[%f]", i, pRec->szStn, fBakBearing);
		if(fBakBearing > 360.0)
		{
			ads_printf("\nERROR: Control should not reach here\nFatal error while calculating Bearing");
			return FALSE;
		}
#endif//_DEBUG
		//for Extra checking 
		fBakBearing = RTOD(pRec->fBearing);
		if(fBakBearing > 360.0)
		{
			AfxMessageBox("ERROR: Control should not reach here\nFatal error while calculating Bearing", MB_ICONSTOP);
			return FALSE;
		}
	}
	{//Calculate Co-Ords
		const AcGePoint3d ptBase(recParam.ptTravBase);
		AcGePoint3d ptCur, ptPrev;

		for(i = 0; i < arrayTravVertex.GetSize(); i++)
		{
			pRec = arrayTravVertex[i];

			if(i == 0)
			{
				ptCur = ptBase;
				pRec->fNorthing = ptBase.y;
				pRec->fEasting = ptBase.x;
			}
			else
			{
				ads_polar(asDblArray(ptPrev), pRec->fBearing, pRec->fDistance, asDblArray(ptCur));
				{//for all other
					pRec->fNorthing = ptCur.y;
					pRec->fEasting = ptCur.x;
				}
			}
			ptPrev = ptCur;
		}
	}
	{//
		double fAngleErr, fDistanceErr, fErrorAdjAngle;
		
		FindClosingError(arrayTravVertex, fAngleErr, fDistanceErr, fErrorAdjAngle);
		ads_printf("\nTRACE: Closing Error : [Angle = %f][Distance = %f][Adjustment Angle = %f]", RTOD(fAngleErr), fDistanceErr, RTOD(fErrorAdjAngle));
		//Adjust Error.....
		AdjustError(arrayTravVertex, fAngleErr, fDistanceErr, fErrorAdjAngle);
	}

	if(bDraw)
	{//Draw Traverse if required
		DrawTraverse(arrayTravVertex, recParam);
	}	
	return TRUE;
}

static void DrawTraverse(const CTravVertexArray& arrayTravVertex, const TRAVDWGPARAM& recParam)
{
	AcGePoint3d ptCur;
	TRAVVERTEX* pRec;
	int i;

	LAYER(recParam.szLyrTraverseOrg, 1);
	LAYER(recParam.szLyrTraverseAdj, 3);
	{//
		AcGePoint3dArray arrayVertexes;
		for(i = 0; i < arrayTravVertex.GetSize(); i++)
		{
			pRec = arrayTravVertex[i];
			ptCur.x = pRec->fEasting;
			ptCur.y = pRec->fNorthing;
			ptCur.z = 0.0;

			arrayVertexes.append(ptCur);
			AnnotateStn(ptCur, recParam.fStnMarkTxtSize, (arrayTravVertex[i])->szStn, recParam.szLyrTraverseOrg, TRUE);
			if(i == arrayTravVertex.GetSize() - 1)
			{//for the Starting Vertex (Error)
				ads_polar(asDblArray(ptCur), (arrayTravVertex[0])->fBearing, (arrayTravVertex[0])->fDistance, asDblArray(ptCur));
				AnnotateStn(ptCur, recParam.fStnMarkTxtSize, (arrayTravVertex[0])->szStn, recParam.szLyrTraverseOrg, TRUE);
				arrayVertexes.append(ptCur);
			}
		}
		POLYLINE(arrayVertexes, recParam.szLyrTraverseOrg, FALSE);
	}
	{////Draw Corrected traverse
		AcGePoint3dArray arrayVertexes;
	
		for(i = 0; i < arrayTravVertex.GetSize(); i++)
		{
			pRec = arrayTravVertex[i];
			ptCur.x = pRec->fEasting	+ pRec->fEastingAdj;
			ptCur.y = pRec->fNorthing	+ pRec->fNorthingAdj;
			ptCur.z = 0.0;

			arrayVertexes.append(ptCur);
			AnnotateStn(ptCur, recParam.fStnMarkTxtSize, (arrayTravVertex[i])->szStn, recParam.szLyrTraverseAdj, FALSE);

			if(i == arrayTravVertex.GetSize() - 1)
			{//Make it closed
				pRec = arrayTravVertex[0];
				ptCur.x = pRec->fEasting	+ pRec->fEastingAdj;
				ptCur.y = pRec->fNorthing	+ pRec->fNorthingAdj;
				ptCur.z = 0.0;
				arrayVertexes.append(ptCur);
			}
		}
		POLYLINE(arrayVertexes, recParam.szLyrTraverseAdj, FALSE);
	}
}
static void AnnotateStn(const AcGePoint3d& ptParam, const double& fTxtSize, const char* pszTxt, const char* pszLyr, const BOOL& /*bIsFieldData*/)
{
	//if(!bIsFieldData)
	//{
	//	CIRCLE(ptParam, (fTxtSize * 0.7), pszLyr);
	//}
	//CIRCLE(ptParam, (fTxtSize * 0.6), pszLyr);
	TEXTMID(ptParam, pszTxt, 0.0, fTxtSize, pszLyr);
}

static double CalcErrorAtStn(const CTravVertexArray& arrayTravVertex, const int& iStnID, const double& fTotalLength, const double& fTotalErr)
{//NOTE: iStnID = '0' based id
	double fValToRet, fAngleTmp;
	int i;
	TRAVVERTEX* pRec;
	
	if(arrayTravVertex.GetSize() <= iStnID)
	{
		return -1.0;//ERROR
	}
	if(0 == iStnID)
		return 0.0;
	
	for(i = 1, fValToRet = 0.0; i <= iStnID; i++)
	{
		pRec = arrayTravVertex[i];
		fValToRet += pRec->fDistance;
	}
	fAngleTmp = atan(fTotalErr / fTotalLength);
	fValToRet = fValToRet * tan(fAngleTmp);
	return fValToRet;
}
static void AdjustError(const CTravVertexArray& arrayTravVertex, const double& /*fAngle*/, const double& fTotalError, const double& fErrorAdjAngle)
{
	double fTotalLength, fErrorInStn;
	int i;
	TRAVVERTEX* pRec;
	
	{//Calculate Total Length
		for(i = 0, fTotalLength = 0.0; i < arrayTravVertex.GetSize(); i++)
		{
			pRec = arrayTravVertex[i];
			fTotalLength += pRec->fDistance;
		}
	}

	//Start Adjusting...Adjust Northing/Easting
	for(i = 0; i < arrayTravVertex.GetSize(); i++)
	{
		pRec = arrayTravVertex[i];
		fErrorInStn = CalcErrorAtStn(arrayTravVertex, i, fTotalLength, fTotalError);
		pRec->fNorthingAdj = fErrorInStn * sin(fErrorAdjAngle);
		pRec->fEastingAdj = fErrorInStn * cos(fErrorAdjAngle);
	}
	//..Adjust Included angle
	for(i = 0; i < arrayTravVertex.GetSize(); i++)
	{
		double fAngleNew;
		
		CalculateCorrIncAngle(arrayTravVertex, i, fAngleNew);
		
		pRec = arrayTravVertex[i];
		pRec->fIncAngleAdj = fAngleNew - pRec->fIncAngle;
	}
}
static void CalculateCorrIncAngle(const CTravVertexArray& arrayTravVertex, const int& iStn, double& fAngle)
{
	AcGePoint3d ptVertex, ptLeft, ptRite;
	TRAVVERTEX* pRec;

	fAngle = 0.0;
	if((iStn >= arrayTravVertex.GetSize()) || (iStn < 0) || (arrayTravVertex.GetSize() <= 2))
	{
		ads_printf("\nERROR: Control should not reach here!!!");
		return;//ERROR
	}
	pRec = arrayTravVertex[iStn];
	ptVertex.y = pRec->fNorthing + pRec->fNorthingAdj;
	ptVertex.x = pRec->fEasting + pRec->fEastingAdj;
	ptVertex.z = 0.0;
	//Special Case
	if(iStn == 0)
	{//First Vertex
		pRec = arrayTravVertex[arrayTravVertex.GetSize() - 1];
		ptLeft.y = pRec->fNorthing + pRec->fNorthingAdj;
		ptLeft.x = pRec->fEasting + pRec->fEastingAdj;
		ptLeft.z = 0.0;
		pRec = arrayTravVertex[iStn + 1];
		ptRite.y = pRec->fNorthing + pRec->fNorthingAdj;
		ptRite.x = pRec->fEasting + pRec->fEastingAdj;
		ptRite.z = 0.0;
	}
	else if(iStn == (arrayTravVertex.GetSize() - 1))
	{//Last Vertex
		pRec = arrayTravVertex[iStn - 1];
		ptLeft.y = pRec->fNorthing + pRec->fNorthingAdj;
		ptLeft.x = pRec->fEasting + pRec->fEastingAdj;
		ptLeft.z = 0.0;
		pRec = arrayTravVertex[0];
		ptRite.y = pRec->fNorthing + pRec->fNorthingAdj;
		ptRite.x = pRec->fEasting + pRec->fEastingAdj;
		ptRite.z = 0.0;
	}
	else
	{//Intermediate
		pRec = arrayTravVertex[iStn - 1];
		ptLeft.y = pRec->fNorthing + pRec->fNorthingAdj;
		ptLeft.x = pRec->fEasting + pRec->fEastingAdj;
		ptLeft.z = 0.0;
		pRec = arrayTravVertex[iStn + 1];
		ptRite.y = pRec->fNorthing + pRec->fNorthingAdj;
		ptRite.x = pRec->fEasting + pRec->fEastingAdj;
		ptRite.z = 0.0;
	}
	double fAngleLeft, fAngleRite;
	fAngleLeft = Angle2D(ptVertex, ptLeft);
	fAngleRite = Angle2D(ptVertex, ptRite);
	if(fAngleRite > fAngleLeft)
		fAngle = (PI * 2.0 ) - (fAngleRite - fAngleLeft);
	else
		fAngle = fAngleLeft - fAngleRite;
}
static void FindClosingError(const CTravVertexArray& arrayTravVertex, double& fAngle, double& fDistance, double& fErrorAdjAngle)
{
	const double fTotalAng = (2 * (arrayTravVertex.GetSize() - 2)) * (PI / 2.0);
	int i;
	double fAngleTmp;

	fErrorAdjAngle = 0.0;
	fDistance = 0.0;
	fAngle = 0.0;

	{//find summation of included angles
		TRAVVERTEX* pRec;

		for(i = 0, fAngleTmp = 0.0; i < arrayTravVertex.GetSize(); i++)
		{
			pRec = arrayTravVertex[i];
			fAngleTmp += pRec->fIncAngle;
		}
	}
	fAngle = fTotalAng - fAngleTmp;
	{//Find the closing error
		AcGePoint3d ptFirstVerDummy, ptFirstVer, ptTmp;
		TRAVVERTEX* pRec;
		
		pRec = arrayTravVertex[0];
		ptFirstVer.x = pRec->fEasting;
		ptFirstVer.y = pRec->fNorthing;
		ptFirstVer.z = 0.0;
		//
		pRec = arrayTravVertex[arrayTravVertex.GetSize() - 1];
		ptTmp.x = pRec->fEasting;
		ptTmp.y = pRec->fNorthing;
		ptTmp.z = 0.0;

		ads_polar(asDblArray(ptTmp), (arrayTravVertex[0])->fBearing, (arrayTravVertex[0])->fDistance, asDblArray(ptFirstVerDummy));
		//
		fErrorAdjAngle = Angle2D(ptFirstVerDummy, ptFirstVer);
		fDistance = Dist2D(ptFirstVer, ptFirstVerDummy);
	}
}
static BOOL ReadInpFile(const CString& strInpFile, CTravVertexArray& Array)
{
	CStdioFile	FileInp;
	long iLineNo;

	if(!FileInp.Open(strInpFile, CFile::modeRead|CFile::typeText))
	{
		ads_printf("\nERROR: Unable to open input file %s", (LPCSTR)strInpFile);
		return FALSE;
	}
	for(iLineNo = 0;;)
	{
		CStringArray strWordArray;
		
		if(!ReadNextValidLine(FileInp, strWordArray, iLineNo))
		{//...EOF reached 
			if(Array.GetSize() <= 0)
			{
				ads_printf("\nData file '%s' doesn't contain valid data\n", (LPCSTR)strInpFile);
				return FALSE;
			}
			return TRUE;
		}
		else
		{//
			double fDist, fAngle, fBearing;
			TRAVVERTEX* pRec;

			fDist = 0.0;
			fAngle = 0.0;
			fBearing = 0.0;

			if(Array.GetSize() == 1)
			{//the 2nd Valid Line
				if(strWordArray.GetSize() != 4)
				{
					ads_printf("\nERROR: Invalid Data found at line #%d in file '%s'\nThis Line should contain following fields <<Station Name>><<Included Angle PrevStn-CurStn-NextStn>><<Bearing to Next Stn>><<Distance from Current Stn to Next Stn>>", iLineNo, (LPCSTR)strInpFile);
					return FALSE;
				}
			}
			else
			{
				if(strWordArray.GetSize() != 3)
				{
					ads_printf("\nERROR: Invalid Data found at line #%d in file '%s'\nThis Line should contain following fields <<Station Name>><<Included Angle PrevStn-CurStn-NextStn>><<Distance from Current Stn to Next Stn>>", iLineNo, (LPCSTR)strInpFile);
					return FALSE;
				}
			}
			//Inc. Angle
			if(!ParseAngleStr(strWordArray[1], fAngle))
			{
				ads_printf("\nERROR: Invalid Included Angle found at line #%d in file '%s'\n", iLineNo, (LPCSTR)strInpFile);
				return FALSE;
			}
			else
			{//Chk the angle... should be in between 0 to 360 Deg
				if((fAngle < 0.0) || (fAngle >= (PI * 2.0)))
				{
					ads_printf("\nERROR: Invalid Included Angle found at line #%d in file '%s'\n", iLineNo, (LPCSTR)strInpFile);
					return FALSE;
				}
			}
			//Distance
			fDist = atof((LPCSTR)strWordArray[2]);
			if(fDist <= 0.0)
			{
				ads_printf("\nERROR: Invalid distance found at line #%d in file '%s'\n", iLineNo, (LPCSTR)strInpFile);
				return FALSE;
			}
			//Bearing for the 2nd Valid Line
			fBearing = 0.0;
			if(Array.GetSize() == 1)
			{
				if(!ParseAngleStr(strWordArray[3], fBearing))
				{
					ads_printf("\nERROR: Invalid Bearing(Angle) found at line #%d in file '%s'\n", iLineNo, (LPCSTR)strInpFile);
					return FALSE;
				}
				fBearing = ConvertBearing(fBearing);//Keep Bearing in AutoCAD Angle System(Anti Clockwise)
				if((fBearing < 0.0) || (fBearing >= (PI * 2.0)))
				{
					ads_printf("\nERROR: Invalid Bearing(Angle) found at line #%d in file '%s'\n", iLineNo, (LPCSTR)strInpFile);
					return FALSE;
				}
			}

			pRec = new TRAVVERTEX;
			memset(pRec, 0, sizeof(TRAVVERTEX));
			
			strcpy(pRec->szStn, (LPCSTR)strWordArray[0]);
			pRec->fBearing = fBearing;//Bearing in AutoCAD Angle System(Anti Clockwise)
			pRec->fDistance = fDist;
			pRec->fIncAngle = fAngle;

			Array.Add(pRec);
		}
	}
	return TRUE;
}

//
BOOL ParseAngleStr(const CString& strParam, double& fAngleInRad)
{
	const CString strWhite = CString("'");
	CStringArray strWordArray;
	double fValA, fValB, fValC;
	
	if(!CFBSheetData::XTractWordsFromLine(strWhite, strParam, strWordArray))
		return FALSE;
	
	if((strWordArray.GetSize() < 1) || (strWordArray.GetSize() > 3))
		return FALSE;


	switch(strWordArray.GetSize())
	{
	case 1:
		fAngleInRad = atof((LPCSTR)strWordArray[0]);
		fAngleInRad = DTOR(fAngleInRad);
		return TRUE;
	case 2:
		fValA = atof((LPCSTR)strWordArray[0]);
		fValB = atof((LPCSTR)strWordArray[1]);
		
		fAngleInRad = fValA + (fValB / 60.0);
		fAngleInRad = DTOR(fAngleInRad);
		return TRUE;
	case 3:
		fValA = atof((LPCSTR)strWordArray[0]);
		fValB = atof((LPCSTR)strWordArray[1]);
		fValC = atof((LPCSTR)strWordArray[2]);
		
		fAngleInRad = fValA + (fValB / 60.0) + (fValC / 3600.0);
		fAngleInRad = DTOR(fAngleInRad);
		return TRUE;
	default:
		return FALSE;
	}

	return TRUE;
}

//
//Converts Bearing(in Radian) to AutoCAD angle (in Radian)
//

CString FormatAngle(const double& fAngInRadParam)
{
	CString strTmp;
	double fDeg, fMin, fSec;
	BOOL bIsNegative;

	BreakAngle(fAngInRadParam, fDeg, fMin, fSec, bIsNegative);
	if(fSec == 0.0)
		strTmp.Format("%.0f'%.0f\"", fDeg, fMin);
	else
		strTmp.Format("%.0f'%.0f\"%.3f", fDeg, fMin, fSec);

	if(bIsNegative)
		strTmp =  CString("-") + strTmp;
	return strTmp;
}
double ConvertBearing(const double& fAngInRadParam)
{
	double fAngInRad;
	
	if(fAngInRadParam <= (PI / 2.0))
	{
		fAngInRad = ((PI / 2.0) - fAngInRadParam);
	}
	else
	{
		fAngInRad = ((PI / 2.0) - fAngInRadParam + PI * 2.0);
	}

#ifdef _DEBUG
	double fTmpA, fTmpB;
	fTmpA = RTOD(fAngInRadParam);
	fTmpB = RTOD(fAngInRad);
	ads_printf("\nFrom ConvertBearing()->> Bearing = %f (Deg) -->> %f (Deg)\n", fTmpA, fTmpB);
#endif //_DEBUG

	return fAngInRad;
}

//Converts AutoCAD Angle System to bearing
double ConvertAcadAngleToBearing(const double& fAngInRadParam)
{
	double fAngInRad;
	
	if(fAngInRadParam < (PI / 2.0))
	{
		fAngInRad =  (PI / 2.0) - fAngInRadParam;
	}
	else
	{
		fAngInRad = (PI * 2.0) - fAngInRadParam + (PI / 2.0);
	}

#ifdef _DEBUG
	//ads_printf("\nFrom ConvertAcadAngleToBearing()->> Bearing = %f (Deg) -->> %f (Deg)\n", RTOD(fAngInRadParam), RTOD(fAngInRad));
#endif //_DEBUG

	return fAngInRad;
}
static void BreakAngle(const double& fAngInRadParam, double& fDeg, double& fMin, double& fSec, BOOL& bIsNegative)
{
	const double fPrecisionInSecond = 1E-6;
	const char chDot = '.';
	double fAngInDeg, fTmp;
	CString strFmtVal, strTmpA, strTmpB;
	int iIndex;
	
	bIsNegative = (fAngInRadParam < 0.0);

	fDeg = 0.0;
	fMin = 0.0;
	fSec = 0.0;

	fAngInDeg = RTOD(fabs(fAngInRadParam));
	strFmtVal.Format("%f", fAngInDeg);
	{//
		strTmpA = strFmtVal;
		iIndex = strTmpA.Find(chDot);
		
		//
		strTmpB = strTmpA.Mid(0, iIndex);
		fDeg = atof(strTmpB);
		//
		strTmpB = strTmpA.Mid(iIndex);
		fMin = (fAngInDeg - fDeg) * 60.0;//With Fraction
		if(fMin == 0.0)
			return;

		fTmp = fMin;//Store val in minutes with fraction
		strFmtVal.Format("%f", fMin);
		strTmpA = strFmtVal;
		iIndex = strTmpA.Find(chDot);
		//
		strTmpB = strTmpA.Mid(0, iIndex);
		fMin = atof(strTmpB);
		//
		strTmpB = strTmpA.Mid(iIndex);
		fSec = (fTmp - fMin) * 60.0;//With Fraction
		if(fSec < fPrecisionInSecond)
			fSec = 0.0;
	}
}
