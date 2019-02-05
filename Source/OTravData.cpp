//OTravData.cpp
#include "stdafx.h"
#include "OTravData.h"
#include "SurveyData.h"
#include "DrawTable.h"
#include "ResultDlg.h"

const CString COTravData::m_strLyrDebug = CString("_DEBUG_");
COTravData::COTravData()
{
}
COTravData::~COTravData()
{
	DeallocatePtrArray(m_arrayRecs);
}
BOOL COTravData::ImportRec(const TRAVVERTEX& Rec, const BOOL& bIsLastRec/*=FALSE*/)
{
	TRAVVERTEX* pRec;
	
	pRec = new TRAVVERTEX;
	*pRec = Rec;
	
	if((m_arrayRecs.GetSize() == 1) || bIsLastRec)
	{
		pRec->fBearing = ConvertBearing(DTOR(pRec->fBearing));//Convert to AutoCAD System(Radian)
	}

	pRec->fIncAngle = DTOR(pRec->fIncAngle);//Convert to Radian
	m_arrayRecs.Add(pRec);
	return TRUE;
}
BOOL COTravData::HasData() const
{
	return (m_arrayRecs.GetSize() <= 0);
}
void COTravData::_CalcAdjustmentReqd(double& fValue, double& fDirection) const
{
	fValue = 0.0;
	fDirection = 0.0;
	if(m_arrayRecs.GetSize() <= 4)
	{
		AfxMessageBox("Invalid # of data points provided\nCalculation failed", MB_ICONSTOP);
		return;
	}
	TRAVVERTEX* pRec;
	AcGePoint3d ptFixed, ptNew;

	pRec = m_arrayRecs[m_arrayRecs.GetSize() - 2];

	ptFixed.x = pRec->fEasting;
	ptFixed.y = pRec->fNorthing;
	ptNew.x = pRec->fEasting + pRec->fEastingAdj;
	ptNew.y = pRec->fNorthing + pRec->fNorthingAdj;

	fValue = Dist2D(ptFixed, ptNew);
	fDirection = Angle2D(ptFixed, ptNew);

}
void COTravData::Adjust(const TRAVDWGPARAM& recParam)
{
	if(m_arrayRecs.GetSize() <= 4)
	{
		AfxMessageBox("Invalid # of data points provided\nCalculation failed", MB_ICONSTOP);
		return;
	}
	const BOOL bDraw = (recParam.fStnMarkTxtSize > 0.0);
	TRAVVERTEX* pRec;
	TRAVVERTEX* pRecNext;
	int i;
	double fBakBearing;

	LAYER(recParam.szLyrCalcTable, 3);
	LAYER(recParam.szLyrTraverseOrg, 1);
	LAYER(recParam.szLyrTraverseAdj, 2);
	LAYER(m_strLyrDebug, 4);

	{//Calculate GPS_2's co-ordinates
		AcGePoint3d ptGPS_1, ptGPS_2;
		TRAVVERTEX* pRecA;
		TRAVVERTEX* pRecB;

		pRecA = m_arrayRecs[0];
		pRecB = m_arrayRecs[1];

		ptGPS_1.x = pRecA->fEasting;
		ptGPS_1.y = pRecA->fNorthing;
		ptGPS_1.z = 0.0;

		ads_polar(asDblArray(ptGPS_1), pRecB->fBearing, pRecB->fDistance, asDblArray(ptGPS_2));

		pRecB->fEasting = ptGPS_2.x;
		pRecB->fNorthing = ptGPS_2.y;
	}
	{//Calculate GPS_4's co-ordinates
		AcGePoint3d ptGPS_1, ptGPS_2;
		TRAVVERTEX* pRecA;
		TRAVVERTEX* pRecB;

		pRecA = m_arrayRecs[m_arrayRecs.GetSize() - 2];
		pRecB = m_arrayRecs[m_arrayRecs.GetSize() - 1];

		ptGPS_1.x = pRecA->fEasting;
		ptGPS_1.y = pRecA->fNorthing;
		ptGPS_1.z = 0.0;

		ads_polar(asDblArray(ptGPS_1), pRecB->fBearing, pRecB->fDistance, asDblArray(ptGPS_2));

		pRecB->fEasting = ptGPS_2.x;
		pRecB->fNorthing = ptGPS_2.y;
	}
	//NOTE : Bearing in AutoCAD Angle System(Anti Clockwise)
	for(i = 1; i < (m_arrayRecs.GetSize() - 1); i++)
	{
		pRec = m_arrayRecs[i];
		pRecNext = m_arrayRecs[i + 1];
		
		//Calculate Bearing ...
		fBakBearing = pRec->fBearing + PI;
		if(fBakBearing > (PI * 2.0))
			fBakBearing = fBakBearing - (PI * 2.0);

		pRecNext->fBearing = fBakBearing - pRec->fIncAngle;
		if(pRecNext->fBearing < 0.0)
			pRecNext->fBearing = (2 * PI) + pRecNext->fBearing;
	
		//ads_printf("\nRec#%d>>[%s][%s]", i, pRec->szStn, FormatAngle(ConvertAcadAngleToBearing(pRec->fBearing)));
	}
	{//Calculate Co-Ords
		pRec = m_arrayRecs[1];
		const AcGePoint3d ptBase(pRec->fEasting, pRec->fNorthing, 0.0);
		AcGePoint3d ptCur, ptPrev;

		for(i = 1; i < (m_arrayRecs.GetSize() - 1); i++)
		{
			pRec = m_arrayRecs[i];

			if(i == 1)
			{
				ptCur = ptBase;
			}
			else if(i == (m_arrayRecs.GetSize() - 2))
			{//2nd Last point fixed....Calculate only the adjustment
				AcGePoint3d ptTmp;

				ads_polar(asDblArray(ptPrev), pRec->fBearing, pRec->fDistance, asDblArray(ptCur));
				
				pRec->fNorthingAdj = pRec->fNorthing - ptCur.y;
				pRec->fEastingAdj = pRec->fEasting - ptCur.x;
			}
			else
			{//for all other
				ads_polar(asDblArray(ptPrev), pRec->fBearing, pRec->fDistance, asDblArray(ptCur));
				pRec->fNorthing = ptCur.y;
				pRec->fEasting = ptCur.x;
			}
			ptPrev = ptCur;	
		}
	}
	{//Calculate Corrections....
		double fTotCorrection, fDirection, fCorrection, fTraverseLen, fCumTraverseLen;
		AcGePoint3d ptCur, ptNew;

		_CalcAdjustmentReqd(fTotCorrection, fDirection);
		for(i = 2, fTraverseLen = 0.0; i < (m_arrayRecs.GetSize() - 2); i++)
		{
			pRec = m_arrayRecs[i];
			fTraverseLen += pRec->fDistance;
		}
		for(i = 2, fCumTraverseLen = 0.0; i < (m_arrayRecs.GetSize() - 2); i++)
		{
			fCorrection = fCumTraverseLen * fTotCorrection / fTraverseLen;
			
			pRec = m_arrayRecs[i];
			ptCur.x = pRec->fEasting;
			ptCur.y = pRec->fNorthing;

			ads_polar(asDblArray(ptCur), fDirection, fCorrection, asDblArray(ptNew));
			
			pRec->fNorthingAdj = pRec->fNorthing - ptNew.y;
			pRec->fEastingAdj = pRec->fEasting - ptNew.x;

			fCumTraverseLen += pRec->fDistance;
		}
	}
	//..Adjust Included angle
	for(i = 2; i <= (m_arrayRecs.GetSize() - 2); i++)
	{
		double fAngleNew;
		
		_CalculateCorrIncAngle(m_arrayRecs, i, fAngleNew);
		
		pRec = m_arrayRecs[i];
		pRec->fIncAngleAdj = fAngleNew - pRec->fIncAngle;
	}
	{//Write Output in dwg/data file)
		CStdioFile fileOut;

		//Open Out file
		if(!fileOut.Open(recParam.szOutFile, CFile::modeCreate|CFile::modeWrite|CFile::typeText))
		{
			ads_printf("\nERROR: Unable to open output file %s", (LPCSTR)recParam.szOutFile);
			return;
		}
		_WriteOutput(fileOut, recParam);
		fileOut.Close();
		ShowResultDlg(recParam.szOutFile);
		Dump();
	}
	if(bDraw)
	{//Draw the traverse
		_DrawTraverse(m_arrayRecs, recParam);
	}
}
void COTravData::_WriteOutput(CStdioFile& fileOut, const TRAVDWGPARAM& recParam) const
{//Dump Results.....
	const BOOL bDraw = (recParam.fStnMarkTxtSize > 0.0);
	CPtrArray arrayOfPointers;
	double fCumIncAngAdj;
	
	fCumIncAngAdj = 0.0;
	for(int i = 0; i < m_arrayRecs.GetSize(); i++)
	{
		CStringArray* pStrArray;
		CString	strTmp, strTmpA;
		TRAVVERTEX* pRec;

		
		pRec = m_arrayRecs[i];
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
			if((i == 0) || (i == (m_arrayRecs.GetSize() - 1)))
			{
				strTmp = "- NA -";
			}
			pStrArray->Add(strTmp);
		}
		{//Inc Angle...#3
			strTmp = FormatAngle(pRec->fIncAngle);
#ifdef _DEBUG
			strTmpA.Format("[%f]", RTOD(pRec->fIncAngle));
			strTmp += strTmpA;
#endif//_DEBUG
			if((i == 0) || (i == (m_arrayRecs.GetSize() - 1)))
			{
				strTmp = "- NA -";
			}
			pStrArray->Add(strTmp);
		}
		{//Correction in Included angle #4
			strTmp = FormatAngle(pRec->fIncAngleAdj);
#ifdef _DEBUG
			strTmpA.Format("[%f]", RTOD(pRec->fIncAngleAdj));
			strTmp += strTmpA;
#endif//_DEBUG
			if((i == 0) || (i == (m_arrayRecs.GetSize() - 1)))
			{
				strTmp = "- NA -";
			}
			pStrArray->Add(strTmp);
		}
		{//Corrected Inc.Angle #5
			strTmp = FormatAngle(pRec->fIncAngle + pRec->fIncAngleAdj);
#ifdef _DEBUG
			strTmpA.Format("[%f]", RTOD(pRec->fIncAngle + pRec->fIncAngleAdj));
			strTmp += strTmpA;
#endif//_DEBUG
			if((i == 0) || (i == (m_arrayRecs.GetSize() - 1)))
			{
				strTmp = "- NA -";
			}
			pStrArray->Add(strTmp);
		}
		{//Distance to Next Station...#6
			strTmp.Format("%f", pRec->fDistance);
			if((i == 0) || (i == (m_arrayRecs.GetSize() - 1)))
			{
				strTmp = "- NA -";
			}
			pStrArray->Add(strTmp);
		}
		{//Northing #7
			strTmp.Format("%f", pRec->fNorthing);
			pStrArray->Add(strTmp);
			//Northing Correction #8
			strTmp.Format("%f", pRec->fNorthingAdj);
			if((i == 0) || (i == 1) || (i == (m_arrayRecs.GetSize() - 1)) || (i == (m_arrayRecs.GetSize() - 2)))
			{
				strTmp = "- NA -";
				if(i == (m_arrayRecs.GetSize() - 2))
					strTmp.Format("Closing Error(N) : %f", pRec->fNorthingAdj);
			}
			pStrArray->Add(strTmp);
			//Corrected-N #9
			strTmp.Format("%f", (pRec->fNorthing + pRec->fNorthingAdj));
			if((i == 0) || (i == 1) || (i == (m_arrayRecs.GetSize() - 1)) || (i == (m_arrayRecs.GetSize() - 2)))
			{
				strTmp = "- NA -";
			}
			pStrArray->Add(strTmp);
		}
		
		{//Easting #10
			strTmp.Format("%f", pRec->fEasting);
			pStrArray->Add(strTmp);
			//Easting Correction #11
			strTmp.Format("%f", pRec->fEastingAdj);
			if((i == 0) || (i == 1) || (i == (m_arrayRecs.GetSize() - 1)) || (i == (m_arrayRecs.GetSize() - 2)))
			{
				strTmp = "- NA -";
				if(i == (m_arrayRecs.GetSize() - 2))
					strTmp.Format("Closing Error(E) : %f", pRec->fEastingAdj);
			}
			pStrArray->Add(strTmp);
			//Corrected-E #12
			strTmp.Format("%f", (pRec->fEasting + pRec->fEastingAdj));
			if((i == 0) || (i == 1) || (i == (m_arrayRecs.GetSize() - 1)) || (i == (m_arrayRecs.GetSize() - 2)))
			{
				strTmp = "- NA -";
			}
			pStrArray->Add(strTmp);
		}
	
		//
		arrayOfPointers.Add(pStrArray);
		//
		fCumIncAngAdj += pRec->fIncAngleAdj;
		if(i == m_arrayRecs.GetSize() - 1)
		{//Captions
			pStrArray = new CStringArray();
			
			strTmp.Format("%s", "");						pStrArray->Add(strTmp);
			strTmp.Format("%s", "");						pStrArray->Add(strTmp);
			strTmp.Format("%s", "");						pStrArray->Add(strTmp);
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
		CString	strLine;
#ifdef _DEBUG
		strLine = "NOTE: The bearing Values are in AutoCAD Angle Format(In degrees)";
		fileOut.WriteString(strLine);
		fileOut.WriteString(CString("\n"));
#endif// _DEBUG
		strLine.Format("****|****|****|****|****|****|->SurvUtilARX on %s|<-|****|****|****|****|****|****\n\n", COleDateTime::GetCurrentTime().Format());
		fileOut.WriteString(strLine);
		fileOut.WriteString(CString("\n"));

		for(int i = 0; i < arrayOfPointers.GetSize(); i++)
		{
			CStringArray* pStrArray;
			
			pStrArray = (CStringArray*)arrayOfPointers[i];
			strLine = "";
			for(int j = 0; j < pStrArray->GetSize(); j++)
			{
				if(j == 0)
				{
					strLine += pStrArray->GetAt(j);
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
void COTravData::Dump() const
{
#ifdef _DEBUG
	
	int i;
	TRAVVERTEX* pRec;

	for(i = 0; i < m_arrayRecs.GetSize(); i++)
	{
		pRec = m_arrayRecs[i];
		ads_printf("\nRec#%d>>[%s][%f][%f][%f][%f][%f][%f][%f][%f]", i, pRec->szStn, pRec->fIncAngle, pRec->fDistance, ConvertAcadAngleToBearing(pRec->fBearing), pRec->fIncAngleAdj, pRec->fNorthing, pRec->fEasting, pRec->fNorthingAdj, pRec->fEastingAdj);
	}
 

#endif//_DEBUG
}
void COTravData::_CalculateCorrIncAngle(const CTravVertexArray& arrayTravVertex, const int& iStn, double& fAngle)
{
	AcGePoint3d ptVertex, ptLeft, ptRite;
	TRAVVERTEX* pRec;

	fAngle = 0.0;
	if((iStn >= (arrayTravVertex.GetSize() - 1)) || (iStn <= 0) || (arrayTravVertex.GetSize() <= 2))
	{
		ads_printf("\nERROR: Control should not reach here!!!");
		return;//ERROR
	}
	pRec = arrayTravVertex[iStn];
	ptVertex.y = pRec->fNorthing + pRec->fNorthingAdj;
	ptVertex.x = pRec->fEasting + pRec->fEastingAdj;
	ptVertex.z = 0.0;

	{//Intermediate
		pRec = arrayTravVertex[iStn - 1];
		ptLeft.y = pRec->fNorthing + pRec->fNorthingAdj;
		ptLeft.x = pRec->fEasting + pRec->fEastingAdj;
		ptLeft.z = 0.0;
		pRec = arrayTravVertex[iStn + 1];
		if((iStn+1) == (arrayTravVertex.GetSize() - 2))
		{//Third GPS point
			ptRite.y = pRec->fNorthing;
			ptRite.x = pRec->fEasting;
			ptRite.z = 0.0;
		}
		else
		{
			ptRite.y = pRec->fNorthing + pRec->fNorthingAdj;
			ptRite.x = pRec->fEasting + pRec->fEastingAdj;
			ptRite.z = 0.0;
		}
	}
	double fAngleLeft, fAngleRite;
	fAngleLeft = Angle2D(ptVertex, ptLeft);
	fAngleRite = Angle2D(ptVertex, ptRite);
	if(fAngleRite > fAngleLeft)
		fAngle = (PI * 2.0 ) - (fAngleRite - fAngleLeft);
	else
		fAngle = fAngleLeft - fAngleRite;
}

void COTravData::_DrawTraverse(const CTravVertexArray& arrayTravVertex, const TRAVDWGPARAM& recParam)
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
			TEXTMID(ptCur, pRec->szStn, 0.0, recParam.fStnMarkTxtSize, recParam.szLyrTraverseOrg);
			DRAWPOINT(ptCur, recParam.szLyrTraverseOrg);
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
			TEXTMID(ptCur, pRec->szStn, 0.0, recParam.fStnMarkTxtSize, recParam.szLyrTraverseAdj);
			CIRCLE(ptCur, recParam.fStnMarkTxtSize, recParam.szLyrTraverseAdj);
			DRAWPOINT(ptCur, recParam.szLyrTraverseAdj);
		}
		POLYLINE(arrayVertexes, recParam.szLyrTraverseAdj, FALSE);
	}
}