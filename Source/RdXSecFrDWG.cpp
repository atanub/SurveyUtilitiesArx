// RdXSecFrDWG.cpp : implementation file
//
#include "stdafx.h"
#include "Util.h"
#include "DrawUtil.h"
#include "RdXSecFrDWG.h"
#include "XData.h"
#include "GeneralUtilFuncs.h"

/* CSD Output file format
**      CH   RL  
9		10   5.0
1	-16	11
1	-12.5	3.65	
3	-4		9.628
2	-2		9.631
1	-1		9.626	
1	0		9.637	
1	1		9.649	
1	2		9.604	
1	4		9.613	
1	18		10.00
1	60		89.0
10
*/
const char*		CRdXSecFrDWG::m_pszLyrDebug				= "SURVUTIL_INSPECTION_LYR";
const char*		CRdXSecFrDWG::m_pszGenLyrName			= "GENERATED_POINTS";
const double	CRdXSecFrDWG::m_fMinBufferInDWGUnits	= 15;
const double	CRdXSecFrDWG::m_fDatumIsMultipleOf		= 5.0;

void CRdXSecFrDWG::RdXSecFrDWG(const RDXSECOFFELEVDATA& RecParam)
{
	AcGePoint3dArray PIArray;
	AcDbVoidPtrArray XSecPtsArray; //Array of 'AcGePoint3dArray' Objects
	AcGePoint3d ptULCornerOfSheet;
	CPointPair3DPtrArray arrayRefXSecLines;
	const AcGePoint3dArray& DatumRangeArray = RecParam.arrayDatumRanges;
	int i;
	CPointAndLyrInfoCache objPointAndLyrInfoCache;

	if(!GetPIData(PIArray))
		return;

	if(ads_usrbrk() == 1)
	{
		ads_printf("\nUser break encountered !\n");
		goto CLEANUP;
	}
	
	if(!GetXSecPtsArray(XSecPtsArray))
		goto CLEANUP;

	ads_printf("\nSelect the reference cross section lines......\n");
	if(!GetUserSpecXSecRefLines(arrayRefXSecLines))
	{//for safety....
		for(i = 0; i < arrayRefXSecLines.GetSize(); i++)
		{
			POINTPAIR3D* pRec;
			pRec = arrayRefXSecLines[i];
			delete pRec;
		}
		if(arrayRefXSecLines.GetSize() > 0)
			arrayRefXSecLines.RemoveAll();
	}
	{//Get all the point coords....for Ordinate Annotation
		const char* szprompts[] = {"Select points for Ordinate Annotation :", "Remove Points :"};
		ads_name SelSet;

		if(SelectPoints(SelSet, szprompts[0], szprompts[1]))
		{
			if(!CGeneralUtilFuncs::ExtractPtsNLyrsFromSelSet(SelSet, objPointAndLyrInfoCache.m_arrayForPoints, objPointAndLyrInfoCache.m_arrayLyrNames))
			{
				ads_ssfree(SelSet);//Free Selset
				goto CLEANUP;
			}
		}
		ads_ssfree(SelSet);//Free Selset
	}

	//Proccess points...
	for(; CGeneralUtilFuncs::IsCurSortDirOK(PIArray) == 0;)
	{
		PIArray.reverse();
	}

	if(!SortXSecPtsArray(PIArray, XSecPtsArray))
		goto CLEANUP;
	
	if(RecParam.iDrawPathWIthMinMaxElev > 0)
	{//
		LAYER(RecParam.szLyrMinMaxPIDPath, 2);
		//RecParam.iDrawPathWIthMinMaxElev Values
		//0 = Don't draw
		//1 = Draw PID having Min Elevs
		//2 = Calculate PID having Min Elevs and Draw C/S [[[DISABLED]]]
		//4 = Draw PID having Max Elevs
		//8 = Calculate PID having Max Elevs and Draw C/S [[[DISABLED]]]

		if((RecParam.iDrawPathWIthMinMaxElev & 1) || (RecParam.iDrawPathWIthMinMaxElev & 2))
		{//2 = MIN Elev
			AcGePoint3dArray arrayResult;

			if(!FindPathContainingMinMaxLevels(XSecPtsArray, FALSE, arrayResult))
				goto CLEANUP;
			//
			if(RecParam.iDrawPathWIthMinMaxElev & 1)
			{
				POLYLINE(arrayResult, RecParam.szLyrMinMaxPIDPath, TRUE);
			}
			/*
			if(RecParam.iDrawPathWIthMinMaxElev & 2)
			{
				PIArray.removeSubArray(0, (PIArray.length() - 1));
				PIArray.append(arrayResult);
			}
			*/
		}
		if((RecParam.iDrawPathWIthMinMaxElev & 4) || (RecParam.iDrawPathWIthMinMaxElev & 8))
		{//4 = MAX Elev
			AcGePoint3dArray arrayResult;

			if(!FindPathContainingMinMaxLevels(XSecPtsArray, TRUE, arrayResult))
				goto CLEANUP;
			//
			if(RecParam.iDrawPathWIthMinMaxElev & 4)
			{
				POLYLINE(arrayResult, RecParam.szLyrMinMaxPIDPath, TRUE);
			}
			/*
			if(RecParam.iDrawPathWIthMinMaxElev & 8)
			{
				PIArray.removeSubArray(0, (PIArray.length() - 1));
				PIArray.append(arrayResult);
			}
			*/
		}
	}
	
	if(arrayRefXSecLines.GetSize() > 0)
	{//Transform Cross section points.....
		if(!TransformAllXSecPoints(PIArray, XSecPtsArray, arrayRefXSecLines))
		{
			goto CLEANUP;
		}
	}
	{//Generate Points in Equal interval...
		AcGePoint3dArray arrayNewPoints;

		if(!GenOrdsInEqInterval(XSecPtsArray, PIArray, RecParam.fOrdInterval, (RecParam.bDrawGeneratedPoints) ? &arrayNewPoints : 0L))
		{
			goto CLEANUP;
		}
		if(RecParam.bDrawGeneratedPoints)
		{
			LAYER(m_pszGenLyrName, 1);
			DRAWPOINTS(arrayNewPoints, m_pszGenLyrName);
		}
	}

	if(RTNORM == ads_getpoint(0L, "\nPick upper left corner of starting drawing sheet :", asDblArray(ptULCornerOfSheet)))
	{////Draw......
		DrawAllSections(ptULCornerOfSheet, XSecPtsArray, PIArray, DatumRangeArray, RecParam, &objPointAndLyrInfoCache);
	}
	else
	{
		ads_printf("\nERROR: User termination\n");
	}
	goto CLEANUP;

	//////////////////////////////////////////////////////////////////////
	CLEANUP :
	{

		DeAllocateXSecPtsArray(XSecPtsArray);
		//
		for(i = 0; i < arrayRefXSecLines.GetSize(); i++)
		{
			POINTPAIR3D* pRec;
			pRec = arrayRefXSecLines[i];
			delete pRec;
		}
	}
}
BOOL CRdXSecFrDWG::GetPIData(AcGePoint3dArray& Array)
{
	const char* pszPmt = "\nSelect polyline as surveyed path :";
	ads_name entName;
	AcGePoint3d pt3D;

	if(RTNORM != ads_entsel(pszPmt, entName, asDblArray(pt3D)))
	{
		ads_printf("\nERROR: No entity selected\n");
		return FALSE;
	}
	//Extract vertices...
	return XtractVertsFr3DPLine(entName, Array);
}

BOOL CRdXSecFrDWG::XtractVertsFr3DPLine(ads_name entName, AcGePoint3dArray& Array, CString* pStrLyr /* = 0L*/)
{
	AcGePoint3d pt3D;
	AcDb3dPolyline* pEnt;
	AcDbObjectId objId;
	AcDbObject* pObj;
	Acad::ErrorStatus ErrCode;

	//Extract vertices...
	ErrCode = acdbGetObjectId(objId, entName);
	if(ErrCode != Acad::eOk)
	{
		return FALSE;
	}
	ErrCode = acdbOpenObject(pObj, objId, AcDb::kForRead);
	if(ErrCode != Acad::eOk)
	{
		return FALSE;
	}
	if(pObj->isA() != AcDb3dPolyline::desc())
	{
		ads_printf("\nERROR: Entity selected is not a 3d polyline\n");
		pObj->close();
		return FALSE;
	}
	pEnt = (AcDb3dPolyline*)pObj;
	if(AcDb::Poly3dType::k3dSimplePoly != pEnt->polyType())
	{
		ads_printf("\nERROR: Selected 3D Polyline contains curved segments !\n");
		pObj->close();
		return FALSE;
	}
	{//Extract points...
		AcDbObjectIterator* pIterator;
		
		pIterator = ((AcDb3dPolyline*)pObj)->vertexIterator();
		for(pIterator->start(); (pIterator->done() == Adesk::kFalse); pIterator->step())
		{
			AcDbObjectId objectIdTmp;
			AcDb3dPolylineVertex* pVert;
			
			objectIdTmp = pIterator->objectId();

			ErrCode = ((AcDb3dPolyline*)pObj)->openVertex(pVert, objectIdTmp, AcDb::kForRead);
			if(ErrCode != Acad::eOk)
			{
				delete pIterator;
				ads_printf("\nERROR: Error occured while iterating vertices!\n");
				pObj->close();
				return FALSE;
			}
			Array.append(pVert->position());
			pVert->close();
		}
		delete pIterator;
	}
	if(0L != pStrLyr)
	{
		char* pszLyr;
		
		pszLyr = pEnt->layer();
		if(0L != pszLyr)
			pStrLyr->Format("%s", pszLyr);
		
		delete []pszLyr;
	}
	pObj->close();
	return TRUE;
}
BOOL CRdXSecFrDWG::GetXSecPtsArray(AcDbVoidPtrArray& ArrayParam)
{
	ads_name SelSet;
	{//Get user selected  ....
		const char* szPrompts[] = {"Select 3d polylines for cross section :", "Remove polylines :"};
		struct  resbuf  Rb01;
		const char szEntName[] = "POLYLINE";
		// Entity type
		Rb01.restype = 0;
		Rb01.resval.rstring = (char*)szEntName;
		Rb01.rbnext = 0L;
		
		if(ads_ssget(":$", szPrompts, 0L, &Rb01, SelSet) != RTNORM)
		{
			ads_printf("\nERROR: No entity selected\n");
			return FALSE;
		}
	}
	//
	long i, iCount, iNoOfEnt;
	if(ads_sslength(SelSet, &iNoOfEnt) != RTNORM)
	{
		ads_ssfree(SelSet);//Free Selset
		return FALSE;//ERROR
	}
	//Extract 'n Draw Points
	for(i = iCount = 0; i < iNoOfEnt; i++)
	{
		ads_name Entity;
		AcGePoint3dArray* pArray;

		if(ads_ssname(SelSet, i, Entity) != RTNORM)
		{
			ads_printf("\nERROR: AutoCAD System error !\n");
			ads_ssfree(SelSet);//Free Selset
			return FALSE;//ERROR
		}
		pArray = new AcGePoint3dArray;
		if(!XtractVertsFr3DPLine(Entity, *pArray))
		{
			ads_printf("\nERROR: AutoCAD System error !\n");
			ads_ssfree(SelSet);//Free Selset
			delete pArray;
			DeAllocateXSecPtsArray(ArrayParam);
			return FALSE;//ERROR
		}
		ArrayParam.append((void*)pArray);
		iCount++;

	}

	ads_ssfree(SelSet);//Free Selset
	return TRUE;
}
void CRdXSecFrDWG::DeAllocateXSecPtsArray(AcDbVoidPtrArray& Array)
{
	long i;
	
	for(i = 0; i < Array.length(); i++)
	{
		AcGePoint3dArray* pArray;
		
		pArray = (AcGePoint3dArray*)Array.at(i);
		delete pArray;
	}
	for(; Array.length() > 0;)
	{
		Array.removeFirst();
	}
}
BOOL CRdXSecFrDWG::ReArrangeCSData(AcGePoint3dArray& arrayXSecPts, const AcGePoint3dArray& PIDArray, const AcGePoint3d& ptMidOfXSec)
{
	long i;
	AcGePoint3d ptStart, ptEnd;
	
	if(arrayXSecPts.length() <= 1)
		return FALSE;//Control should never reach here!!!

	for(i = 1; i < PIDArray.length(); i++)
	{
		ptStart = PIDArray[i - 1];
		ptEnd = PIDArray[i - 0];
		if(IsPointOnLine(ptMidOfXSec, ptStart, ptEnd))
		{//
			int iRetVal;
			
			if((arrayXSecPts[0] == ptMidOfXSec) || (arrayXSecPts[arrayXSecPts.length() - 1] == ptMidOfXSec))
			{//Special case: All the CS points lies in one side of alignment
				if(arrayXSecPts[0] != ptMidOfXSec)
				{
					arrayXSecPts.reverse();
				}
				iRetVal = PtInWhichSide(arrayXSecPts[arrayXSecPts.length() - 1], ptStart, ptEnd);
				if(iRetVal < 0)
				{//All the points lies in left hand side....
					//direction is from Left->Rite
					//need to revert the direction for CalculateOffset() compatibility issue
					arrayXSecPts.reverse();
					#ifdef _DEBUG
						//LAYER(m_pszLyrDebug, 1);
						//POLYLINE(arrayXSecPts, m_pszLyrDebug, TRUE);
						//LINE(ptStart, ptEnd, m_pszLyrDebug);
					#endif //_DEBUG
				}
				//ads_printf("\nAlgo (Special case: All the CS points lies in one side of alignment) under construction!");
				return TRUE;
			}
			else
			{
				iRetVal = PtInWhichSide(arrayXSecPts[0], ptStart, ptEnd);
				if(iRetVal > 0)
				{//Rite->Left direction!! so reverse....
					arrayXSecPts.reverse();//(defun c:pp() (command "pedit" PAUSE "ed" PAUSE "X" "" ""))
					#ifdef _DEBUG
						//LAYER(m_pszLyrDebug, 1);
						//POLYLINE(arrayXSecPts, m_pszLyrDebug, TRUE);
						//LINE(ptStart, ptEnd, m_pszLyrDebug);
					#endif //_DEBUG
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}
BOOL CRdXSecFrDWG::FindPathContainingMinMaxLevels(const AcDbVoidPtrArray& XSecArray, const BOOL bIsForMaxPath, AcGePoint3dArray& arrayResult)
{
	long i;

	if(arrayResult.length() > 0)
	{
		ads_printf("\nFATAL ERROR: Control should never reach here!\n");
		return FALSE;
	}
	for(i = 0; i < XSecArray.length(); i++)
	{
		long j;
		const AcGePoint3dArray* pArrayCSD = (AcGePoint3dArray*)XSecArray[i];
		AcGePoint3d ptTheDest;

		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encountered !\n");
			return FALSE;
		}
		if(pArrayCSD->length() <= 0)
		{
			ads_printf("\nFATAL ERROR: Control should never reach here!\n");
			return FALSE;
		}
		ptTheDest = pArrayCSD->at(0);
		for(j = 1; j < pArrayCSD->length(); j++)
		{
			if(bIsForMaxPath)
			{
				if(pArrayCSD->at(j).z > ptTheDest.z)
				{
					ptTheDest = pArrayCSD->at(j);
				}
			}
			else
			{
				if(pArrayCSD->at(j).z < ptTheDest.z)
				{
					ptTheDest = pArrayCSD->at(j);
				}
			}
		}
		arrayResult.append(ptTheDest);
	}
	if(XSecArray.length() != arrayResult.length())
	{
		ads_printf("\nFATAL ERROR: Control should never reach here!\n");
		return FALSE;
	}
	return(arrayResult.length() > 0);
}
BOOL CRdXSecFrDWG::SortXSecPtsArray(const AcGePoint3dArray& PIDArray, AcDbVoidPtrArray& XSecArray)
{
	long i, iIndex;
	AcDbVoidPtrArray XSecArrayNew;
	
	for(; XSecArray.length() > 0;)
	{
		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encountered !\n");
			return FALSE;
		}
		iIndex =  GetNearestXSec(PIDArray, XSecArray);
		if(iIndex < 0)
		{
			ads_printf("\nERROR: Failed to find cross section points\nPossible cause : None of cross section designated polyline vertices lies on surveyed path\n");
			return FALSE;
		}

		XSecArrayNew.append(XSecArray[iIndex]);
		XSecArray.removeAt(iIndex);
	}
	XSecArray.append(XSecArrayNew);
	
	//Make CSD uni directional....
	for(i = 0; i < XSecArray.length(); i++)
	{
		AcGePoint3d ptMidOfXSec;
		double fDistDUMMY;
		CString strMsg;

		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encountered !\n");
			return FALSE;
		}
		if(!GetChainage((AcGePoint3dArray*)XSecArray[i], PIDArray, fDistDUMMY, &ptMidOfXSec))
		{
			strMsg.Format("ERROR: Failed to calculate chainage for cross section #%d\nPossible cause : No points found on road centre line", i+1);
			ads_printf("\n%s\n", strMsg);
			ShowInvalidCSDataSeg(*((AcGePoint3dArray*)XSecArray[i]), strMsg);

			strMsg.Format("Do you want to draw the erroneous polyline defined as C/S data in Layer \"%s\"?", m_pszLyrDebug);
			if(IDYES == AfxMessageBox(strMsg, MB_ICONQUESTION|MB_YESNO))
			{
				LAYER(m_pszLyrDebug, 2);
				POLYLINE(*((AcGePoint3dArray*)XSecArray[i]), m_pszLyrDebug, TRUE);
			}
			return FALSE;
		}
		if(!ReArrangeCSData(*((AcGePoint3dArray*)XSecArray[i]), PIDArray, ptMidOfXSec))
		{
			ads_printf("\nERROR: Failed to find cross section points\nPossible cause : None of cross section designated polyline vertices lies on surveyed path\n");
			return FALSE;
		}
	}
	return TRUE;
}

long CRdXSecFrDWG::GetNearestXSec(const AcGePoint3dArray& PIDArray, const AcDbVoidPtrArray& XSecArray)
{
	long i, iIndex;
	double fDist, fMinDist;
	CString strMsg;

	for(i = 0; i < XSecArray.length(); i++)
	{
		if(!GetChainage((AcGePoint3dArray*)XSecArray[i], PIDArray, fDist))
		{
			strMsg.Format("ERROR: Failed to calculate chainage for cross section #%d\nPossible cause : No points found on road centre line", i+1);
			ads_printf("\n%s\n", strMsg);
			ShowInvalidCSDataSeg(*((AcGePoint3dArray*)XSecArray[i]), strMsg);

			strMsg.Format("Do you want to draw the erroneous polyline defined as C/S data in Layer \"%s\"?", m_pszLyrDebug);
			if(IDYES == AfxMessageBox(strMsg, MB_ICONQUESTION|MB_YESNO))
			{
				LAYER(m_pszLyrDebug, 2);
				POLYLINE(*((AcGePoint3dArray*)XSecArray[i]), m_pszLyrDebug, TRUE);
			}
			return -1;
		}
		if(i == 0)
		{
			fMinDist = fDist;
			iIndex = 0;
		}
		if(fDist < fMinDist)
		{
			iIndex = i;
			fMinDist = fDist;
		}
	}
	return iIndex;
}
BOOL CRdXSecFrDWG::GetChainage(const AcGePoint3dArray* pArray, const AcGePoint3dArray& PIDArray, double& fChg, AcGePoint3d* pPtMidOfXSec /*= 0L*/, BOOL* pbIsVersFrLeftToRight /*= 0L*/)
{
	long i, j;
	AcGePoint3d ptStart, ptEnd, ptOnXSec;
	
	for(i = 1, fChg = 0.0; i < PIDArray.length(); i++)
	{
		ptStart = PIDArray[i - 1];
		ptEnd = PIDArray[i - 0];
		for(j = 0; j < pArray->length(); j++)
		{
			ptOnXSec = pArray->at(j);
			if(IsPointOnLine(ptOnXSec, ptStart, ptEnd))
			{//
				double fAnglePI;

				fAnglePI = Angle2D(ptStart, ptEnd);
				fChg += Dist2D(ptStart, ptOnXSec);
				if(pPtMidOfXSec != 0L)
				{
					*pPtMidOfXSec = ptOnXSec;
				}
				if(pbIsVersFrLeftToRight != 0L)
				{
					double fAngleTmp;
					
					fAngleTmp = Angle2D(ptOnXSec, pArray->at(0));
					*pbIsVersFrLeftToRight = (fAngleTmp > fAnglePI) && (fAngleTmp < fAnglePI + PI);
				}
				return TRUE;
			}
		}
		fChg += Dist2D(ptStart, ptEnd);
	}
	return FALSE;
}
//Drawing Routines.....
void CRdXSecFrDWG::DrawOrdinates(const AcGePoint3d& ptGrphOrg, const AcGePoint3dArray& XSecPts, const AcGePoint3d& ptMidOfXSec, const RDXSECOFFELEVDATA& RecParam, const double& fMinOff, const double& fMaxOff, const double& fDatum, const double& fChainage, CStdioFile* pOutFile, CPointAndLyrInfoCache* pObjPointAndLyrInfoCache/* = 0L*/)
{
	double fXAxisLen, fOrdDist, fOffset, fLastAnnOffset;
	AcGePoint3dArray ProfTopPtsArray;
	AcGePoint3d PtStart, PtEnd;
	int j, i;
	ads_name entLast;
	CString strTmp;

	fXAxisLen = (fabs(fMinOff) + fMaxOff);
	const int iNumOfRows = (RecParam.bDrawNorthingEastingAnnSection) ? 4:2;

	for(i = 0, fLastAnnOffset = 0.0; i < XSecPts.length(); i++)
	{
		const AcGePoint3d& ptOnXSecn = XSecPts[i];
		if(!CalculateOffset(ptOnXSecn, XSecPts, ptMidOfXSec, fOffset))
		{
			return;
		}
		const char* pszOrdinateAnn = 0L;
		if(0L != pObjPointAndLyrInfoCache)
			pszOrdinateAnn = pObjPointAndLyrInfoCache->FindLayer(&ptOnXSecn);

		const double fPrecision = 1.0E-4;
		const double fRem = fabs((RecParam.fOrdInterval > 0.0) ? fmod(fOffset, RecParam.fOrdInterval) : 0.0);
		const BOOL bAnnotateThisOrd = (((i + 1) == XSecPts.length()) || (i == 0) || (RecParam.fOrdInterval <= 0.0) || ((RecParam.fOrdInterval > 0.0) && ((fRem <= fPrecision) || (fabs(fRem - RecParam.fOrdInterval) <= fPrecision))));

		if((fabs(fLastAnnOffset - fOffset) <= fPrecision) && (i > 0))
			continue;

		{//Write the out file............
			if(pOutFile != 0L)
			{
				const char* pszCSRecFmt = "%d\t%s\t%s\t[N:%sE:%s]\n";
				CString strToWrite;
				if(i == 0)
				{
					const char* pszCSHeaderFmt = "**\tCHG.\tDatum\n%d\t%s\t%s\n";

					strToWrite.Format(pszCSHeaderFmt, 9, PrintDouble(fChainage), PrintDouble(fDatum));
					pOutFile->WriteString(strToWrite);
				}
				strToWrite.Format(pszCSRecFmt, 1, PrintDouble(fOffset), PrintDouble(XSecPts[i].z), PrintDouble(XSecPts[i].y), PrintDouble(XSecPts[i].x));
				pOutFile->WriteString(strToWrite);
				if(i == (XSecPts.length() - 1))
				{
					const char* pszCSFooterFmt = "%d\n";

					strToWrite.Format(pszCSFooterFmt, 10);
					pOutFile->WriteString(strToWrite);
				}
			}
		}
		//Calculate distnace from Drawing Axis....
		fOrdDist = (fXAxisLen - fMaxOff + fOffset) * RecParam.dXScale;

		//Call Draw Routines..
		
		{//Draw Ordinate...
			PtStart = ptGrphOrg;
			PtStart.x += fOrdDist;
			PtEnd = PtStart;
			PtEnd.y += (XSecPts[i].z - fDatum) * RecParam.dYScale;
		
			if(bAnnotateThisOrd)
			{
				LINE(PtStart, PtEnd, RecParam.szLyrOrdinate);
			}
			ProfTopPtsArray.append(PtEnd);
			if(0L != pszOrdinateAnn)
			{
				TEXTLEFT(PtEnd, pszOrdinateAnn, DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
			}
		}
		//Elev 
		PtStart = ptGrphOrg;
		PtStart.x  += fOrdDist;
		PtStart.y  -= (0.5 * RecParam.dRowHt);
		if(bAnnotateThisOrd)
		{
			TEXTMID(PtStart, RTOS(XSecPts[i].z), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
			if(RTNORM != ads_entlast (entLast))
			{
				ads_printf("\nERROR: Fatal error!");
				return;
			}
			if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszElevTag, XSecPts[i].z))
			{
				ads_printf("\nERROR: Fatal error!");
				return;
			}
		}
		//Offset
		PtStart = ptGrphOrg;
		PtStart.x  += fOrdDist;
		PtStart.y  -= (1.5 * RecParam.dRowHt);
		if(bAnnotateThisOrd)
		{
			TEXTMID(PtStart, RTOS(fOffset), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
			if(RTNORM != ads_entlast (entLast))
			{
				ads_printf("\nERROR: Fatal error!");
				return;
			}
		}
		if(RecParam.bDrawNorthingEastingAnnSection)
		{
			//Northing 
			PtStart = ptGrphOrg;
			PtStart.x  += fOrdDist;
			PtStart.y  -= (2.5 * RecParam.dRowHt);
			if(bAnnotateThisOrd)
			{
				TEXTMID(PtStart, RTOS(XSecPts[i].y), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
				if(RTNORM != ads_entlast (entLast))
				{
					ads_printf("\nERROR: Fatal error!");
					return;
				}
				if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszNorthTag, XSecPts[i].y))
				{
					ads_printf("\nERROR: Fatal error!");
					return;
				}
			}
			//Easting 
			PtStart = ptGrphOrg;
			PtStart.x  += fOrdDist;
			PtStart.y  -= (3.5 * RecParam.dRowHt);
			if(bAnnotateThisOrd)
			{
				TEXTMID(PtStart, RTOS(XSecPts[i].x), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
				if(RTNORM != ads_entlast (entLast))
				{
					ads_printf("\nERROR: Fatal error!");
					return;
				}
				if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszEastTag, XSecPts[i].x))
				{
					ads_printf("\nERROR: Fatal error!");
					return;
				}
			}
		}

		//Draw Ticks
		for(j = 0; ((j <= iNumOfRows) && (bAnnotateThisOrd)); j++)
		{
			if(iNumOfRows == j)
			{
				PtStart = ptGrphOrg;
				PtStart.x  += fOrdDist;
				PtStart.y  -= ((j * RecParam.dRowHt)  + (RecParam.dNumAnnSize * 0.0));

				PtEnd = PtStart;
				PtEnd.y  += (RecParam.dNumAnnSize * 1.0);
			}
			else
			{
				switch(j)
				{
				case 0:
					PtStart = ptGrphOrg;
					PtStart.x  += fOrdDist;
					PtStart.y  -= ((j * RecParam.dRowHt)  + (RecParam.dNumAnnSize * 0.0));

					PtEnd = PtStart;
					PtEnd.y  -= (RecParam.dNumAnnSize * 1.0);
					break;
				default:
					PtStart = ptGrphOrg;
					PtStart.x  += fOrdDist;
					PtStart.y  -= ((j * RecParam.dRowHt) - (RecParam.dNumAnnSize * 0.5));

					PtEnd = PtStart;
					PtEnd.y  -= (RecParam.dNumAnnSize * 1.0);
					break;
				}
			}
			LINE(PtStart, PtEnd, RecParam.szLyrAnnotation);
		}
		fLastAnnOffset = fOffset;
	}
	POLYLINE(ProfTopPtsArray, RecParam.szLyrTopSurface, FALSE);
	if(strlen(RecParam.szXistTopSurfAnnBlk) > 0)
	{
		CXSecStn::AnnotateTopSurface(ProfTopPtsArray, RecParam.dXistTopSurfAnnBlkScale, (RecParam.dXistTopSurfAnnBlkSpaceMin * RecParam.dXScale), RecParam.szXistTopSurfAnnBlk, RecParam.szLyrTopSurface);
	}
}
BOOL CRdXSecFrDWG::GetDatumValue(const AcGePoint3dArray& DatumRangeArray, const double& fChainage, double& fDatum)
{
	int i;
	BOOL bFound;

	if(DatumRangeArray.length() <= 0)
		return FALSE;

	for(i = 0, bFound = FALSE; i < DatumRangeArray.length(); i++)
	{
		if((fChainage < DatumRangeArray[i].y) && (fChainage >= DatumRangeArray[i].x))
		{
			fDatum = DatumRangeArray[i].z;
			bFound = TRUE;
			return TRUE;
		}
	}
	if(fChainage > DatumRangeArray[DatumRangeArray.length() - 1].y)
	{//Invalid datum range array.......
		fDatum = DatumRangeArray[DatumRangeArray.length() - 1].z;
		return TRUE;
	}
	return FALSE;
}
int CRdXSecFrDWG::FindMaxLenOfDatumStr(const AcGePoint3dArray& DatumRangeArray)
{
	int i, iMaxDatumDigits;

	for(i = iMaxDatumDigits = 0; i < DatumRangeArray.length(); i++)
	{
		if((int)strlen(RTOS(DatumRangeArray[i].z)) > iMaxDatumDigits)
			iMaxDatumDigits = strlen(RTOS(DatumRangeArray[i].z));
	}
	return iMaxDatumDigits;
}
BOOL CRdXSecFrDWG::FindMinMaxOffElevInStn(const AcGePoint3dArray& Array, const AcGePoint3d& ptMidOfXSec, double& fMinOff, double& fMaxOff, double& fMinElev, double& fMaxElev)
{
	int i;
	double fDist;
	BOOL bLeft;

	fMinOff = fMaxOff = fMinElev = fMaxElev = 0.0;

	for(i = 0, bLeft = TRUE; i < Array.length(); i++)
	{
		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encountered !\n");
			return FALSE;
		}
		if(i == 0)
		{
			fMaxElev = Array[i].z;
			fMinElev = Array[i].z;
			if(!CalculateOffset(Array[i], Array, ptMidOfXSec, fMinOff))
				return FALSE;
			fMaxOff = fMinOff;
		}
		else
		{
			if(Array[i].z > fMaxElev)
				fMaxElev = Array[i].z;
			if(Array[i].z < fMinElev)
				fMinElev = Array[i].z;
			if(!CalculateOffset(Array[i], Array, ptMidOfXSec, fDist))
				return FALSE;
			if(fDist > fMaxOff)
				fMaxOff = fDist;
			if(fDist < fMinOff)
				fMinOff = fDist;
		}
	}
	return TRUE;
}
BOOL CRdXSecFrDWG::CalculateOffset(const AcGePoint3d& ptParam, const AcGePoint3dArray& Array, const AcGePoint3d& ptMidOfXSec, double& fOffset)
{
	int i;
	BOOL bLeft;

	for(i = 0, fOffset = 0, bLeft = TRUE; i < Array.length(); i++)
	{
		if((ptMidOfXSec == Array[i]) && bLeft)
		{
			bLeft = FALSE;
			if(ptParam == ptMidOfXSec)
			{
				fOffset = 0.0f;
				return TRUE;
			}
		}
		else
		{
			if(ptParam == Array[i])
			{
				fOffset = Dist2D(ptMidOfXSec, Array[i]);
				if(bLeft)
				{
					fOffset = 0.0 - fOffset;
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}
void CRdXSecFrDWG::DrawAxis(const AcGePoint3d& ptLLCOfExtent, const RDXSECOFFELEVDATA& RecParam, const double& fDatum, const double& fMinOff, const double& fMaxOff, const double& fLeftOfXAxisWid, AcGePoint3d& ptGraphOrg)
{
	int i, iCurRowNo;
	double	dTotalXAxisLen;
	const double fTitleXtn = RecParam.dRowAnnSize * 1.5 * 1.5;
	AcGePoint3d PtStart, PtEnd;

	dTotalXAxisLen = RecParam.dXScale * (fabs(fMinOff) + fMaxOff) + fLeftOfXAxisWid;

	const int iNumOfRows = (RecParam.bDrawNorthingEastingAnnSection) ? 4:2;
	//Draw Axis..Horz Lines
	for(i = 0; i <= iNumOfRows; i++)
	{
		if((i == 0) || (i == iNumOfRows))
		{
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * RecParam.dRowHt);	PtStart.z	= 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * RecParam.dRowHt);	PtEnd.z		= 0.0;
			PtStart.y += fTitleXtn;
			PtEnd.y += fTitleXtn;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			if(i == iNumOfRows)
			{
				PtStart.x	= ptLLCOfExtent.x;					PtStart.y  = ptLLCOfExtent.y	+ (i * RecParam.dRowHt) - RecParam.dNumAnnSize;	PtStart.z	= 0.0;
				PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y	+ (i * RecParam.dRowHt) - RecParam.dNumAnnSize;	PtEnd.z		= 0.0;
				PtStart.y += fTitleXtn;
				PtEnd.y += fTitleXtn;
				LINE(PtStart, PtEnd, RecParam.szLyrAxis);
				continue;
			}
		}
		switch(i)
		{
		case 0:
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * RecParam.dRowHt) + RecParam.dNumAnnSize;	PtStart.z	= 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * RecParam.dRowHt) + RecParam.dNumAnnSize;	PtEnd.z		= 0.0;
			PtStart.y += fTitleXtn;
			PtEnd.y += fTitleXtn;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		default:
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * RecParam.dRowHt) - (RecParam.dNumAnnSize / 2.0);	PtStart.z = 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * RecParam.dRowHt) - (RecParam.dNumAnnSize / 2.0);	PtEnd.z = 0.0;
			PtStart.y += fTitleXtn;
			PtEnd.y += fTitleXtn;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			PtStart[X]	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * RecParam.dRowHt) + (RecParam.dNumAnnSize / 2.0);	PtStart.z = 0.0;
			PtEnd[X]	= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * RecParam.dRowHt) + (RecParam.dNumAnnSize / 2.0);	PtEnd.z = 0.0;
			PtStart.y += fTitleXtn;
			PtEnd.y += fTitleXtn;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		}

	}
	//Draw Axis..Vert Lines
	PtStart.x	= ptLLCOfExtent.x;	PtStart.y	= ptLLCOfExtent.y + (0.0		* RecParam.dRowHt);	PtStart.z = 0.0;
	PtEnd.x		= ptLLCOfExtent.x;	PtEnd.y		= ptLLCOfExtent.y + (iNumOfRows * RecParam.dRowHt);	PtEnd.z = 0.0;
	PtStart.y += fTitleXtn;
	PtEnd.y += fTitleXtn;
	LINE(PtStart, PtEnd, RecParam.szLyrAxis);

	iCurRowNo = 0;
	if(RecParam.bDrawNorthingEastingAnnSection)
	{
		//Easting
		PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ ((iCurRowNo + 0.5) * RecParam.dRowHt);	PtStart.z = 0.0;
		PtStart.y += fTitleXtn;
		TEXTLEFT(PtStart, RecParam.szEastingTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
		//Northing
		iCurRowNo++;
		PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ ((iCurRowNo + 0.5) * RecParam.dRowHt);	PtStart.z = 0.0;
		PtStart.y += fTitleXtn;
		TEXTLEFT(PtStart, RecParam.szNorthingTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
		//
		iCurRowNo++;
	}

	//Text Labels...
	//Offset 
	PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ ((iCurRowNo + 0.5) * RecParam.dRowHt);	PtStart.z = 0.0;
	PtStart.y += fTitleXtn;
	TEXTLEFT(PtStart, RecParam.szOffsetTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	//
	iCurRowNo++;
	//Elev 
	PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ ((iCurRowNo + 0.5) * RecParam.dRowHt);	PtStart.z = 0.0;
	PtStart.y += fTitleXtn;
	TEXTLEFT(PtStart, RecParam.szElevTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	
	{//Datum 
		CString strTmp;
		
		PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (iNumOfRows * RecParam.dRowHt) + (1.25 * RecParam.dRowAnnSize);	PtStart.z = 0.0;
		
		PtStart.y += fTitleXtn;
		strTmp.Format("%s %s %s", RecParam.szDatumTagPrefix, RTOS(fDatum), RecParam.szDatumTagSuffix);
		TEXTLEFT(PtStart, strTmp, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	}
	ptGraphOrg = ptLLCOfExtent;
	ptGraphOrg.x += fLeftOfXAxisWid;
	ptGraphOrg.y += (iNumOfRows * RecParam.dRowHt);
	ptGraphOrg.y += fTitleXtn;
}
BOOL CRdXSecFrDWG::IsCSDwgPossible(const AcGePoint3d& ptLROfCS, const double& fCSWid, const double& fCSHt, const AcGePoint3d& ptULOfNPlate, const double& fNPWid, const double& fNPHt)
{
	AcGePoint3d ptLLCOfNPlate;

	ptLLCOfNPlate.x = ptULOfNPlate.x;
	ptLLCOfNPlate.y = ptULOfNPlate.y - fNPHt;
	ptLLCOfNPlate.z = 0.0;
	{//Chk whether any vertex of CS Dwg extent lies in the NPlate Rectangle or not....
		AcGePoint3d ptTmp;

		//LLC
		ptTmp = ptLROfCS;
		ptTmp.x = ptLROfCS.x - fCSWid;
		if(IsPointWithinRect(ptTmp, ptLLCOfNPlate, fNPWid, fNPHt))
			return FALSE;
		//LRC
		ptTmp = ptLROfCS;
		if(IsPointWithinRect(ptTmp, ptLLCOfNPlate, fNPWid, fNPHt))
			return FALSE;
		//URC
		ptTmp = ptLROfCS;
		ptTmp.y = ptLROfCS.y + fCSHt;
		if(IsPointWithinRect(ptTmp, ptLLCOfNPlate, fNPWid, fNPHt))
			return FALSE;
		//ULC
		ptTmp = ptLROfCS;
		ptTmp.y = ptLROfCS.y + fCSHt;
		ptTmp.x = ptLROfCS.x - fCSWid;
		if(IsPointWithinRect(ptTmp, ptLLCOfNPlate, fNPWid, fNPHt))
			return FALSE;
	}
	{//Chk whether any vertex of NPlate lies in the CS Dwg extent Rectangle or not....
		AcGePoint3d ptTmp, ptLLCOfCS;

		ptLLCOfCS = ptLROfCS;
		ptLLCOfCS.x = ptLROfCS.x - fCSWid;

		//LLC
		ptTmp = ptLLCOfNPlate;
		if(IsPointWithinRect(ptTmp, ptLLCOfCS, fCSWid, fCSHt))
			return FALSE;
		//LRC
		ptTmp = ptLLCOfNPlate;
		ptTmp.x += fNPWid;
		if(IsPointWithinRect(ptTmp, ptLLCOfCS, fCSWid, fCSHt))
			return FALSE;
		//URC
		ptTmp = ptLLCOfNPlate;
		ptTmp.x += fNPWid;
		ptTmp.y += fNPHt;
		if(IsPointWithinRect(ptTmp, ptLLCOfCS, fCSWid, fCSHt))
			return FALSE;
		//ULC
		ptTmp = ptLLCOfNPlate;
		ptTmp.y += fNPHt;
		if(IsPointWithinRect(ptTmp, ptLLCOfCS, fCSWid, fCSHt))
			return FALSE;
	}
	return TRUE;
}
BOOL CRdXSecFrDWG::IsPointWithinRect(const AcGePoint3d& ptParam, const AcGePoint3d& ptLLCOfRect, const double& fWid, const double& fHt)
{
	BOOL bFlag;
	
	bFlag = (ptParam.x >= ptLLCOfRect.x) && (ptParam.x <= ptLLCOfRect.x + fWid);
	bFlag = bFlag && (ptParam.y >= ptLLCOfRect.y) && (ptParam.y <= ptLLCOfRect.y + fHt);
	return bFlag;
}
void CRdXSecFrDWG::RectangleUL(const AcGePoint3d& ptUpperLeft, const double& fWid, const double& fHt, const char* pszLyr)
{
	AcGePoint3d Pt01, Pt02, Pt03, Pt04;
	AcGePoint3dArray ptArrayTmp;

	//Upper Left
	Pt01 = ptUpperLeft;
	//Upper Right
	Pt02 = Pt01;
	Pt02.x = Pt01.x + fWid;
	//Lower Right
	Pt03 = Pt02;
	Pt03.y = Pt02.y - fHt;
	//Lower Left
	Pt04 = Pt03;
	Pt04.x = Pt03.x - fWid;

	ptArrayTmp.append(Pt01);
	ptArrayTmp.append(Pt02);
	ptArrayTmp.append(Pt03);
	ptArrayTmp.append(Pt04);
	ptArrayTmp.append(Pt01);

	POLYLINE(ptArrayTmp, pszLyr, TRUE);
}
void CRdXSecFrDWG::RectangleLR(const AcGePoint3d& ptLowerRight, const double& fWid, const double& fHt, const char* pszLyr)
{
	AcGePoint3d ptUpperLeft;

	//Upper Left
	ptUpperLeft = ptLowerRight;
	ptUpperLeft.x = ptLowerRight.x - fWid;
	ptUpperLeft.y = ptLowerRight.y + fHt;
	
	RectangleUL(ptUpperLeft, fWid, fHt, pszLyr);
}
///////
BOOL CRdXSecFrDWG::GetNextSectionRange(const int& iStartIndex, const double& fCurRowDepth, const AcDbVoidPtrArray& Array, const AcGePoint3dArray& PIDArray, const AcGePoint3dArray& DatumRangeArray, const RDXSECOFFELEVDATA& RecParam, int& iEndIndex, double& fLeftOfXAxisWid, double& fMaxHeight)
{
	const double fTitleXtn = RecParam.dCSLabelAnnSize * 1.5;
	const double fTxtWidMF = 1.0;
	const double fCell2CellDist = RecParam.dNumAnnSize * 4.0;
	int i, iMaxLenOfTagStrs, iMaxDatumDigits;
	double fBotOfXAxisLen, fCSWid, fCSHt, fCumWid, fMaxElevIncOrdAnnTxtTmp, fMinOff, fMaxOff, fMinElev, fMaxElev, fCurShtWid, fPrevMaxHt;
	AcGePoint3d ptMidOfXSec;
	double fChainage, fDatum;

	if(iStartIndex >= Array.length())
		return FALSE;
	
	iMaxDatumDigits = FindMaxLenOfDatumStr(DatumRangeArray);

	iMaxLenOfTagStrs = __max (strlen(RecParam.szElevTag),		strlen(RecParam.szOffsetTag));
	iMaxLenOfTagStrs = __max (strlen(RecParam.szNorthingTag),	strlen(RecParam.szOffsetTag));
	iMaxLenOfTagStrs = __max (strlen(RecParam.szEastingTag),	strlen(RecParam.szOffsetTag));
	iMaxLenOfTagStrs = __max ((int)(strlen(RecParam.szDatumTagPrefix) + iMaxDatumDigits + (int)strlen(RecParam.szDatumTagSuffix)), iMaxLenOfTagStrs);

	fLeftOfXAxisWid = iMaxLenOfTagStrs * RecParam.dRowAnnSize * fTxtWidMF;
	fBotOfXAxisLen = (RecParam.dCSLabelAnnSize * 1.5) + RecParam.dRowHt * ((RecParam.bDrawNorthingEastingAnnSection) ? 4.0:2.0) + fTitleXtn;
	

	fCumWid = fPrevMaxHt = fMaxHeight = 0.0;
	for(i = iStartIndex, iEndIndex = iStartIndex; i < Array.length(); i++)
	{
		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encountered\n");
			return FALSE;
		}
		if(!GetChainage((AcGePoint3dArray*)(Array[i]), PIDArray, fChainage, &ptMidOfXSec))
			return FALSE;

		if(!FindMinMaxOffElevInStn(*((AcGePoint3dArray*)(Array[i])), ptMidOfXSec, fMinOff, fMaxOff, fMinElev, fMaxElev))
			return FALSE;
		if(DatumRangeArray.length() > 0)
		{
			if(!GetDatumValue(DatumRangeArray, fChainage, fDatum))
				return FALSE;
		}
		else
		{//
			fDatum =  CalculateAutoDatum(fMinElev, RecParam.dYScale, RecParam.fMinOrdinateHt, RecParam.fDatumDenominator);
		}


		fCSWid = (fabs(fMinOff) + fMaxOff) * RecParam.dXScale + fLeftOfXAxisWid + fCell2CellDist;
		fCumWid += fCSWid;
		
		fMaxElevIncOrdAnnTxtTmp = (fMaxElev - fDatum) * RecParam.dYScale;
		if(fMaxElevIncOrdAnnTxtTmp >= fMaxHeight)
		{
			fPrevMaxHt = fMaxHeight;
			fMaxHeight = fMaxElevIncOrdAnnTxtTmp;
		}

		fCSHt = fMaxHeight + fBotOfXAxisLen + fCell2CellDist;
		
		//Check sheet width & height ......
		if(fCSWid >= RecParam.dSheetWid)
		{
			ads_printf("\nERROR: Failed to draw cross section\nPossible cause: Sheet width is too small");
			return FALSE;
		}
		if(fCSHt >= RecParam.dSheetHeight)
		{
			ads_printf("\nERROR: Failed to draw cross section\nPossible cause: Sheet height is too small");
			return FALSE;
		}

		//
		if((RecParam.dSheetHeight - (fCurRowDepth + fCSHt)) <= RecParam.dNPlateHeight)
		{
			fCurShtWid = RecParam.dSheetWid - RecParam.dNPlateWid;
		}
		else
		{
			fCurShtWid = RecParam.dSheetWid;
		}
		if((RecParam.dSheetHeight - (fCurRowDepth + fCSHt)) < 0.0)
		{//Out of Range
			iEndIndex = i - 1;
			if((fMaxElevIncOrdAnnTxtTmp == fMaxHeight) && (fPrevMaxHt > 0.0))
				fMaxHeight = fPrevMaxHt;
			fMaxHeight += fBotOfXAxisLen;
			fMaxHeight += fCell2CellDist;
			return TRUE;
		}

		if(fCumWid > fCurShtWid)
		{
			iEndIndex = i - 1;
			if((fMaxElevIncOrdAnnTxtTmp == fMaxHeight) && (fPrevMaxHt > 0.0))
				fMaxHeight = fPrevMaxHt;
			fMaxHeight += fBotOfXAxisLen;
			fMaxHeight += fCell2CellDist;
			return TRUE;
		}
		else
		{
			if(fCumWid == fCurShtWid)
			{
				iEndIndex = i;
				fMaxHeight += fBotOfXAxisLen;
				fMaxHeight += fCell2CellDist;
				return TRUE;
			}
		}
	}
	iEndIndex = Array.length() - 1;
	fMaxHeight += fBotOfXAxisLen;
	fMaxHeight += fCell2CellDist;
	return TRUE;
}

BOOL CRdXSecFrDWG::DrawCSRow(const AcGePoint3d& ptLLCStart, const int& iStartIndex, const int& iEndIndex, const AcDbVoidPtrArray& Array, const AcGePoint3dArray& PIDArray, const AcGePoint3dArray& DatumRangeArray, const RDXSECOFFELEVDATA& RecParam, const double& fLeftOfXAxisWid, const double& fCSHeight, const CString& strCSGridLyr, const AcGePoint3d& ptULOfNPlate, int& iLastDrawnCS, CStdioFile* pOutFile, CPointAndLyrInfoCache* pObjPointAndLyrInfoCache/* = 0L*/)
{
	const double fTitleXtn = RecParam.dCSLabelAnnSize * 1.5;
	const double fTxtWidMF = 1.0;
	const double fCell2CellDist = RecParam.dNumAnnSize * 4.0;
	int i;
	double fCSWid, fMinOff, fMaxOff, fMinElev, fMaxElev, fDatum, fChainage;
	AcGePoint3d ptGrphOrg, ptLLCOfCS, ptLROfCS, ptMidOfXSec;
	
	if((iStartIndex >= Array.length()) || (iEndIndex >= Array.length()))
		return FALSE;

	//Drawing .....
	for(i = iStartIndex, iLastDrawnCS = iStartIndex, ptLLCOfCS = ptLLCStart; i <= iEndIndex; i++)
	{
		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encountered\n");
			return FALSE;
		}
		if(!GetChainage((AcGePoint3dArray*)(Array[i]), PIDArray, fChainage, &ptMidOfXSec))
			return FALSE;

		if(!FindMinMaxOffElevInStn(*((AcGePoint3dArray*)(Array[i])), ptMidOfXSec, fMinOff, fMaxOff, fMinElev, fMaxElev))
			return FALSE;

		if(DatumRangeArray.length() > 0)
		{
			if(!GetDatumValue(DatumRangeArray, fChainage, fDatum))
				return FALSE;
		}
		else
		{//
			fDatum =  CalculateAutoDatum(fMinElev, RecParam.dYScale, RecParam.fMinOrdinateHt, RecParam.fDatumDenominator);
		}
		//
		fCSWid = (fabs(fMinOff) + fMaxOff) * RecParam.dXScale + fLeftOfXAxisWid + fCell2CellDist;

		ptLROfCS.x = ptLLCOfCS.x;
		ptLROfCS.y = ptLLCOfCS.y;
		ptLROfCS.x += fCSWid;
		
		if(fCSWid >= RecParam.dSheetWid)
		{
			ads_printf("\nERROR: Failed to draw cross section\nPossible cause: Sheet width is too small");
			AfxMessageBox("Failed to draw cross section\nPossible cause: Sheet width is too small", MB_ICONSTOP);
			return FALSE;
		}
		if(fCSHeight >= RecParam.dSheetHeight)
		{
			ads_printf("\nERROR: Failed to draw cross section\nPossible cause: Sheet height is too small");
			AfxMessageBox("Failed to draw cross section\nPossible cause: Sheet height is too small", MB_ICONSTOP);
			return FALSE;
		}
		if(!IsCSDwgPossible(ptLROfCS, fCSWid, fCSHeight, ptULOfNPlate, RecParam.dNPlateWid, RecParam.dNPlateHeight))
		{
			ads_printf("\nFatal error : DrawCSRow\n");
			return TRUE;
		}

		ads_printf("\rPlease wait...Drawing Cross Section at Chainage %f (#%d/%d)", fChainage, i+1, Array.length());
			
		DrawAxis(ptLLCOfCS, RecParam, fDatum, fMinOff, fMaxOff, fLeftOfXAxisWid, ptGrphOrg);
		DrawOrdinates(ptGrphOrg, *((AcGePoint3dArray*)(Array[i])), ptMidOfXSec, RecParam, fMinOff, fMaxOff, fDatum, fChainage, pOutFile, pObjPointAndLyrInfoCache);

		{//XSec Label
			AcGePoint3d ptTmp;
			CString strTmp;
			const double fTitleXtn = RecParam.dCSLabelAnnSize * 1.5;
	
			strTmp.Format("%s %s", RecParam.szXSecLabelTag, RTOS(fChainage + RecParam.dStartChg));

			ptTmp.x = ptLLCOfCS.x;
			ptTmp.y = ptLLCOfCS.y;
			ptTmp.x += fCSWid / 2.0;
			ptTmp.y += (fTitleXtn / 2.0);
			TEXTMID(ptTmp, strTmp, 0.0, RecParam.dCSLabelAnnSize, RecParam.szLyrAxis);
		}
		RectangleLR(AcGePoint3d(ptLLCOfCS.x+fCSWid, ptLLCOfCS.y, 0.0), fCSWid, fCSHeight, strCSGridLyr);
		ptLLCOfCS.x += fCSWid;
		iLastDrawnCS++;
	}
	return TRUE;
}
BOOL CRdXSecFrDWG::WritePIDFile(const AcGePoint3dArray& PIDArray, CStdioFile* pOutFile)
{
	long i;
	CString strToWrite;
	const char* pszPIDataFmt = "%s\t%s\n";
	
	for(i = 0; i < PIDArray.length(); i++)
	{
		const AcGePoint3d& ptTmp = PIDArray[i];
		if(i == 0)
		{
			const char* pszCSHeaderFmt = "**\tNorthing\tEasting\n";

			strToWrite.Format(pszCSHeaderFmt);
			pOutFile->WriteString(strToWrite);
		}
		strToWrite.Format(pszPIDataFmt, PrintDouble(ptTmp.y), PrintDouble(ptTmp.x));
		pOutFile->WriteString(strToWrite);
	}
	return TRUE;
}
BOOL CRdXSecFrDWG::DrawAllSections(const AcGePoint3d& ptULCornerOfSheet, const AcDbVoidPtrArray& Array, const AcGePoint3dArray& PIDArray, const AcGePoint3dArray& DatumRangeArray, const RDXSECOFFELEVDATA& RecParam, CPointAndLyrInfoCache* pObjPointAndLyrInfoCache/* = 0L*/)
{
	const double fSheetShiftFac = 0.1;
	const CString strSheetLyr = "DRAWING_SHEET";
	const CString strCSGridLyr = "CROSS_SEXN_GRID";
	int i, iEndIndex;
	double fSheetOrgX, fCumY, fSheetOrgY, fTotalHeight, fLeftOfXAxisWid;
	AcGePoint3d ptULOfNPlate;
	CStdioFile* pOutFileCSD;
	CStdioFile* pOutFilePID;
	CStdioFile	FileOutCSD, FileOutPID;

	pOutFileCSD = 0L;
	if(strlen(RecParam.szOutFileCSD) > 0)
	{
		if(!FileOutCSD.Open(RecParam.szOutFileCSD, CFile::modeWrite|CFile::modeCreate|CFile::typeText))
		{
			ads_printf("\n\nERROR: Unable to open input file \"%s\"\n", RecParam.szOutFileCSD);
			return FALSE;
		}
		pOutFileCSD = &FileOutCSD;
	}
	pOutFilePID = 0L;
	if(strlen(RecParam.szOutFilePID) > 0)
	{
		if(!FileOutPID.Open(RecParam.szOutFilePID, CFile::modeWrite|CFile::modeCreate|CFile::typeText))
		{
			ads_printf("\n\nERROR: Unable to open input file \"%s\"\n", RecParam.szOutFilePID);
			return FALSE;
		}
		pOutFilePID = &FileOutPID;
		//Write now....
		ads_printf("\nPlease wait...Writing PID output file \"%s\"...", RecParam.szOutFilePID);
		WritePIDFile(PIDArray, pOutFilePID);
		ads_printf("Done!");
	}

	LAYER(strCSGridLyr, 1);
	LAYER(strSheetLyr, 2);

	LAYER(RecParam.szLyrAnnotation, 7);
	LAYER(RecParam.szLyrTopSurface, 1);
	LAYER(RecParam.szLyrOrdinate, 3);
	LAYER(RecParam.szLyrAxis, 6);

	fSheetOrgX = ptULCornerOfSheet.x;
	fSheetOrgY = ptULCornerOfSheet.y;

	//Sheet Border
	RectangleUL(AcGePoint3d(fSheetOrgX, fSheetOrgY, 0.0), RecParam.dSheetWid, RecParam.dSheetHeight, strSheetLyr);
	{//Name Plate Border
		ptULOfNPlate.x = fSheetOrgX + RecParam.dSheetWid - RecParam.dNPlateWid;
		ptULOfNPlate.y = fSheetOrgY - RecParam.dSheetHeight + RecParam.dNPlateHeight;
		ptULOfNPlate.z = 0.0;
		RectangleUL(ptULOfNPlate, RecParam.dNPlateWid, RecParam.dNPlateHeight, strSheetLyr);
	}
	ads_printf("\nPlease wait...Drawing Cross Section(s)...\n");
	for(i = 0, fCumY = 0.0; i < Array.length(); )
	{
		int iLastDrawnCS;

		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encountered\nTotal %d# of cross section(s) drawn\n", i);
			return FALSE;
		}

		if(!GetNextSectionRange(i, fCumY, Array, PIDArray, DatumRangeArray, RecParam, iEndIndex, fLeftOfXAxisWid, fTotalHeight))
			return FALSE;
		if(!DrawCSRow(AcGePoint3d(fSheetOrgX, fSheetOrgY - fTotalHeight, 0.0), i, iEndIndex, Array, PIDArray, DatumRangeArray, RecParam, fLeftOfXAxisWid, fTotalHeight, strCSGridLyr, ptULOfNPlate, iLastDrawnCS, pOutFileCSD, pObjPointAndLyrInfoCache))
			return FALSE;
		
		fCumY += fTotalHeight;
		fSheetOrgY -= fTotalHeight;
		if(fCumY >= RecParam.dSheetHeight)
		{
			fCumY = 0.0;
			fSheetOrgX += RecParam.dSheetWid + (RecParam.dSheetWid * fSheetShiftFac);
			fSheetOrgY = ptULCornerOfSheet.y;
			if(i < Array.length())
			{
				//Sheet Border
				RectangleUL(AcGePoint3d(fSheetOrgX, fSheetOrgY, 0.0), RecParam.dSheetWid, RecParam.dSheetHeight, strSheetLyr);
				{//Name Plate Border
					ptULOfNPlate.x = fSheetOrgX + RecParam.dSheetWid - RecParam.dNPlateWid;
					ptULOfNPlate.y = fSheetOrgY - RecParam.dSheetHeight + RecParam.dNPlateHeight;
					ptULOfNPlate.z = 0.0;
					RectangleUL(ptULOfNPlate, RecParam.dNPlateWid, RecParam.dNPlateHeight, strSheetLyr);
				}
			}
		}
		i = iLastDrawnCS;
	}
	ads_printf("\nDone !\n");
	return TRUE;
}

void CRdXSecFrDWG::ShowInvalidCSDataSeg(const AcGePoint3dArray& Array, const CString& strMsg)
{
	CString strMsgExt;
	int i;

	if(Array.length() <= 1)
	{
		ads_printf("\nFATAL ERROR: Not sufficient point(s) present\n");
		return;
	}

	strMsgExt.Format("%s\nDo you want to view the erroneous polyline defined as C/S data ?", strMsg);
	if(IDYES != AfxMessageBox(strMsgExt, MB_ICONQUESTION|MB_YESNO))
		return;
	for(i = 1; i < Array.length();)
	{
		char szDummy[255];
		
		ads_printf("\nCurrent Segment is from (%.3f, %.3f, %.3f) to (%.3f, %.3f, %.3f)", Array[i - 1].x, Array[i - 1].y, Array[i - 1].z, Array[i].x, Array[i].y, Array[i].z);
		CGeneralUtilFuncs::DrawArrowHeadOnDisp(Array[i - 1], Array[i]);
		if(RTNORM != ads_getstring(0, "\nPress 'Space Bar' to view next segment...", szDummy))
		{
			return;
		}
		ads_redraw(0L, 1);
		if(ads_usrbrk() == 1)
		{
			return;
		}
		if(i == (Array.length() - 1))
			i = 1;
		else
			i++;
	}
}
CString CRdXSecFrDWG::PrintDouble(const double fVal)
{
	CString strToRet;

	strToRet.Format("%f", fVal);
	RemoveTrailingZeros(strToRet);
	return strToRet;
}
void CRdXSecFrDWG::RevertPLine()
{
	const char* pszPmt = "\nSelect polyline as to revert :";
	ads_name entName;
	AcGePoint3d pt3D;
	AcGePoint3dArray Array;
	CString strLyr;

	if(RTNORM != ads_entsel(pszPmt, entName, asDblArray(pt3D)))
	{
		ads_printf("\nERROR: No entity selected\n");
		return;
	}
	//Extract vertices...
	if(!XtractVertsFr3DPLine(entName, Array, &strLyr))
	{
		ads_printf("\nERROR: Failed to extract vertices\n");
		return;
	}
	Array.reverse();

	//Create the new entity.....
	POLYLINE(Array, strLyr, TRUE);
	
	//Delete the entity.....
	if(ads_entdel(entName) != RTNORM)
	{
		ads_printf("\nERROR: AutoCAD System error !\n");
		return;//ERROR
	}
	ads_printf("\nPolyline vertex sequence reverted successfully\n");
}
BOOL CRdXSecFrDWG::GetUserSpecXSecRefLines(CPointPair3DPtrArray& Array)
{
	if(Array.GetSize() != 0)
		return 0;
	
	ads_name SelSet;

	{//Get all the Lines....
		struct  resbuf  Rb01;
		const char szEntName[] = "LINE";
		// Entity type
		Rb01.restype = 0;
		Rb01.resval.rstring = (char*)szEntName;
		Rb01.rbnext = 0L; 
		
		if(ads_ssget(0L, 0L, 0L, &Rb01, SelSet) != RTNORM)
		{
			return FALSE;
		}
	}
	{//Entity(s)....
		long i, iNoOfEnt;
		ads_name Entity;
		
		iNoOfEnt = 0;
		if(ads_sslength(SelSet, &iNoOfEnt) != RTNORM)
		{
			ads_ssfree(SelSet);//Free Selset
			return FALSE;//ERROR
		}
		for(i = 0; i < iNoOfEnt; i++)
		{
			if(ads_ssname(SelSet, i, Entity) != RTNORM)
			{
				ads_printf("\nERROR: AutoCAD System error !\n");
				ads_ssfree(SelSet);//Free Selset
				return FALSE;//ERROR
			}
			{//Extract Data...
				AcDbObjectId objId;
				AcDbObject* pObj;
				Acad::ErrorStatus ErrCode;

				ErrCode = acdbGetObjectId(objId, Entity);
				if(ErrCode != Acad::eOk)
				{
					ads_ssfree(SelSet);//Free Selset
					ads_printf("\nERROR: AutoCAD System error !\n");
					return -1;
				}
				ErrCode = acdbOpenObject(pObj, objId, AcDb::kForRead);
				if(ErrCode != Acad::eOk)
				{
					ads_ssfree(SelSet);//Free Selset
					ads_printf("\nERROR: AutoCAD System error !\n");
					return -1;
				}
				if(pObj->isA() != AcDbLine::desc())
				{
					pObj->close();
					ads_ssfree(SelSet);//Free Selset
					ads_printf("\nERROR: AutoCAD System error !\n");
					return -1;
				}
				{//Append to the Array 
					POINTPAIR3D* pRec;
					
					pRec = new POINTPAIR3D;
					
					pRec->pt01 = ((AcDbLine*)pObj)->startPoint();
					pRec->pt01.z = 0.0;
					pRec->pt02 = ((AcDbLine*)pObj)->endPoint();
					pRec->pt02.z = 0.0;
			
					Array.Add(pRec);
				}
				pObj->close();
			}
		}
		return TRUE;
	}
	ads_ssfree(SelSet);//Free Selset
	return TRUE;
}

BOOL CRdXSecFrDWG::CalcChainageOfPropCSD(const AcGePoint3dArray& arrayPID, const POINTPAIR3D* pRecParam, double& fChgParam)
{
	int i;
	AcGePoint3d ptPrev, ptCur, ptResult, ptParamADummy, ptParamBDummy;

	if(arrayPID.length() < 2)
	{
		ads_printf("\nERROR: Invalid #of points defined as PI points\n");
		return FALSE;
	}
	ptParamADummy = pRecParam->pt01;	ptParamADummy.z = 0.0;
	ptParamBDummy = pRecParam->pt02;	ptParamBDummy.z = 0.0;

	ptPrev = arrayPID[0];
	fChgParam = 0.0;
	for(i = 1; i < arrayPID.length(); i++)
	{
		ptCur = arrayPID[i];
		ptPrev = arrayPID[i - 1];
		
		ptPrev.z = 0.0;
		ptCur.z = 0.0;

		if(RTNORM == ads_inters(asDblArray(ptPrev), asDblArray(ptCur), asDblArray(ptParamADummy), asDblArray(ptParamBDummy), 1, asDblArray(ptResult)))
		{//Intersection point found
			fChgParam += Dist2D(ptPrev, ptResult);
			return TRUE;
		}
		fChgParam += Dist2D(ptPrev, ptCur);
	}
	return FALSE;//Not found
}
BOOL CRdXSecFrDWG::TransformAllXSecPoints(const AcGePoint3dArray& PIDArray, const AcDbVoidPtrArray& XSecArray, CPointPair3DPtrArray& arrayRefXSecLines)
{
	{//Create the CSD pairs ....
		CPointPair3DPtrArray arrayRefXSecLinesSorted;
		const double fPrecision = 1E-6;
		int i;

		for(i = 0; i < XSecArray.length(); i++)
		{
			double fChg;
			CString strMsg;
			int k;
			BOOL bMatchFound;

			if(ads_usrbrk() == 1)
			{
				ads_printf("\nUser break encountered !\n");
				return FALSE;
			}
			if(!GetChainage((AcGePoint3dArray*)XSecArray[i], PIDArray, fChg))
			{
				strMsg.Format("ERROR: Failed to calculate chainage for cross section #%d\nPossible cause : No points found on road centre line", i+1);
				ads_printf("\n%s\n", strMsg);
				return FALSE;
			}
			//Find the Nearest proposed CSD data
			for(k = 0, bMatchFound = FALSE; k < arrayRefXSecLines.GetSize();)
			{
				POINTPAIR3D* pRec;
				double fChgThis;
				
				pRec = arrayRefXSecLines[k];
				if(CalcChainageOfPropCSD(PIDArray, pRec, fChgThis))
				{
					if(fabs(fChgThis - fChg) <= fPrecision)
					{
						arrayRefXSecLinesSorted.Add(pRec);
						arrayRefXSecLines.RemoveAt(k);
						bMatchFound = TRUE;
						#ifdef _DEBUG
							ads_printf("\nReference line found for CS data # %d!", i);
						#endif// _DEBUG
						bMatchFound = TRUE;
						break;
					}
					k++;//Increase the counter 
				}
				else
				{//Chainage calculation failed!
					POINTPAIR3D* pRecToRemove;
					
					pRecToRemove = arrayRefXSecLines.GetAt(k);
					arrayRefXSecLines.RemoveAt(k);
					delete pRecToRemove;
					//NOTE: Don't Increase the counter 
				}
			}
			if(!bMatchFound)
			{
				POINTPAIR3D* pRecNew;
				
				pRecNew = new POINTPAIR3D;
				arrayRefXSecLinesSorted.Add(pRecNew);
				#ifdef _DEBUG
				#endif// _DEBUG
				ads_printf("\nReference line NOT FOUND for CS data # %d!, the CS Data drawn in layer \"%s\"", i, m_pszLyrDebug);
				LAYER(m_pszLyrDebug, 1);
				POLYLINE(*((AcGePoint3dArray*)(XSecArray[i])), m_pszLyrDebug, TRUE);
			}
		}
		//Deallocate ....the reference lines which  has no association.....
		#ifdef _DEBUG
		ads_printf("\nTotal %d # of reference lines without CS data found!", arrayRefXSecLines.GetSize());
		#endif// _DEBUG
		for(i = 0; i < arrayRefXSecLines.GetSize(); i++)
		{
			POINTPAIR3D* pRec;
			pRec = arrayRefXSecLines[i];
			delete pRec;
		}
		if(arrayRefXSecLines.GetSize() > 0)
		{
			arrayRefXSecLines.RemoveAll();
		}
		arrayRefXSecLines.Append(arrayRefXSecLinesSorted);
	}
	if(arrayRefXSecLines.GetSize() != XSecArray.length())
	{
		ads_printf("\nFATAL ERROR: Control should not reach here!! - TransformAllXSecPoints()");
		return FALSE;
	}
	{//Transform....
		int i;

		for(i = 0; i < XSecArray.length(); i++)
		{
			AcGePoint3dArray* pArrayCSD;
			POINTPAIR3D* pRec;

			if(ads_usrbrk() == 1)
			{
				ads_printf("\nUser break encountered !\n");
				return FALSE;
			}
			pArrayCSD = (AcGePoint3dArray*)XSecArray[i];
			pRec = arrayRefXSecLines[i];
			if(!TransformXSecData(*pArrayCSD, pRec->pt01, pRec->pt02))
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

BOOL CRdXSecFrDWG::TransformXSecData(AcGePoint3dArray& arrayCSD, const AcGePoint3d& ptStartOfLine, const AcGePoint3d& ptEndOfLine)
{
	AcGePoint3dArray arrayResult;
	int i;

	if((ptStartOfLine == ptEndOfLine) && (ptStartOfLine.x == 0.0) && (ptStartOfLine.y == 0.0))
	{
		return TRUE;
	}
	for(i = 0; i < arrayCSD.length(); i++)
	{
		AcGePoint3d ptResult;

		if(!FindPerpPointOnLine(arrayCSD[i], ptStartOfLine, ptEndOfLine, ptResult))
		{
			return FALSE;
		}
		ptResult.z = arrayCSD[i].z;
		arrayResult.append(ptResult);
	}
	if(arrayResult.length() != arrayCSD.length())
	{
		ads_printf("\nFATAL ERROR: Control should not reach here!! - TransformXSecData()");
		return FALSE;
	}
	arrayCSD.removeSubArray(0, (arrayCSD.length() - 1));
	arrayCSD.append(arrayResult);

	#ifdef _DEBUG
		LAYER("TransformXSecData", 5);
		POLYLINE(arrayCSD, "TransformXSecData", TRUE);
	#endif// _DEBUG
	return TRUE;
}

BOOL CRdXSecFrDWG::GenOrdsInEqInterval(const AcDbVoidPtrArray& XSecArray, const AcGePoint3dArray& PIDArray, const double& fOrdInterval, AcGePoint3dArray* pArrayNewPoints)
{
	int i;

	if(fOrdInterval <= 0.0)
		return TRUE;//generation not required!!

	for(i = 0; i < XSecArray.length(); i++)
	{
		AcGePoint3d ptMidOfXSec;
		double fDistDUMMY;
		CString strMsg;

		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encountered !\n");
			return FALSE;
		}
		if(!GetChainage((AcGePoint3dArray*)XSecArray[i], PIDArray, fDistDUMMY, &ptMidOfXSec))
		{
			strMsg.Format("ERROR: Failed to calculate chainage for cross section #%d\nPossible cause : No points found on road centre line", i+1);
			ads_printf("\n%s\n", strMsg);
			ShowInvalidCSDataSeg(*((AcGePoint3dArray*)XSecArray[i]), strMsg);

			strMsg.Format("Do you want to draw the erroneous polyline defined as C/S data in Layer \"%s\"?", m_pszLyrDebug);
			if(IDYES == AfxMessageBox(strMsg, MB_ICONQUESTION|MB_YESNO))
			{
				LAYER(m_pszLyrDebug, 2);
				POLYLINE(*((AcGePoint3dArray*)XSecArray[i]), m_pszLyrDebug, TRUE);
			}
			return FALSE;
		}
		if(!GenOrdsInEqInterval(*((AcGePoint3dArray*)XSecArray[i]), ptMidOfXSec, fOrdInterval, pArrayNewPoints))
		{
			ads_printf("\nFATAL ERROR: Ordinate generation failed at CSD#%d\n", (i+1));
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CRdXSecFrDWG::GenOrdsInEqInterval(AcGePoint3dArray& arrayCSD, const AcGePoint3d& ptMidOfXSec, const double& fOrdInterval, AcGePoint3dArray* pArrayNewPoints)
{
	int i;
	double fOffsetLeftMost;
	AcGePoint3dArray arrayCSDResult;

#ifdef _DEBUG
	//for(i = 0; i < arrayCSD.length(); i++)
	//{
	//	ads_printf("\n%d [%f, %f, %f]", i, arrayCSD[i].x, arrayCSD[i].y, arrayCSD[i].z);
	//}
#endif//_DEBUG
	if(fOrdInterval <= 0.0)
	{
		return FALSE;
	}
	if(arrayCSD.length() < 2)
	{
		return FALSE;
	}
	if(!CalculateOffset(arrayCSD[0], arrayCSD, ptMidOfXSec, fOffsetLeftMost))
	{
		return FALSE;
	}
	{//Refine Extreme left ordinate offset...
		double fCumDist;

		fCumDist = 0.0;
		while(TRUE)
		{
			if(fCumDist <= fOffsetLeftMost)
			{
				if(fCumDist == fOffsetLeftMost)
					fOffsetLeftMost = fCumDist;
				else
					fOffsetLeftMost = fCumDist + fOrdInterval;
				
				break;
			}
			fCumDist -= fOrdInterval;
		}
	}
	const double fLeftOffsetAngle = Angle2D(ptMidOfXSec, arrayCSD[0]);
	//Generate Ordinates....
	for(i = 1; i < arrayCSD.length(); i++)
	{
		double fOffsetCur, fOffsetPrev;
		AcGePoint2dArray arrayOffElev;

		if(!CalculateOffset(arrayCSD[i - 0], arrayCSD, ptMidOfXSec, fOffsetCur))
		{
			return FALSE;
		}
		if(!CalculateOffset(arrayCSD[i - 1], arrayCSD, ptMidOfXSec, fOffsetPrev))
		{
			return FALSE;
		}
		if(!GetEqIntervalOffsetDist(fOffsetCur, arrayCSD[i - 0].z, fOffsetPrev, arrayCSD[i - 1].z, fOffsetLeftMost, fOrdInterval, arrayOffElev))
		{
			return FALSE;
		}
		{//Insert 
			int j;
			
			arrayCSDResult.append(arrayCSD[i - 1]);
			for(j = 0; j < arrayOffElev.length(); j++)
			{
				const double fOffset = arrayOffElev[j].x;
				const double fElev = arrayOffElev[j].y;
				AcGePoint3d ptResult;

				ads_polar(asDblArray(ptMidOfXSec), fLeftOffsetAngle, fOffset, asDblArray(ptResult));
				ptResult.z = fElev;
				arrayCSDResult.append(ptResult);
				if(0L != pArrayNewPoints)
				{
					pArrayNewPoints->append(ptResult);
				}
			}
			if((i + 1) == arrayCSD.length())
			{
				arrayCSDResult.append(arrayCSD[i]);
			}
		}
	}
	arrayCSD = arrayCSDResult;
#ifdef _DEBUG
	//for(i = 0; i < arrayCSD.length(); i++)
	//{
	//	ads_printf("\n%d [%f, %f, %f]", i, arrayCSD[i].x, arrayCSD[i].y, arrayCSD[i].z);
	//}
#endif//_DBUG
	return TRUE;
}
BOOL CRdXSecFrDWG::GetEqIntervalOffsetDist(const double& fOffsetCur, const double& fElevCur, const double& fOffsetPrev, const double& fElevPrev, const double& fOffsetLeftMost, const double& fOrdInterval, AcGePoint2dArray& arrayResult)
{
	double fCumValue;

	if((arrayResult.length() > 0) || (fOffsetCur < fOffsetPrev))
	{
		ads_printf("\nERROR: Control should never reach here!!! ==> GetEqIntervalOffsetDist()");
		return FALSE;
	}
	//Calculate Start 'fCumValue'...!!
	for(fCumValue = fOffsetLeftMost; ; fCumValue += fOrdInterval)
	{
		if((fCumValue > fOffsetPrev) && (fCumValue < fOffsetCur))
		{
			const double fElev = fElevPrev + (fCumValue - fOffsetPrev) * (fElevCur - fElevPrev) / (fOffsetCur - fOffsetPrev);
			arrayResult.append(AcGePoint2d(fCumValue, fElev));
		}
		if(fCumValue > fOffsetCur)
		{
			return TRUE;
		}
	}
	return TRUE;
}

const char*	CRdXSecFrDWG::CPointAndLyrInfoCache::FindLayer(const AcGePoint3d* pPoint) const
{
	int j;
	
	for(j = 0; j < m_arrayForPoints.length(); j++)
	{
		if(Adesk::kTrue == pPoint->isEqualTo(m_arrayForPoints[j]))
		{
			const CString* pStr = m_arrayLyrNames.GetData() + j;
			return (LPCSTR)(*pStr);
		}
	}
	return 0L;
}


