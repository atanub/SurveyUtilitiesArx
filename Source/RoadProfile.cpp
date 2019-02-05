//RoadProfile.cpp
#include "stdafx.h"
#include "Util.h"
#include "DrawUtil.h"
#include "RoadProfile.h"
#include "SurveyData.h"
#include "XData.h"
#include "XSecFrCSDEx.h"

const double CRoadProfile::m_fTxtWidMF = 1.0;
const char CRoadProfile::pszGenLyrName[] = "GENERATED_ORDINATES";

CRoadProfile::CRoadProfile(const BOOL& bConstHasNEZData) : m_bConstHasNEZData(bConstHasNEZData), m_pDwgParam(0L)
{
	m_dDsgnPflStartElev = 0.0;
	m_pPtArrayExistingDesignProfile = 0L;
	m_pPtLLCParam = 0L;
	m_fStartChgOnExistingDesignProfile = 0.0;
}
CRoadProfile::CRoadProfile(const CRoadProfile&) : m_bConstHasNEZData(FALSE), m_pDwgParam(0L)
{
	ads_printf("\nERROR: Control should NEVER reach here [CRoadProfile::CRoadProfile()] !!");
}
CRoadProfile::~CRoadProfile()
{
	DeallocatePtrArray(m_RPflChgZDataRecs);
	DeallocatePtrArray(m_GradDataRecs);
	DeallocatePtrArray(m_ArrayGradCompact);
	DeallocatePtrArray(m_RPflChgZDataRecsAfterCalc);
	if(0L != m_pPtArrayExistingDesignProfile)
		delete m_pPtArrayExistingDesignProfile;
}
BOOL CRoadProfile::ImportRec(const ROADPROFILE_CHG_Z& Rec)
{
	ROADPROFILE_CHG_Z* pRec;
	
	pRec = new ROADPROFILE_CHG_Z;
	memcpy(pRec, &Rec, sizeof(Rec));
	m_RPflChgZDataRecs.Add(pRec);
	return TRUE;
}
BOOL CRoadProfile::ImportGradRec(const GRADDETAIL& Rec)
{
	GRADDETAIL* pRec;
	
	pRec = new GRADDETAIL;
	memcpy(pRec, &Rec, sizeof(Rec));
	m_GradDataRecs.Add(pRec);
	return TRUE;
}
BOOL CRoadProfile::HasData() const
{
	if((m_RPflChgZDataRecs.GetSize() <= 0) || (m_GradDataRecs.GetSize() <= 0))
	{
		return FALSE;
	}
	return TRUE;
}
BOOL CRoadProfile::IsChgDefinedAsDsgnLevData(const CGradDataRecs& arrayParam, const double& fChg)
{//Insert all the Design Level Transition points
	int i;

	for(i = 0; i < arrayParam.GetSize(); i++)
	{
		const GRADDETAIL* pRec = arrayParam[i];
		if(pRec->dDist == fChg)
		{
			return TRUE;
		}
	}
	return FALSE;
}
double	CRoadProfile::GetStartChainage() const
{
	if(m_RPflChgZDataRecs.GetSize() <= 0)
		return 0.0;
	
	ROADPROFILE_CHG_Z* pRec = m_RPflChgZDataRecs[0];
	return pRec->dChainage;
}

BOOL CRoadProfile::Draw(const RDPFL_CHG_Z_DWG_PARAM& objDwgParam, const AcGePoint3d& ptLLCParam, const AcGePoint3dArray* pPtArrayExistingDesignProfile /* = 0L*/, double* pfStartChgOnExistingDesignProfile /*= 0L*/)
{
	double fWid, fHt;
	AXISDETAILXLSPFL	recAxisDet;
	AcGePoint2d ptGraphOrg;
	int i;
	
	//Set Member variables...
	m_pDwgParam = &objDwgParam;
	m_pPtLLCParam = &ptLLCParam;

	{//Existing Design Profile
		if(0L != m_pPtArrayExistingDesignProfile)
		{
			delete m_pPtArrayExistingDesignProfile;
			m_pPtArrayExistingDesignProfile = 0L;
		}
		if(pPtArrayExistingDesignProfile != 0L)
		{
			m_pPtArrayExistingDesignProfile = new AcGePoint3dArray(32);
			m_pPtArrayExistingDesignProfile->append(*pPtArrayExistingDesignProfile);
		}
		if(0L != pfStartChgOnExistingDesignProfile)
		{
			m_fStartChgOnExistingDesignProfile = *pfStartChgOnExistingDesignProfile;
		}
	}

	_SortByChainage(m_RPflChgZDataRecs);

	DeallocatePtrArray(m_RPflChgZDataRecsAfterCalc);
	if(!_GenOrdinatesAtEqInterval(m_RPflChgZDataRecs, m_RPflChgZDataRecsAfterCalc, objDwgParam.fOrdInterval))
	{
		return FALSE;
	}
	{//Insert all the Design Level Transition points
		for(i = 0; i < m_GradDataRecs.GetSize(); i++)
		{
			const GRADDETAIL* pRec = m_GradDataRecs[i];
			if(!_CalcNInsertPointAtDist(pRec->dDist, m_RPflChgZDataRecsAfterCalc))
			{
				ads_printf("\n\nFATAL ERROR: Failed to calculate Design/Existing elevation at CH:%.3f!", pRec->dDist);
			}
			else
			{
				ads_printf("\n\n TRACE: Successfully calculated Design/Existing elevation at CH:%.3f!", pRec->dDist);
			}
		}
	}
	if(!_CalculateDsgnLevels())
	{
		return FALSE;
	}

	if(!_GetExtents(fWid, fHt, &recAxisDet))
		return FALSE;
	
	LAYER(objDwgParam.szLyrAxis,			1);
	LAYER(objDwgParam.szLyrOrdinate,		2);
	LAYER(objDwgParam.szLyrAnnotation,		3);
	LAYER(objDwgParam.szLyrTopSurfaceOrg,	4);
	LAYER(objDwgParam.szLyrTopSurfaceDsgn,	5);
	LAYER(objDwgParam.szLyrAnnotationEx,	6);
	LAYER(objDwgParam.szLyrTopSurfaceDsgnExisting,	7);//27-Sep-2006

	_DrawAxis(ptGraphOrg, recAxisDet);
	_DrawOrdinates(m_RPflChgZDataRecsAfterCalc, m_ArrayGradCompact, ptGraphOrg, recAxisDet);
/*
	_DrawLabel(objDwgAttribs, ptLLC, recAxisDet);
*/
	return TRUE;
}
double CRoadProfile::_GetMaxElev(const CRPflChgZDataRecs& arrayParam)
{
	double fMaxOrdinateHt;
	int i;

	fMaxOrdinateHt = 0.0;
	for(i = 0; i < arrayParam.GetSize(); i++)
	{
		ROADPROFILE_CHG_Z* pRec;

		pRec = arrayParam[i];
		const double fMaxVal = (pRec->dElevXisting > pRec->dElevDsgn) ? pRec->dElevXisting : pRec->dElevDsgn;
		if(fMaxVal > fMaxOrdinateHt)
			fMaxOrdinateHt = fMaxVal;
	}
	return fMaxOrdinateHt;
}

void CRoadProfile::_DrawOrdinates(const CRPflChgZDataRecs& arrayParam, const CGradDataRecs& arrayCompactGrads, const AcGePoint2d& ptGrphOrg, const AXISDETAILXLSPFL& /*recAxisDet*/)
{
	const double fPrecision = 1.0E-4;
	AcGePoint3dArray arrayTopSurfaceExisting, arrayTopSurfaceDesign, arrayTopSurfaceExistingDesign;
	AcGePoint3d PtStart, PtEnd, PtTxtOrg;
	double fOrdDist;
	int i, j;
	unsigned short iRowIndex;
	ads_name entLast;

	if(arrayParam.GetSize() <= 0)
	{
		return;
	}
	const double fMaxElev = _GetMaxElev(arrayParam);
	const double fMarginOfPflAnnLine = 0.2 * (fMaxElev - m_pDwgParam->dDatum);

	//Draw Ords
	const double fMinChg = (arrayParam[0])->dChainage;
	for(i = 0; i < arrayParam.GetSize(); i++)
	{
		ROADPROFILE_CHG_Z* pRec;
		BOOL bSuppressBaseAnn;

		pRec = arrayParam[i];
		if((i > 0) && (pRec->dChainage < arrayParam[i - 1]->dChainage))
		{
			ads_printf("\nERROR: Fatal error! ->>> CRoadProfile::_DrawOrdinates() -> Chainages are not in Ascending order!! >> i = %d", i);
			assert(0);
			return;
		}
		//
		const BOOL bIsChgDefinedAsDsgnLevData = IsChgDefinedAsDsgnLevData(arrayCompactGrads, pRec->dChainage);
		const double fRem = fabs((m_pDwgParam->fOrdInterval > 0.0) ? fmod(pRec->dChainage, m_pDwgParam->fOrdInterval) : 0.0);
		const BOOL bAnnotateThisOrd = (((i + 1) == arrayParam.GetSize()) || (i == 0) || (m_pDwgParam->fOrdInterval <= 0.0) || bIsChgDefinedAsDsgnLevData || ((m_pDwgParam->fOrdInterval > 0.0) && ((fRem <= fPrecision) || (fabs(fRem - m_pDwgParam->fOrdInterval) <= fPrecision))));
		const int iLastRow = (0L == m_pPtArrayExistingDesignProfile) ? 4 : 6;
		
		if(m_pDwgParam->fOrdInterval > 0.0)
		{
			bSuppressBaseAnn = !bAnnotateThisOrd;
		}
		else
		{
			bSuppressBaseAnn = (pRec->lBit & 8);
		}
		fOrdDist = (pRec->dChainage - fMinChg) * m_pDwgParam->dXScale;

		{//Ordinate Annotation.............
			CMapStringToString	mapRowTagToOrdValue;	//To store the value of an ordinate WRT row tag
			CMapStringToString	mapRowTagToXDataTag;	//To store the value of an ordinate WRT XData tag

			mapRowTagToOrdValue.SetAt(m_pDwgParam->szCutFillTag,			RTOS(pRec->dElevDsgn - pRec->dElevXisting));
			mapRowTagToOrdValue.SetAt(m_pDwgParam->szDsgnElevTag,			RTOS(pRec->dElevDsgn));
			mapRowTagToOrdValue.SetAt(m_pDwgParam->szXElevTag,				RTOS(pRec->dElevXisting));
			mapRowTagToOrdValue.SetAt(m_pDwgParam->szChgTag,				RTOS(pRec->dChainage));
			if(0L != m_pPtArrayExistingDesignProfile)
			{
				mapRowTagToOrdValue.SetAt(m_pDwgParam->szExistingDsgnElevTag,	RTOS(pRec->dElevXistingDsgn));
				mapRowTagToOrdValue.SetAt(m_pDwgParam->szDsgnElevDifferenceTag, RTOS(pRec->dElevXistingDsgn - pRec->dElevDsgn));
			}
			
			mapRowTagToXDataTag.SetAt(m_pDwgParam->szDsgnElevTag, CXDataPflAndCS::m_pszElevTag);
			mapRowTagToXDataTag.SetAt(m_pDwgParam->szXElevTag, CXDataPflAndCS::m_pszElevTag);
			mapRowTagToXDataTag.SetAt(m_pDwgParam->szChgTag, CXDataPflAndCS::m_pszChgTag);

			{//
				POSITION posn; 
				CString strKey, strValue, strXDataTag, strRowIndex;

				posn = mapRowTagToOrdValue.GetStartPosition();
				while(posn != 0L)
				{
					mapRowTagToOrdValue.GetNextAssoc(posn, strKey, strValue);

					if(!m_mapRowTagNIndex.Lookup(strKey, strRowIndex))
						continue;
					
					iRowIndex = (unsigned short)atoi(strRowIndex);
					PtTxtOrg.x = ptGrphOrg.x; PtTxtOrg.y = ptGrphOrg.y;
					PtTxtOrg.x  += fOrdDist;
					PtTxtOrg.y  -= ((iRowIndex - 0.5) * m_pDwgParam->dRowHt);
					if(!bSuppressBaseAnn)
					{
						TEXTMID(PtTxtOrg, strValue, DTOR(90.0), m_pDwgParam->dNumAnnTxtSize, m_pDwgParam->szLyrAnnotation);

						if(mapRowTagToXDataTag.Lookup((LPCSTR)strKey, strXDataTag))
						{
							if(RTNORM != ads_entlast (entLast))
							{
								ads_printf("\nERROR: Fatal error!");
								return;
							}
							if(1 != CXDataPflAndCS::AttachXData(entLast, strXDataTag, atof(strValue)))
							{
								ads_printf("\nERROR: Fatal error!");
								return;
							}
						}
					}
				}
			}
		}
		{//Ordinates & Ticks
			const double fElev =  m_pDwgParam->dYScale * (pRec->dElevXisting - m_pDwgParam->dDatum);//As per request made on 29/06/2003
			const double fElevExistingDesign =  m_pDwgParam->dYScale * (pRec->dElevXistingDsgn - m_pDwgParam->dDatum);//As per request made on 29/06/2003
			PtStart.x = ptGrphOrg.x;
			PtStart.y = ptGrphOrg.y;
			PtStart.x  += fOrdDist;

			PtEnd = PtStart;
			PtEnd.y  += fElev;
			
			if((pRec->lBit & 1) && (!bSuppressBaseAnn))
			{
				if((i == 0) || ((i + 1) == arrayParam.GetSize() || bIsChgDefinedAsDsgnLevData))
				{
					PtEnd = PtStart;
					PtEnd.y  += (m_pDwgParam->dYScale * (pRec->dElevDsgn - m_pDwgParam->dDatum));
				}
				LINE(PtStart, PtEnd, m_pDwgParam->szLyrOrdinate);
			}
			{//Add to Vertex Array...
				PtEnd = PtStart;
				PtEnd.y += fElev;
				arrayTopSurfaceExisting.append(PtEnd);
				
				PtEnd = PtStart;
				PtEnd.y += (pRec->dElevDsgn - m_pDwgParam->dDatum) * m_pDwgParam->dYScale;
				arrayTopSurfaceDesign.append(PtEnd);
			
				if(0L != m_pPtArrayExistingDesignProfile)
				{
					PtEnd = PtStart;
					PtEnd.y += fElevExistingDesign;
					arrayTopSurfaceExistingDesign.append(PtEnd);
				}
			}
			//Text Block Annotation
			//******************************************************
			//*Bit Code Specification for each Line of Station data
			//*	0 = No Text/Block Annotation & Ordinate Line
			//*	1 = Only Draws Ordinate Line
			//*	2 = Only Draws Text Annotation
			//*	4 = Only Inserts the specified Blockname
			//*	8 =	Suppresses basic (N/E/Elev) annotation
			//******************************************************
			if(pRec->lBit & 2)
			{
				PtStart = arrayTopSurfaceExisting[arrayTopSurfaceExisting.length() - 1];
				PtEnd = PtStart;
				PtEnd.y = ptGrphOrg.y + m_pDwgParam->fTxtAnnotationMargin + ((fMaxElev - m_pDwgParam->dDatum) * m_pDwgParam->dYScale);
				LINE(PtStart, PtEnd, m_pDwgParam->szLyrAnnotationEx);

				PtEnd.y += m_pDwgParam->dNumAnnTxtSize;
				TEXTLEFT(PtEnd, pRec->szDesc, DTOR(m_pDwgParam->fTxtAnnotationAngleInDeg), m_pDwgParam->dNumAnnTxtSize, m_pDwgParam->szLyrAnnotationEx);
			}
			if(pRec->lBit & 4)
			{
				PtStart = PtEnd;
				PtStart.y += m_pDwgParam->dNumAnnTxtSize;
				if(!IsBlockExists(pRec->szBlock))
				{
					ads_printf("\nERROR: Block \"%s\" does not exists...Insertion failed\n", pRec->szBlock);
				}
				else
				{
					BLOCK(pRec->szBlock, PtStart, m_pDwgParam->szLyrAnnotation, 0.0, m_pDwgParam->dNumAnnTxtSize, m_pDwgParam->dNumAnnTxtSize);
				}
			}

			if(!bSuppressBaseAnn)
			{//Draw Ticks
				for(j = 0; j <= iLastRow; j++)
				{
					if((j == 0) || (j == iLastRow))
					{//1st & last row....
						if(j == 0)
						{//1st row....
							PtStart.x = ptGrphOrg.x;
							PtStart.y = ptGrphOrg.y;
							PtStart.x  += fOrdDist;
							PtStart.y  -= ((j * m_pDwgParam->dRowHt)  + (m_pDwgParam->dNumAnnTxtSize * 0.0));

							PtEnd = PtStart;
							PtEnd.y  -= (m_pDwgParam->dNumAnnTxtSize * 1.0);
						}
						else
						{//last row....
							PtStart.x = ptGrphOrg.x;
							PtStart.y = ptGrphOrg.y;
							PtStart.x  += fOrdDist;
							PtStart.y  -= ((j * m_pDwgParam->dRowHt)  + (m_pDwgParam->dNumAnnTxtSize * 0.0));

							PtEnd = PtStart;
							PtEnd.y  += (m_pDwgParam->dNumAnnTxtSize * 1.0);
						}
					}
					else
					{//other rows....
						PtStart.x = ptGrphOrg.x;
						PtStart.y = ptGrphOrg.y;
						PtStart.x  += fOrdDist;
						PtStart.y  -= ((j * m_pDwgParam->dRowHt) - (m_pDwgParam->dNumAnnTxtSize * 0.5));

						PtEnd = PtStart;
						PtEnd.y  -= (m_pDwgParam->dNumAnnTxtSize * 1.0);
					}
					LINE(PtStart, PtEnd, m_pDwgParam->szLyrAnnotation);
				}
			}
		}
	}
	{//
		PtStart.x = ptGrphOrg.x;
		PtStart.y = ptGrphOrg.y + m_pDwgParam->fTxtAnnotationMargin + ((fMaxElev - m_pDwgParam->dDatum) * m_pDwgParam->dYScale);
		PtStart.z = 0.0;
		
		PtEnd = arrayTopSurfaceExisting[arrayTopSurfaceExisting.length() - 1];
		PtEnd.y = PtStart.y;

		LINE(PtStart, PtEnd, m_pDwgParam->szLyrAnnotationEx);
	}
	POLYLINE(arrayTopSurfaceExisting, m_pDwgParam->szLyrTopSurfaceOrg, FALSE);
	if(strlen(m_pDwgParam->szXistTopSurfAnnBlk) > 0)
	{
		CXSecStn::AnnotateTopSurface(arrayTopSurfaceExisting, m_pDwgParam->dXistTopSurfAnnBlkScale, (m_pDwgParam->dXistTopSurfAnnBlkSpaceMin * m_pDwgParam->dXScale), m_pDwgParam->szXistTopSurfAnnBlk, m_pDwgParam->szLyrTopSurfaceOrg);
	}
	POLYLINE(arrayTopSurfaceDesign, m_pDwgParam->szLyrTopSurfaceDsgn, FALSE);
	if(0L != m_pPtArrayExistingDesignProfile)
	{
		POLYLINE(arrayTopSurfaceExistingDesign, m_pDwgParam->szLyrTopSurfaceDsgnExisting, FALSE);
	}

	{//for Vertical Alignment Annotation......
		GRADDETAIL* pRecPrev;
		GRADDETAIL* pRecCur;
		CString strGrad, strTmp;

		for(i = 1; i < arrayCompactGrads.GetSize(); i++)
		{
			pRecPrev = arrayCompactGrads[i - 1];
			pRecCur = arrayCompactGrads[i];

			fOrdDist = (pRecPrev->dDist - fMinChg) * m_pDwgParam->dXScale;
			//
			PtStart.x = ptGrphOrg.x; PtStart.y = ptGrphOrg.y;
			PtStart.x  += fOrdDist;
			PtStart.y  += (0.5 * m_pDwgParam->dRowHt);

			fOrdDist = (pRecCur->dDist - fMinChg) * m_pDwgParam->dXScale;
			//
			PtEnd.x = ptGrphOrg.x; PtEnd.y = ptGrphOrg.y;
			PtEnd.x  += fOrdDist;
			PtEnd.y  += (0.5 * m_pDwgParam->dRowHt);

			LINE(PtStart, PtEnd, m_pDwgParam->szLyrAnnotation);
			if(i == 1)
				CIRCLE(PtStart, m_pDwgParam->dNumAnnTxtSize, m_pDwgParam->szLyrAnnotation);
			//
			CIRCLE(PtEnd, m_pDwgParam->dNumAnnTxtSize, m_pDwgParam->szLyrAnnotation);

			fOrdDist = ((pRecPrev->dDist - fMinChg) + ((pRecCur->dDist - pRecPrev->dDist) / 2.0)) * m_pDwgParam->dXScale;
			//
			PtTxtOrg.x = ptGrphOrg.x; PtTxtOrg.y = ptGrphOrg.y;
			PtTxtOrg.x  += fOrdDist;
			PtTxtOrg.y  += (0.5 * m_pDwgParam->dRowHt);
			PtTxtOrg.y  += (0.6 * m_pDwgParam->dNumAnnTxtSize);
			
			if(pRecPrev->dSlope1inValue != 0.0)
			{
				strTmp = RTOS(fabs(pRecPrev->dSlope1inValue));
				RemoveTrailingZeros(strTmp);
				strGrad.Format("%c1 in %s", ((pRecPrev->dSlope1inValue >= 0.0) ? '+':'-'), strTmp);
			}
			else
			{
				strGrad.Format("No Slope", 0);
			}
			TEXTMID(PtTxtOrg, strGrad, 0.0, m_pDwgParam->dNumAnnTxtSize, m_pDwgParam->szLyrAnnotation);
		}
	}
}
void CRoadProfile::_DrawAxis(AcGePoint2d& ptGraphOrg, const AXISDETAILXLSPFL& recAxisDet)
{
	int i;
	double	dTotalXAxisLen;
	AcGePoint3d PtStart, PtEnd, ptLLCOfExtent;
	CString strTmp;
	
	m_mapRowTagNIndex.RemoveAll();
	if((0L == m_pDwgParam) || (0L == m_pPtLLCParam))
	{
		ads_printf("\nERROR: Control should NEVER reach here [CRoadProfile::_DrawAxis()]!!");
		return;
	}

	ptLLCOfExtent = *m_pPtLLCParam;
	ptLLCOfExtent.x += recAxisDet.fMargin;
	ptLLCOfExtent.y += (recAxisDet.fTitleXtn + recAxisDet.fMargin);

	dTotalXAxisLen = recAxisDet.fLeftOfYAxis + recAxisDet.fRiteOfYAxis;

	const int iLastRow = (0L == m_pPtArrayExistingDesignProfile) ? 4 : 6;
	//_Draw Axis..Horz Lines
	for(i = 0; i <= iLastRow; i++)
	{
		if((i == 0) || (i == iLastRow))
		{//1st & last Rows ..................
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * m_pDwgParam->dRowHt);	PtStart.z	= 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * m_pDwgParam->dRowHt);	PtEnd.z		= 0.0;
			
			LINE(PtStart, PtEnd, m_pDwgParam->szLyrAxis);

			if(i == 0)
			{
				PtStart.x	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * m_pDwgParam->dRowHt) + m_pDwgParam->dNumAnnTxtSize;	PtStart.z	= 0.0;
				PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * m_pDwgParam->dRowHt) + m_pDwgParam->dNumAnnTxtSize;	PtEnd.z		= 0.0;

				LINE(PtStart, PtEnd, m_pDwgParam->szLyrAxis);
			}
			else
			{//last Row ..................
				PtStart.x	= ptLLCOfExtent.x;					PtStart.y  = ptLLCOfExtent.y	+ (i * m_pDwgParam->dRowHt) - m_pDwgParam->dNumAnnTxtSize;	PtStart.z	= 0.0;
				PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y	+ (i * m_pDwgParam->dRowHt) - m_pDwgParam->dNumAnnTxtSize;	PtEnd.z		= 0.0;

				LINE(PtStart, PtEnd, m_pDwgParam->szLyrAxis);
			}

		}
		else
		{//Other Rows ..................
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * m_pDwgParam->dRowHt) - (m_pDwgParam->dNumAnnTxtSize / 2.0);	PtStart.z = 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * m_pDwgParam->dRowHt) - (m_pDwgParam->dNumAnnTxtSize / 2.0);	PtEnd.z = 0.0;

			LINE(PtStart, PtEnd, m_pDwgParam->szLyrAxis);
			
			PtStart[X]	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * m_pDwgParam->dRowHt) + (m_pDwgParam->dNumAnnTxtSize / 2.0);	PtStart.z = 0.0;
			PtEnd[X]	= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * m_pDwgParam->dRowHt) + (m_pDwgParam->dNumAnnTxtSize / 2.0);	PtEnd.z = 0.0;

			LINE(PtStart, PtEnd, m_pDwgParam->szLyrAxis);
		}

	}
	//_Draw Axis..Vert Lines
	PtStart.x	= ptLLCOfExtent.x;	PtStart.y	= ptLLCOfExtent.y + (0.0 * m_pDwgParam->dRowHt);	PtStart.z = 0.0;
	PtEnd.x		= ptLLCOfExtent.x;	PtEnd.y		= ptLLCOfExtent.y + (iLastRow * m_pDwgParam->dRowHt);	PtEnd.z = 0.0;

	LINE(PtStart, PtEnd, m_pDwgParam->szLyrAxis);

	//Text Labels...
	if(0L != m_pPtArrayExistingDesignProfile)
	{
		//Elev (Existing Design)
		strTmp.Format("%d", (iLastRow - 5));
		m_mapRowTagNIndex.SetAt(m_pDwgParam->szExistingDsgnElevTag, strTmp);
		PtStart.x	= ptLLCOfExtent.x + (m_pDwgParam->dLabTxtSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (5.5 * m_pDwgParam->dRowHt);	PtStart.z = 0.0;
		TEXTLEFT(PtStart, m_pDwgParam->szExistingDsgnElevTag, 0.0, m_pDwgParam->dLabTxtSize, m_pDwgParam->szLyrAxis);
		//Elev (Design Elev Difference)
		strTmp.Format("%d", (iLastRow - 4));
		m_mapRowTagNIndex.SetAt(m_pDwgParam->szDsgnElevDifferenceTag, strTmp);
		PtStart.x	= ptLLCOfExtent.x + (m_pDwgParam->dLabTxtSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (4.5 * m_pDwgParam->dRowHt);	PtStart.z = 0.0;
		TEXTLEFT(PtStart, m_pDwgParam->szDsgnElevDifferenceTag, 0.0, m_pDwgParam->dLabTxtSize, m_pDwgParam->szLyrAxis);
	}
	//Elev (Proposed)
	strTmp.Format("%d", (iLastRow - 3));
	m_mapRowTagNIndex.SetAt(m_pDwgParam->szCutFillTag, strTmp);
	PtStart.x	= ptLLCOfExtent.x + (m_pDwgParam->dLabTxtSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (3.5 * m_pDwgParam->dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, m_pDwgParam->szCutFillTag, 0.0, m_pDwgParam->dLabTxtSize, m_pDwgParam->szLyrAxis);
	//Elev (Existing)
	strTmp.Format("%d", (iLastRow - 2));
	m_mapRowTagNIndex.SetAt(m_pDwgParam->szDsgnElevTag, strTmp);
	PtStart.x	= ptLLCOfExtent.x + (m_pDwgParam->dLabTxtSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (2.5 * m_pDwgParam->dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, m_pDwgParam->szDsgnElevTag, 0.0, m_pDwgParam->dLabTxtSize, m_pDwgParam->szLyrAxis);
	//Cut/Fill
	strTmp.Format("%d", (iLastRow - 1));
	m_mapRowTagNIndex.SetAt(m_pDwgParam->szXElevTag, strTmp);
	PtStart.x	= ptLLCOfExtent.x + (m_pDwgParam->dLabTxtSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (1.5 * m_pDwgParam->dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, m_pDwgParam->szXElevTag, 0.0, m_pDwgParam->dLabTxtSize, m_pDwgParam->szLyrAxis);
	//Chainage
	strTmp.Format("%d", (iLastRow - 0));
	m_mapRowTagNIndex.SetAt(m_pDwgParam->szChgTag, strTmp);
	PtStart.x	= ptLLCOfExtent.x + (m_pDwgParam->dLabTxtSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (0.5 * m_pDwgParam->dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, m_pDwgParam->szChgTag, 0.0, m_pDwgParam->dLabTxtSize, m_pDwgParam->szLyrAxis);
	
	{//Datum 
		CString strTmp;
		
		PtStart.x	= ptLLCOfExtent.x + (m_pDwgParam->dLabTxtSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (iLastRow * m_pDwgParam->dRowHt) + (1.25 * m_pDwgParam->dLabTxtSize);	PtStart.z = 0.0;
		
		strTmp.Format("%s %s", m_pDwgParam->szDatum, RTOS(m_pDwgParam->dDatum));

		TEXTLEFT(PtStart, strTmp, 0.0, m_pDwgParam->dLabTxtSize, m_pDwgParam->szLyrAxis);
	}
	ptGraphOrg.x = ptLLCOfExtent.x;
	ptGraphOrg.y = ptLLCOfExtent.y;

	ptGraphOrg.x += recAxisDet.fLeftOfYAxis;
	ptGraphOrg.y += (iLastRow * m_pDwgParam->dRowHt);
}
BOOL CRoadProfile::_GetExtents(double& fWid, double& fHt, AXISDETAILXLSPFL* pAxisDetRec)
{
	double fMinCumDist, fMaxCumDist, fMaxElev;

	if(pAxisDetRec == 0L)
	{
		ads_printf("\nERROR: Control should NEVER reach here [CRoadProfile::GetExtents()]!!");
		return FALSE;
	}

	{//Initialize default values which are dependent on 'XLSXSECDWGPARAM'
		pAxisDetRec->fTxtWidMF		= 1.0;
		pAxisDetRec->fTitleXtn		= m_pDwgParam->dRowHt * 2.0;
		pAxisDetRec->fMargin		= m_pDwgParam->dLabTxtSize * 1.0;
		pAxisDetRec->iMaxDatumDigits= 7;
		
		pAxisDetRec->fRiteOfYAxis = 0.0;
	}

	{//Find overall Min/Max Offset, Elev
		double fMaxElevXist, fMaxElevDsgn;
		
		_FindMinMaxCumDistElev(m_RPflChgZDataRecsAfterCalc, fMinCumDist, fMaxCumDist, fMaxElevXist, fMaxElevDsgn);
		fMaxElev = (fMaxElevXist > fMaxElevDsgn) ? fMaxElevXist:fMaxElevDsgn;
		
		pAxisDetRec->fRiteOfYAxis = (fMaxCumDist - fMinCumDist) * m_pDwgParam->dXScale;
	}
	{//Calculate Extents now(Horz)
		int iMaxLenOfTagStrs;

		iMaxLenOfTagStrs = __max (strlen(m_pDwgParam->szXElevTag), strlen(m_pDwgParam->szDsgnElevTag));
		iMaxLenOfTagStrs = __max ((int)strlen(m_pDwgParam->szDatum) + (int)pAxisDetRec->iMaxDatumDigits, iMaxLenOfTagStrs);
		iMaxLenOfTagStrs = __max ((int)strlen(m_pDwgParam->szCutFillTag), iMaxLenOfTagStrs);
		iMaxLenOfTagStrs = __max ((int)strlen(m_pDwgParam->szChgTag), iMaxLenOfTagStrs);
		iMaxLenOfTagStrs = __max ((int)strlen(m_pDwgParam->szDsgnElevDifferenceTag), iMaxLenOfTagStrs);
		iMaxLenOfTagStrs = __max ((int)strlen(m_pDwgParam->szExistingDsgnElevTag), iMaxLenOfTagStrs);

		fWid = 0.0;
		fWid += pAxisDetRec->fRiteOfYAxis;//Space for Ordinates
		fWid += (iMaxLenOfTagStrs * m_pDwgParam->dLabTxtSize * m_fTxtWidMF);// Space for Annoation
	
		fWid += (2.0 * pAxisDetRec->fMargin);
		
		pAxisDetRec->fLeftOfYAxis = (iMaxLenOfTagStrs * m_pDwgParam->dLabTxtSize * pAxisDetRec->fTxtWidMF);// Space for Annoation
	}
	{//Calculate Extents now(Vert)

		fHt = 0.0;
		fHt += ((fMaxElev - m_pDwgParam->dDatum) * m_pDwgParam->dYScale);//Space for Ordinates
		fHt += (m_pDwgParam->dRowHt * 4.0 + pAxisDetRec->fTitleXtn);// Space for Annoation
		
		fHt += (2.0 * pAxisDetRec->fMargin);

		pAxisDetRec->fTopOfXAxis = (fMaxElev - m_pDwgParam->dDatum) * m_pDwgParam->dYScale;//Space for Ordinates
		pAxisDetRec->fBotOfXAxis = (m_pDwgParam->dRowHt * 4.0 + pAxisDetRec->fTitleXtn);// Space for Annoation
	}
	return TRUE;
}
//
//NOTE: Should be called after Sorting/Design Level Calculation
//
BOOL CRoadProfile::_FindMinMaxCumDistElev(const CRPflChgZDataRecs& arrayParam, double& fMinCumDist, double& fMaxCumDist, double& fMaxElevXist, double& fMaxElevDsgn)
{
	int i;
	if(arrayParam.GetSize() <= 0)
		return FALSE;//Not Initialized Properly
	fMinCumDist = (arrayParam[0])->dChainage;
	fMaxCumDist = (arrayParam[arrayParam.GetSize() - 1])->dChainage;

	for(i = 1, fMaxElevXist = (arrayParam[0])->dElevXisting, fMaxElevDsgn = (arrayParam[0])->dElevDsgn; i < arrayParam.GetSize(); i++)
	{
		if((arrayParam[i])->dElevXisting > fMaxElevXist)
			fMaxElevXist = (arrayParam[i])->dElevXisting;
		if((arrayParam[i])->dElevDsgn > fMaxElevDsgn)
			fMaxElevDsgn = (arrayParam[i])->dElevDsgn;
		if((arrayParam[i])->dElevXistingDsgn > fMaxElevDsgn)
			fMaxElevDsgn = (arrayParam[i])->dElevXistingDsgn;
	}
	return TRUE;
}

BOOL CRoadProfile::_CalculateDsgnLevels()
{
	double dStartChg, dDsgnLevOfStartGrad;
	int i;

	if(m_RPflChgZDataRecsAfterCalc.GetSize() < 1)
		return FALSE;

	if(!_GetElevOfStartGradChg(m_dDsgnPflStartElev, (m_RPflChgZDataRecsAfterCalc[0])->dChainage, m_GradDataRecs, dDsgnLevOfStartGrad))
		return FALSE;

	if(!_GetGradElevs(dDsgnLevOfStartGrad, m_GradDataRecs))
		return FALSE;

	for(i = 0; i < m_RPflChgZDataRecsAfterCalc.GetSize(); i++)
	{
		ROADPROFILE_CHG_Z* pRec;

		pRec = m_RPflChgZDataRecsAfterCalc[i];
		if(i == 0)
		{
			dStartChg = pRec->dChainage;
		}
		if(!_GetDsgnLevel(pRec->dChainage, dDsgnLevOfStartGrad, m_GradDataRecs, pRec->dElevDsgn))
		{
			return FALSE;
		}
#ifdef _DEBUG
		ads_printf("\n_DEBUG Design Level >> CH: %f ->> Z: %f", pRec->dChainage, pRec->dElevDsgn);
#endif //_DEBUG

		if(0L != m_pPtArrayExistingDesignProfile)
		{
			if(!_GetExistingDsgnLevel(pRec->dChainage, *m_pPtArrayExistingDesignProfile, pRec->dElevXistingDsgn, &m_fStartChgOnExistingDesignProfile))
			{
				/*
				// 01-Nov-06: Application should consider existing design level as ZERO when the existing design level calculation fails...
				const double fLength = CalculateLength(*m_pPtArrayExistingDesignProfile);
				ads_printf("\nERROR: Failed to calculate existing design level at CH: %f\nPossible cause: The selected existing design profile length(=%f) is not long enough!", pRec->dChainage, fLength);
				return FALSE;
				*/
				ads_printf("\nWarning: Failed to calculate existing design level at CH: %f. Hence considering the Level as ZERO.", pRec->dChainage, pRec->dElevXistingDsgn);
			}
		}
#ifdef _DEBUG
		ads_printf("\n_DEBUG Existing Design Level>> CH: %f ->> Z: %f", pRec->dChainage, pRec->dElevXistingDsgn);
#endif //_DEBUG
	}
	
	{//Gradient-
		double dStartChg, dEndChg;

		dStartChg = (m_RPflChgZDataRecsAfterCalc[0])->dChainage;
		dEndChg = (m_RPflChgZDataRecsAfterCalc[m_RPflChgZDataRecsAfterCalc.GetSize() - 1])->dChainage;

		if(!_GetGradElevsWRTXistPfl(dDsgnLevOfStartGrad, dStartChg, dEndChg, m_GradDataRecs, m_ArrayGradCompact))
		{
			ASSERT(FALSE);
		}
	}
	return TRUE;
}
BOOL CRoadProfile::_GetDsgnLevel(const double& dCumDist, const double& dDsgnLevOfStartGrad, const CGradDataRecs& ArrayGradient, double& dDsgnLev, double* pGrad /*= 0L*/)
{
	int i;
	double dElev;
	GRADDETAIL* pRecStart;
	GRADDETAIL* pRecEnd;
	
	dDsgnLev = dElev = dDsgnLevOfStartGrad;
	for(i = 1, pRecStart = 0L, pRecEnd = 0L; i < ArrayGradient.GetSize(); i++)
	{
		pRecStart = ArrayGradient[i - 1];
		pRecEnd = ArrayGradient[i];
		if((dCumDist >= pRecStart->dDist) && (dCumDist <= pRecEnd->dDist))
		{
			dElev += ((dCumDist - pRecStart->dDist) * SlopeToDistMF(pRecStart->dSlope1inValue));
			dDsgnLev = dElev;
			if(pGrad != 0L)
				*pGrad = pRecStart->dSlope1inValue;
			return TRUE;
		}
		dElev += ((pRecEnd->dDist - pRecStart->dDist) * SlopeToDistMF(pRecStart->dSlope1inValue));
	}
	dElev += ((dCumDist - pRecEnd->dDist) * SlopeToDistMF(pRecEnd->dSlope1inValue));
	dDsgnLev = dElev;
	if(pGrad != 0L)
		*pGrad = pRecEnd->dSlope1inValue;
	return TRUE;
}
BOOL CRoadProfile::_GetExistingDsgnLevel(const double& dCumDistParam, const AcGePoint3dArray& arrayPtsExistingDesignProfile, double& dExistingDsgnLev, const double* pfStartChgOnExistingDesignProfile/* = 0L*/)
{
	int i;
	double dCumDist, dTmp;
	AcGePoint3d ptStart, ptEnd;
	
	dExistingDsgnLev = dCumDist = 0.0;
	dCumDist = (pfStartChgOnExistingDesignProfile == 0L) ? 0.0 : *pfStartChgOnExistingDesignProfile;
	for(i = 1; i < arrayPtsExistingDesignProfile.length(); i++)
	{
		ptStart = arrayPtsExistingDesignProfile[i - 1];
		ptEnd = arrayPtsExistingDesignProfile[i];

		dTmp = Dist2D(ptStart, ptEnd);
		if((dCumDistParam >= dCumDist) && (dCumDistParam <= (dCumDist + dTmp)))
		{
			const double fDeltaDist = (dCumDistParam - dCumDist);
			const double fDeltaElev = (ptEnd.z - ptStart.z);

			dExistingDsgnLev = fDeltaDist * (fDeltaElev / dTmp);
			dExistingDsgnLev += ptStart.z;
			return TRUE;
		}
		dCumDist += dTmp;
	}
	return FALSE;
}
BOOL CRoadProfile::_GetElevOfStartGradChg(const double& dStartElev, const double& dStartChg, const CGradDataRecs& ArrayGradient, double& dDsgnLev)
{
	int i;
	int iStartIndex;
	double dElev;
	GRADDETAIL* pRecStart;
	GRADDETAIL* pRecEnd;
	
	dDsgnLev = dStartElev;
	for(i = 1, iStartIndex = -1, pRecStart = 0L, pRecEnd = 0L; i < ArrayGradient.GetSize(); i++)
	{
		pRecStart = ArrayGradient[i - 1];
		pRecEnd = ArrayGradient[i];
		if((dStartChg >= pRecStart->dDist) && (dStartChg <= pRecEnd->dDist))
		{
			if(pRecStart->dSlope1inValue != 0.0)
				dElev  = dStartElev - ((dStartChg - pRecStart->dDist) * pRecStart->dSlope1inValue) / 100.0;
			else
				dElev  = dStartElev - 0.0;
			iStartIndex = i - 1;
			break;
		}
	}
	if(iStartIndex < 0) 
		return FALSE;
	if(iStartIndex == 0) 
	{
		dDsgnLev = dElev;
		return TRUE;
	}
	
	for(i = iStartIndex - 1; i >= 0; i--)
	{
		pRecStart = ArrayGradient[i];
		if(pRecStart->dSlope1inValue != 0.0)
			dElev  = dElev - (((ArrayGradient[i + 1])->dDist - pRecStart->dDist) * pRecStart->dSlope1inValue) / 100.0;
	}
	dDsgnLev = dElev;
	return TRUE;
}
BOOL CRoadProfile::_GetGradElevs(const double& dDsgnLevOfStartGrad, const CGradDataRecs& ArrayGradient)
{
	int i;
	GRADDETAIL* pRec;
	
	for(i = 0; i < ArrayGradient.GetSize(); i++)
	{
		pRec = ArrayGradient[i];
		if(!_GetDsgnLevel(pRec->dDist, dDsgnLevOfStartGrad, ArrayGradient, pRec->dElev))
			return FALSE;
	}
	return TRUE;
}
BOOL CRoadProfile::_GetGradElevsWRTXistPfl(const double& dDsgnLevOfStartGrad, const double& dStartChg, const double& dEndChg, const CGradDataRecs& ArrayGradient, CGradDataRecs& ArrayGradCompact)
{
	int i;
	double dElev, dGrad;
	GRADDETAIL* pRecNew;
	GRADDETAIL* pRec;
	
	//First Transition.....
	if(!_GetDsgnLevel(dStartChg, dDsgnLevOfStartGrad, ArrayGradient, dElev, &dGrad))
		return FALSE;
	pRecNew= new GRADDETAIL;
	pRecNew->dSlope1inValue = dGrad;
	pRecNew->dDist = dStartChg;
	pRecNew->dElev = dElev;
	ArrayGradCompact.Add(pRecNew);
#ifdef _DEBUG
	ads_printf("\n_DEBUG >> Compacting ....CH: %f, Z: %f, %%: %f", pRecNew->dDist, pRecNew->dElev, pRecNew->dSlope1inValue);
#endif //_DEBUG
	//Others
	for(i = 0, pRec = 0L; i < ArrayGradient.GetSize(); i++)
	{
		pRec = ArrayGradient[i];
		if((pRec->dDist > dStartChg) && (pRec->dDist < dEndChg))
		{
			//Other Transition.....
			pRecNew= new GRADDETAIL;
			*pRecNew = *pRec;
			ArrayGradCompact.Add(pRecNew);
#ifdef _DEBUG
			ads_printf("\n_DEBUG >> Compacting ....CH: %f, Z: %f, %%: %f", pRecNew->dDist, pRecNew->dElev, pRecNew->dSlope1inValue);
#endif //_DEBUG
		}
	}
	//Last Transition.....
	if(!_GetDsgnLevel(dEndChg, dDsgnLevOfStartGrad, ArrayGradient, dElev, &dGrad))
		return FALSE;
	pRecNew= new GRADDETAIL;
	pRecNew->dSlope1inValue = dGrad;
	pRecNew->dDist = dEndChg;
	pRecNew->dElev = dElev;
	ArrayGradCompact.Add(pRecNew);
#ifdef _DEBUG
	ads_printf("\n_DEBUG >> Compacting ....CH: %f, Z: %f, %%: %f", pRecNew->dDist, pRecNew->dElev, pRecNew->dSlope1inValue);
#endif //_DEBUG

	return TRUE;
}

void CRoadProfile::_SortByChainage(CRPflChgZDataRecs& RecArrayParam)
{
	CRPflChgZDataRecs RecArray;
	int iIndex, iOrgLen;
	
	iOrgLen = RecArrayParam.GetSize();
	for(; RecArrayParam.GetSize() > 0;)
	{
		iIndex = _GetMinCumDistIndex(RecArrayParam);
		if(iIndex >= 0)
		{
			ROADPROFILE_CHG_Z* pRec;
			
			pRec = RecArrayParam.GetAt(iIndex);
			RecArrayParam.RemoveAt(iIndex);
			RecArray.Add(pRec);
		}
		else
		{
			ads_printf("\nERROR: Control should NEVER reach here [CRoadProfile::_SortByChainage() Tag=A]\n");
		}
	}
	if(RecArrayParam.GetSize() != 0)
	{
		ads_printf("\nERROR: Control should NEVER reach here [CRoadProfile::_SortByChainage()]\n");
		return;
	}
	RecArrayParam.Append(RecArray);
	if(iOrgLen != RecArrayParam.GetSize())
	{
		ads_printf("\nERROR: Control should NEVER reach here [CRoadProfile::_SortByChainage()]\n");
		return;
	}
}
int CRoadProfile::_GetMinCumDistIndex(const CRPflChgZDataRecs& RecArray)
{
	int i, iIndex;
	
	if(RecArray.GetSize() < 1)
		return -1;//Error
	if(RecArray.GetSize() == 1)
		return 0;

	for(i = 0, iIndex = 0; i < RecArray.GetSize(); i++)
	{
		ROADPROFILE_CHG_Z* pRecMin;
		ROADPROFILE_CHG_Z* pRecCur;

		pRecCur = RecArray[i];
		pRecMin = RecArray[iIndex];
		
		if(pRecCur->dChainage < pRecMin->dChainage)
			iIndex = i;
	}
	return iIndex;
}

BOOL CRoadProfile::_GenOrdinatesAtEqInterval(const CRPflChgZDataRecs& arrayOrgData, CRPflChgZDataRecs& arrayGenData, const double& fInterval)
{
	int i;

	DeallocatePtrArray(arrayGenData);
	if(arrayOrgData.GetSize() <= 0)
	{
		return FALSE;
	}
	//Copy the original records first....
	for(i = 0; i < arrayOrgData.GetSize(); i++)
	{
		const ROADPROFILE_CHG_Z* pRec = arrayOrgData[i];
		ROADPROFILE_CHG_Z* pRecNew;

		pRecNew = new ROADPROFILE_CHG_Z;
		*pRecNew = *pRec;
		
		arrayGenData.Add(pRecNew);
	}
	if(fInterval <= 0.0)
		return TRUE;

	{//
		const double fMaxChg = arrayOrgData[arrayGenData.GetSize() - 1]->dChainage;
		double dCumDist;

		//Determine the start chainage (multiple of interval)...
		dCumDist = 0.0;
		while(dCumDist < fMaxChg)
		{
			if(dCumDist >= arrayGenData[0]->dChainage)
			{
				break;
			}
			dCumDist += fInterval;
		}

		while(dCumDist < fMaxChg)
		{
			if(!_CalcNInsertPointAtDist(dCumDist, arrayGenData))
			{
				ads_printf("\nERROR: Fatal error!");
				break;
			}
			dCumDist += fInterval;
		}
	}

	return TRUE;
}
BOOL CRoadProfile::_CalcNInsertPointAtDist(const double& dDistParam, CRPflChgZDataRecs& arrayRecords, const double fPrecision /*= 1.0E-4*/)
{
	int i;

	for(i = 1; i < arrayRecords.GetSize(); i++)
	{
		const ROADPROFILE_CHG_Z* pRecStart = arrayRecords[i - 1];
		const ROADPROFILE_CHG_Z* pRecEnd = arrayRecords[i];

		if((fabs(pRecStart->dChainage - dDistParam) <= fPrecision) || (fabs(pRecEnd->dChainage - dDistParam) <= fPrecision))
		{
			return TRUE;//No need to insert 
		}
		if((dDistParam > pRecStart->dChainage) && (dDistParam < pRecEnd->dChainage))
		{//Need to insert here....
			ROADPROFILE_CHG_Z* pRecNew;
			
			pRecNew = new ROADPROFILE_CHG_Z;
			memset(pRecNew, 0, sizeof(ROADPROFILE_CHG_Z));
			pRecNew->dChainage = dDistParam;
			pRecNew->dElevXisting = pRecStart->dElevXisting + (dDistParam - pRecStart->dChainage) * (pRecEnd->dElevXisting - pRecStart->dElevXisting) / (pRecEnd->dChainage - pRecStart->dChainage);
			//Text Block Annotation
			//******************************************************
			//*Bit Code Specification for each Line of Station data
			//*	0 = No Text/Block Annotation & Ordinate Line
			//*	1 = Only Draws Ordinate Line
			//*	2 = Only Draws Text Annotation
			//*	4 = Only Inserts the specified Blockname
			//*	8 =	Suppresses basic (N/E/Elev) annotation
			//******************************************************
			pRecNew->lBit = 1;
			strcpy(pRecNew->szBlock, "");
			strcpy(pRecNew->szDesc, "");
			#ifdef _DEBUG
				strcpy(pRecNew->szDesc, pszGenLyrName);
				pRecNew->lBit = 3;//Draws Ordinate Line + Text Annotation
			#endif//_DEBUG

			arrayRecords.InsertAt(i, pRecNew);
			return TRUE;
		}
	}
	return FALSE;
}

