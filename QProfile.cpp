// QProfile.cpp : implementation file
//
#include "stdafx.h"
#include "Util.h"
#include "DrawUtil.h"
#include "QProfile.h"
#include "XData.h"
#include "XSecFrCSDEx.h"

static void DrawAxis(const RDPFLNEZDATA&, AcGePoint3d&, const double&);
static BOOL CalcNInsertPointAtDist(double, AcGePoint3dArray&, BOOL, const double fPrecision = 1.0E-4);
static BOOL GenPtsInEqInterval(AcGePoint3dArray&, const double&, BOOL);
static int GetAnnStringIndex(const AcGePoint3d&, const AcGePoint3dArray*, const CStringArray*);
static void AnnotateQProfilePoint(const AcGePoint3d&, const double&, const double&, const RDPFLNEZDATA&);

void DrawQProfile(AcGePoint3dArray& arrayPoints, const RDPFLNEZDATA& RecParam, const AcGePoint3dArray* pArrayForOrdAnn /*= 0L*/, const CStringArray* pStrArrayForOrdAnn/*= 0L*/)
{
	const char* pszOutFileFmtHeader = "Cum. Dist\tNorthing\tEasting\tElevation\tLayer\n";
	const char* pszOutFileFmt = "%.3f\t%.3f\t%.3f\t%.3f\t%s\n";
	const char* pszGenLyrName = "GENERATED_POINTS";
	AcGePoint3dArray ProfTopPtsArray;
	AcGePoint3d PtAxisOrg;
	AcGePoint3d PtStart, PtEnd, PtTxtOrg;
	double dCumDist, dDistFrLastOrd, dCumDistOfLastAnnOrd;
	int i, j;
	CString strTmp;
	ads_name entLast;
	FILE* pFileOut;
	
	pFileOut = 0L;
	if(strlen(RecParam.szOutFile) > 0)
	{
		pFileOut = fopen(RecParam.szOutFile, "wt");
		if(0L == pFileOut)
		{
			ads_printf("\nERROR: Failed to open output file \"%s\"", RecParam.szOutFile);
		}
	}

	dCumDist = dDistFrLastOrd = dCumDistOfLastAnnOrd = 0.0;
	for(i = 1; i < arrayPoints.length(); i++)
	{
		if(!RecParam.bConsider3DPts)
			dCumDist += Dist2D(arrayPoints[i - 1], arrayPoints[i]);
		else
			dCumDist += Dist3D(arrayPoints[i - 1], arrayPoints[i]);
	}
	if(RecParam.dOrdInterval > 0.0)
	{
		AcGePoint3dArray arrayPtsGenerated;

		if(RecParam.dOrdInterval > dCumDist)
		{
			ads_printf("\nERROR: Possible cause : Ordinate Interval is too large");
			goto CLEANUP;
		}
		const BOOL bKeepOrgPoints = FALSE;
		if(!GenPtsInEqInterval(arrayPoints, RecParam.dOrdInterval, RecParam.bConsider3DPts))
		{
			ads_printf("\nERROR: Possible cause : Ordinate Interval is too large");
			goto CLEANUP;
		}
	}
	LAYER(RecParam.szLyrAnnotation, 1);
	LAYER(RecParam.szLyrTopSurface, 2);
	LAYER(RecParam.szLyrOrdinate, 3);
	LAYER(RecParam.szLyrAxis, 4);

	//Draw Axis
	DrawAxis(RecParam, PtAxisOrg, dCumDist);

	//Draw Ords
	dCumDist = dDistFrLastOrd = 0.0;
	LAYER(pszGenLyrName, 3);
	for(i = 0; i < arrayPoints.length(); i++)
	{
		const int iAnnStringIndex = GetAnnStringIndex(arrayPoints[i], pArrayForOrdAnn, pStrArrayForOrdAnn);
		if(i > 0)
		{
			if(!RecParam.bConsider3DPts)
			{
				dDistFrLastOrd = Dist2D(arrayPoints[i - 1], arrayPoints[i]);
			}
			else
			{
				dDistFrLastOrd = Dist3D(arrayPoints[i - 1], arrayPoints[i]);
			}
			dCumDist += dDistFrLastOrd;
		}
		const double fPrecision = 1.0E-4;
		const double fRem = fabs((RecParam.dOrdInterval > 0.0) ? fmod(dCumDist, RecParam.dOrdInterval) : 0.0);
		const BOOL bAnnotateThisOrd = (((i + 1) == arrayPoints.length()) || (i == 0) || (RecParam.dOrdInterval <= 0.0) || ((RecParam.dOrdInterval > 0.0) && ((fRem <= fPrecision) || (fabs(fRem - RecParam.dOrdInterval) <= fPrecision))));
		
		//if((fabs(dDistFrLastOrd) <= fPrecision) && (i > 0))
		//	continue;

		//Set X & Y
		PtTxtOrg.x	= PtAxisOrg.x + RecParam.dXScale * dCumDist; PtTxtOrg.z = 0.0;

		//Distance 
		PtTxtOrg.y  = PtAxisOrg.y - (3.5 * RecParam.dRowHt);
		if(bAnnotateThisOrd)
		{
			TEXTMID(PtTxtOrg, RTOS((dCumDist - dCumDistOfLastAnnOrd)), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
			if(RTNORM != ads_entlast (entLast))
			{
				ads_printf("\nERROR: Fatal error!");
				goto CLEANUP;
			}
			//if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszChgTag, dDistFrLastOrd))
			//{
			//	ads_printf("\nERROR: Fatal error!");
			//	return;
			//}
		}
		//Cum Distance 
		PtTxtOrg.y  = PtAxisOrg.y - (4.5 * RecParam.dRowHt);
		if(bAnnotateThisOrd)
		{
			TEXTMID(PtTxtOrg, RTOS(dCumDist), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
			if(RTNORM != ads_entlast (entLast))
			{
				ads_printf("\nERROR: Fatal error!");
				goto CLEANUP;
			}
			if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszChgTag, dCumDist))
			{
				ads_printf("\nERROR: Fatal error!");
				goto CLEANUP;
			}
		}

		//Easting 
		PtTxtOrg.y  = PtAxisOrg.y	- (2.5 * RecParam.dRowHt);
		if(bAnnotateThisOrd)
		{
			TEXTMID(PtTxtOrg, RTOS((arrayPoints[i]).x), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
			if(RTNORM != ads_entlast (entLast))
			{
				ads_printf("\nERROR: Fatal error!");
				goto CLEANUP;
			}
			if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszEastTag, (arrayPoints[i]).x))
			{
				ads_printf("\nERROR: Fatal error!");
				goto CLEANUP;
			}
		}

		//Northing 
		PtTxtOrg.y  = PtAxisOrg.y	- (1.5 * RecParam.dRowHt);
		if(bAnnotateThisOrd)
		{
			TEXTMID(PtTxtOrg, RTOS((arrayPoints[i]).y), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
			if(RTNORM != ads_entlast (entLast))
			{
				ads_printf("\nERROR: Fatal error!");
				goto CLEANUP;
			}
			if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszNorthTag, (arrayPoints[i]).y))
			{
				ads_printf("\nERROR: Fatal error!");
				goto CLEANUP;
			}
		}
	
		//Elev 
		PtTxtOrg.y  = PtAxisOrg.y	- (0.5 * RecParam.dRowHt);
		if(bAnnotateThisOrd)
		{
			TEXTMID(PtTxtOrg, RTOS((arrayPoints[i]).z), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
			if(RTNORM != ads_entlast (entLast))
			{
				ads_printf("\nERROR: Fatal error!");
				goto CLEANUP;
			}
			if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszElevTag, (arrayPoints[i]).z))
			{
				ads_printf("\nERROR: Fatal error!");
				goto CLEANUP;
			}
		}

		{//Ord
			AcGePoint3d PtTmp;

			PtTxtOrg.x	= PtAxisOrg.x + RecParam.dXScale * dCumDist; PtTxtOrg.z = 0.0;
			PtTxtOrg.y  = PtAxisOrg.y	- (0.0 * RecParam.dRowHt);
			
			PtTmp.x = PtTxtOrg.x; PtTmp.y = PtTxtOrg.y; PtTmp.z = PtTxtOrg.z;
			PtTmp.y = PtTxtOrg.y + RecParam.dYScale * ((arrayPoints[i]).z - RecParam.dDatum);
			
			if(bAnnotateThisOrd)
			{
				LINE(PtTxtOrg, PtTmp, RecParam.szLyrOrdinate);
			}
			{//
				AcGePoint3d PtOrdVertex;
				//Add to Vertex Array...
				PtOrdVertex.x = PtAxisOrg.x + RecParam.dXScale * dCumDist;
				PtOrdVertex.y = PtTxtOrg[Y] + RecParam.dYScale * ((arrayPoints[i]).z - RecParam.dDatum);
				PtOrdVertex.z = 0.0;
				
				ProfTopPtsArray.append(PtOrdVertex);
				if((iAnnStringIndex >= 0) && (pArrayForOrdAnn->length() > 0) && (pStrArrayForOrdAnn->GetSize() > 0))
				{
					AcGePoint3d ptResultTmp;

					ads_polar(asDblArray(PtOrdVertex), DTOR(90.0), RecParam.dNumAnnSize, asDblArray(ptResultTmp));
					TEXTLEFT(ptResultTmp, pStrArrayForOrdAnn->GetAt(iAnnStringIndex), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
				}
			}
			//Draw Ticks
			for(j = 0; ((j <= 5) && (bAnnotateThisOrd)); j++)
			{
				switch(j)
				{
				case 0:
					PtTxtOrg.x	= PtAxisOrg.x + RecParam.dXScale * dCumDist;	PtTxtOrg.y	= RecParam.PtOrg.y	+ (j * RecParam.dRowHt) + (RecParam.dNumAnnSize * 0.0);	PtTxtOrg.z = 0.0;
					PtTmp.x	= PtAxisOrg.x + RecParam.dXScale * dCumDist;		PtTmp.y	= RecParam.PtOrg.y	+ (j * RecParam.dRowHt) + (RecParam.dNumAnnSize * 1.0);	PtTmp.z = 0.0;
					break;
				case 5:
					PtTxtOrg.x	= PtAxisOrg.x + RecParam.dXScale * dCumDist;	PtTxtOrg[Y]	= RecParam.PtOrg.y	+ (j * RecParam.dRowHt) - (RecParam.dNumAnnSize * 1.0);	PtTxtOrg.z = 0.0;
					PtTmp.x	= PtAxisOrg.x + RecParam.dXScale * dCumDist;	PtTmp[Y]	= RecParam.PtOrg.y	+ (j * RecParam.dRowHt) + (RecParam.dNumAnnSize * 0.0);	PtTmp.z = 0.0;
					break;
				default:
					PtTxtOrg.x	= PtAxisOrg.x + RecParam.dXScale * dCumDist;	PtTxtOrg[Y]	= RecParam.PtOrg.y	+ (j * RecParam.dRowHt) - (RecParam.dNumAnnSize * 0.5);	PtTxtOrg.z = 0.0;
					PtTmp.x	= PtAxisOrg.x + RecParam.dXScale * dCumDist;	PtTmp[Y]	= RecParam.PtOrg.y	+ (j * RecParam.dRowHt) + (RecParam.dNumAnnSize * 0.5);	PtTmp.z = 0.0;
					break;
				}
				LINE(PtTmp, PtTxtOrg, RecParam.szLyrOrdinate);
			}
		}
		if(bAnnotateThisOrd)
		{
			dCumDistOfLastAnnOrd = dCumDist;
		}
		const BOOL bSpecialLine = ((bAnnotateThisOrd) && (RecParam.dOrdInterval > 0.0) && (i != 0) && ((i + 1) != arrayPoints.length()));
		if(0L != pFileOut)
		{
			if(i == 0)
			{
				//pszOutFileFmtHeader = "Cum. Dist\tNorthing\tEasting\tElevation\n"
				fprintf(pFileOut, pszOutFileFmtHeader);
			}
			//pszOutFileFmt = "%.3f\t%.3f\t%.3f\t%.3f%s\n;
			if((iAnnStringIndex >= 0) && (pArrayForOrdAnn->length() > 0) && (pStrArrayForOrdAnn->GetSize() > 0))
			{
				fprintf(pFileOut, pszOutFileFmt, dCumDist, (arrayPoints[i]).y, (arrayPoints[i]).x, (arrayPoints[i]).z, pStrArrayForOrdAnn->GetAt(iAnnStringIndex));
			}
			else
			{
				const char* pszOtherLyrName = "UNKNOWN_LAYER";
				
				fprintf(pFileOut, pszOutFileFmt, dCumDist, (arrayPoints[i]).y, (arrayPoints[i]).x, (arrayPoints[i]).z, (bSpecialLine) ? pszGenLyrName : pszOtherLyrName);
			}
			if((i + 1) == arrayPoints.length())
			{
				ads_printf("\nOutput file \"%s\" created successfully", RecParam.szOutFile);
			}
		}
		if(RecParam.bDrawGeneratedPoints && bSpecialLine)
		{
			DRAWPOINT(arrayPoints[i], pszGenLyrName);
		}
	}
	POLYLINE(ProfTopPtsArray, RecParam.szLyrTopSurface, FALSE);
	if(strlen(RecParam.szXistTopSurfAnnBlk) > 0)
	{
		CXSecStn::AnnotateTopSurface(ProfTopPtsArray, RecParam.dXistTopSurfAnnBlkScale, (RecParam.dXistTopSurfAnnBlkSpaceMin * RecParam.dXScale), RecParam.szXistTopSurfAnnBlk, RecParam.szLyrTopSurface);
	}
	goto CLEANUP;
	//////////////////////////////////////////////////////////////////////
	CLEANUP :
	{
		if(0L != pFileOut)
		{
			fclose(pFileOut);
		}
	}
}

static void DrawAxis(const RDPFLNEZDATA& RecParam, AcGePoint3d& PtAxisOrg, const double& dXAxisLen)
{
	int i, iMaxLen;
	double	dTotalXAxisLen, dXtraXAxisLen;
	AcGePoint3d PtStart, PtEnd;

	//Find Max lengthy str...
	iMaxLen = __max (__max (__max (strlen(RecParam.szElevTag), strlen(RecParam.szNorthingTag)), __max (strlen(RecParam.szEastingTag), strlen(RecParam.szCumDistTag))), strlen(RecParam.szDistTag));
	
	dXtraXAxisLen = iMaxLen * (RecParam.dRowAnnSize * 0.9) * 1.20;
	dTotalXAxisLen = dXAxisLen * RecParam.dXScale + dXtraXAxisLen;

	PtAxisOrg.x	= RecParam.PtOrg.x + dXtraXAxisLen;	PtAxisOrg.y  = RecParam.PtOrg.y	+ (5.0 * RecParam.dRowHt);	PtAxisOrg.z = 0.0;

	//Draw Axis..Horz Lines
	for(i = 0; i <= 5; i++)
	{
		if((i == 0) || (i == 5))
		{
			PtStart.x	= RecParam.PtOrg.x;						PtStart.y	= RecParam.PtOrg.y	+ (i * RecParam.dRowHt);	PtStart.z	= 0.0;
			PtEnd.x		= RecParam.PtOrg.x + dTotalXAxisLen;	PtEnd.y		= RecParam.PtOrg.y	+ (i * RecParam.dRowHt);	PtEnd.z		= 0.0;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
		}
		switch(i)
		{
		case 0:
			PtStart.x	= RecParam.PtOrg.x;						PtStart.y	= RecParam.PtOrg.y	+ (i * RecParam.dRowHt) + RecParam.dNumAnnSize;	PtStart.z	= 0.0;
			PtEnd.x		= RecParam.PtOrg.x + dTotalXAxisLen;	PtEnd.y		= RecParam.PtOrg.y	+ (i * RecParam.dRowHt) + RecParam.dNumAnnSize;	PtEnd.z		= 0.0;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		case 5:
			PtStart.x	= RecParam.PtOrg.x;						PtStart.y  = RecParam.PtOrg.y	+ (i * RecParam.dRowHt) - RecParam.dNumAnnSize;	PtStart.z	= 0.0;
			PtEnd.x		= RecParam.PtOrg.x + dTotalXAxisLen;	PtEnd.y		= RecParam.PtOrg.y	+ (i * RecParam.dRowHt) - RecParam.dNumAnnSize;	PtEnd.z		= 0.0;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		default:
			PtStart.x	= RecParam.PtOrg.x;						PtStart.y	= RecParam.PtOrg.y	+ (i * RecParam.dRowHt) - (RecParam.dNumAnnSize / 2.0);	PtStart.z = 0.0;
			PtEnd.x		= RecParam.PtOrg.x + dTotalXAxisLen;	PtEnd.y		= RecParam.PtOrg.y	+ (i * RecParam.dRowHt) - (RecParam.dNumAnnSize / 2.0);	PtEnd.z = 0.0;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			PtStart[X]	= RecParam.PtOrg.x;						PtStart.y	= RecParam.PtOrg.y	+ (i * RecParam.dRowHt) + (RecParam.dNumAnnSize / 2.0);	PtStart.z = 0.0;
			PtEnd[X]	= RecParam.PtOrg.x + dTotalXAxisLen;	PtEnd.y		= RecParam.PtOrg.y	+ (i * RecParam.dRowHt) + (RecParam.dNumAnnSize / 2.0);	PtEnd.z = 0.0;
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		}

	}
	//Draw Axis..Vert Lines
	PtStart.x	= RecParam.PtOrg.x;	PtStart.y	= RecParam.PtOrg.y	+ (0.0 * RecParam.dRowHt);	PtStart.z = 0.0;
	PtEnd.x		= RecParam.PtOrg.x;	PtEnd.y		= RecParam.PtOrg.y	+ (5.0 * RecParam.dRowHt);	PtEnd.z = 0.0;
	LINE(PtStart, PtEnd, RecParam.szLyrAxis);

	//Text Labels...
	//Cum Distance 
	PtStart.x	= RecParam.PtOrg.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = RecParam.PtOrg.y	+ (0.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, RecParam.szCumDistTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	//Distance 
	PtStart.x	= RecParam.PtOrg.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = RecParam.PtOrg.y	+ (1.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, RecParam.szDistTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	//Easting 
	PtStart.x	= RecParam.PtOrg.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = RecParam.PtOrg.y	+ (2.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, RecParam.szEastingTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	//Northing 
	PtStart.x	= RecParam.PtOrg.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = RecParam.PtOrg.y	+ (3.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, RecParam.szNorthingTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	//Elev 
	PtStart.x	= RecParam.PtOrg.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = RecParam.PtOrg.y	+ (4.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, RecParam.szElevTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	
	{//Datum 
		CString strTmp;
		
		PtStart.x	= RecParam.PtOrg.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = RecParam.PtOrg.y	+ (5.0 * RecParam.dRowHt) + (1.25 * RecParam.dRowAnnSize);	PtStart.z = 0.0;
		
		strTmp.Format("%s %s", RecParam.szDatumTag, RTOS(RecParam.dDatum));
		TEXTLEFT(PtStart, strTmp, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	}
}
static BOOL GenPtsInEqInterval(AcGePoint3dArray& arrayPoints, const double& fInterval, BOOL bConsider3DPts)
{
	double dCumDist, dCumDistEqInterval;
	int i;

	//Calculate the total distance
	for(i = 1, dCumDist = 0.0; i < arrayPoints.length(); i++)
	{
		if(!bConsider3DPts)
		{
			dCumDist += Dist2D(arrayPoints[i - 1], arrayPoints[i]);
		}
		else
		{
			dCumDist += Dist3D(arrayPoints[i - 1], arrayPoints[i]);
		}
	}
	dCumDistEqInterval = 0.0;
	while(dCumDistEqInterval < dCumDist)
	{
		dCumDistEqInterval += fInterval;
		if(!CalcNInsertPointAtDist(dCumDistEqInterval, arrayPoints, bConsider3DPts))
		{
			break;
		}
	}
	return TRUE;
}
static BOOL CalcNInsertPointAtDist(double dDistParam, AcGePoint3dArray& arrayPoints, BOOL bConsider3DPts, const double fPrecision /*= 1.0E-4*/)
{
	double dCumDist;
	int i;

	//Calculate the total distance
	for(i = 1, dCumDist = 0.0; i < arrayPoints.length(); i++)
	{
		double fDist;

		if(!bConsider3DPts)
		{
			fDist = Dist2D(arrayPoints[i - 1], arrayPoints[i]);
		}
		else
		{
			fDist = Dist3D(arrayPoints[i - 1], arrayPoints[i]);
		}
		dCumDist += fDist;
		//
		if((dCumDist > dDistParam) && ((dCumDist - dDistParam) <= fPrecision))
		{
			return TRUE;//No need to insert 
		}
		if(dCumDist > dDistParam)
		{
			const double fAngle = Angle2D(arrayPoints[i], arrayPoints[i - 1]);
			AcGePoint3d ptResult;

			if(!bConsider3DPts)
			{
				ads_polar(asDblArray(arrayPoints[i]), fAngle, (dCumDist - dDistParam), asDblArray(ptResult));
				ptResult.z = arrayPoints[i].z + ((arrayPoints[i - 1].z - arrayPoints[i].z) * (dCumDist - dDistParam)) / fDist;
			}
			else
			{//Yet to be implemented!!!!
				ads_printf("\nThis facility considering 3D coordinates is YET TO BE IMPLEMENTED!!\n");
				return FALSE;
			}
			arrayPoints.insertAt(i, ptResult);
			return TRUE;
		}
	}
	return FALSE;
}
static int GetAnnStringIndex(const AcGePoint3d& ptParam, const AcGePoint3dArray* pArrayForOrdAnn, const CStringArray* pStrArrayForOrdAnn)
{
	if((0L == pArrayForOrdAnn) || (0L == pStrArrayForOrdAnn))
		return -1;//Do Nothing
	if(pArrayForOrdAnn->length() != pStrArrayForOrdAnn->GetSize())
	{
		ads_printf("\nERROR: Invalid parameter passed TRACE: From >> AnnotateOrdinate()!\n");
		return -1;
	}
	int i;

	for(i = 0; i < pArrayForOrdAnn->length(); i++)
	{
		if(Adesk::kTrue == ptParam.isEqualTo(pArrayForOrdAnn->at(i)))
		{
			return i;
		}
	}
	return -1;
}
//
//
void AnnotateQProfile(const double& fDataumLineY, const AcGePoint3dArray& arrayPickedPoints, const AcGePoint3dArray& arrayPoints, const RDPFLNEZDATA& RecParam)
{
	int i;
	const double fPrec = 1.0E-6;
	AcGePoint3d PtAxisOrg;

	if(arrayPickedPoints.length() > 0)
	{//Calculate Axis Origin
		PtAxisOrg.x = arrayPoints[0].x;
		PtAxisOrg.y = fDataumLineY;
	}
	for(i = 0; i < arrayPickedPoints.length(); i++)
	{
		AcGePoint3dArray arrayLineSegPoints;
		double fElev, fChg;
		
		fChg = (arrayPickedPoints[i].x - arrayPoints[0].x) / RecParam.dXScale;
		if(fChg < 0.0)
		{
			ads_printf("\nERROR: Failed to annotate point #%d", (i+1));
			continue;
		}
		fElev = RecParam.dDatum + (arrayPickedPoints[i].y - fDataumLineY) / RecParam.dYScale;
		//
		AnnotateQProfilePoint(PtAxisOrg, fChg, fElev, RecParam);
	}
}
static void AnnotateQProfilePoint(const AcGePoint3d& PtAxisOrg, const double& fChg, const double& fElev, const RDPFLNEZDATA& RecParam)
{
	const AcGePoint3d ptToAnnotateInPlanVw(0.0, 0.0, fElev);
	AcGePoint3d PtTxtOrg;
	int j;
	ads_name entLast;

	//Set X & Y
	PtTxtOrg.x	= PtAxisOrg.x + RecParam.dXScale * fChg; PtTxtOrg.z = 0.0;

	//Distance from Last Ordinate
	PtTxtOrg.y  = PtAxisOrg.y - (3.5 * RecParam.dRowHt);
	TEXTMID(PtTxtOrg, RTOS(0.0), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
	if(RTNORM != ads_entlast (entLast))
	{
		ads_printf("\nERROR: Fatal error!");
		return;
	}
	//Cum Distance 
	PtTxtOrg.y  = PtAxisOrg.y - (4.5 * RecParam.dRowHt);
	TEXTMID(PtTxtOrg, RTOS(fChg), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
	if(RTNORM != ads_entlast (entLast))
	{
		ads_printf("\nERROR: Fatal error!");
		return;
	}
	if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszChgTag, fChg))
	{
		ads_printf("\nERROR: Fatal error!");
		return;
	}

	//Easting 
	PtTxtOrg.y  = PtAxisOrg.y	- (2.5 * RecParam.dRowHt);
	TEXTMID(PtTxtOrg, RTOS(ptToAnnotateInPlanVw.x), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
	if(RTNORM != ads_entlast (entLast))
	{
		ads_printf("\nERROR: Fatal error!");
		return;
	}
	if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszEastTag, ptToAnnotateInPlanVw.x))
	{
		ads_printf("\nERROR: Fatal error!");
		return;
	}

	//Northing 
	PtTxtOrg.y  = PtAxisOrg.y	- (1.5 * RecParam.dRowHt);
	TEXTMID(PtTxtOrg, RTOS(ptToAnnotateInPlanVw.y), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
	if(RTNORM != ads_entlast (entLast))
	{
		ads_printf("\nERROR: Fatal error!");
		return;
	}
	if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszNorthTag, ptToAnnotateInPlanVw.y))
	{
		ads_printf("\nERROR: Fatal error!");
		return;
	}

	//Elev 
	PtTxtOrg.y  = PtAxisOrg.y	- (0.5 * RecParam.dRowHt);
	TEXTMID(PtTxtOrg, RTOS(fElev), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
	if(RTNORM != ads_entlast (entLast))
	{
		ads_printf("\nERROR: Fatal error!");
		return;
	}
	if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszElevTag, fElev))
	{
		ads_printf("\nERROR: Fatal error!");
		return;
	}

	{//Ord
		AcGePoint3d PtTmp;

		PtTxtOrg.x	= PtAxisOrg.x + RecParam.dXScale * fChg; PtTxtOrg.z = 0.0;
		PtTxtOrg.y  = PtAxisOrg.y	- (0.0 * RecParam.dRowHt);
		
		PtTmp.x = PtTxtOrg.x; PtTmp.y = PtTxtOrg.y; PtTmp.z = PtTxtOrg.z;
		PtTmp.y = PtTxtOrg.y + RecParam.dYScale * (fElev - RecParam.dDatum);
		
		LINE(PtTxtOrg, PtTmp, RecParam.szLyrOrdinate);
		//Draw Ticks
		for(j = 0; j <= 5; j++)
		{
			switch(j)
			{
			case 0:
				PtTxtOrg.x	= PtAxisOrg.x + RecParam.dXScale * fChg;	PtTxtOrg.y	= RecParam.PtOrg.y	+ (j * RecParam.dRowHt) + (RecParam.dNumAnnSize * 0.0);	PtTxtOrg.z = 0.0;
				PtTmp.x	= PtAxisOrg.x + RecParam.dXScale * fChg;		PtTmp.y	= RecParam.PtOrg.y	+ (j * RecParam.dRowHt) + (RecParam.dNumAnnSize * 1.0);	PtTmp.z = 0.0;
				break;
			case 5:
				PtTxtOrg.x	= PtAxisOrg.x + RecParam.dXScale * fChg;	PtTxtOrg[Y]	= RecParam.PtOrg.y	+ (j * RecParam.dRowHt) - (RecParam.dNumAnnSize * 1.0);	PtTxtOrg.z = 0.0;
				PtTmp.x	= PtAxisOrg.x + RecParam.dXScale * fChg;		PtTmp[Y]	= RecParam.PtOrg.y	+ (j * RecParam.dRowHt) + (RecParam.dNumAnnSize * 0.0);	PtTmp.z = 0.0;
				break;
			default:
				PtTxtOrg.x	= PtAxisOrg.x + RecParam.dXScale * fChg;	PtTxtOrg[Y]	= RecParam.PtOrg.y	+ (j * RecParam.dRowHt) - (RecParam.dNumAnnSize * 0.5);	PtTxtOrg.z = 0.0;
				PtTmp.x	= PtAxisOrg.x + RecParam.dXScale * fChg;		PtTmp[Y]	= RecParam.PtOrg.y	+ (j * RecParam.dRowHt) + (RecParam.dNumAnnSize * 0.5);	PtTmp.z = 0.0;
				break;
			}
			LINE(PtTmp, PtTxtOrg, RecParam.szLyrOrdinate);
		}
	}
}
