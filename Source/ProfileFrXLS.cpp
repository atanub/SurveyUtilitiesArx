// ProfileFrXLS.cpp : implementation file
//
#include "stdafx.h"
#include "Util.h"
#include "DrawUtil.h"
#include "XSecFrXLS.h"
#include "ProfileFrXLS.h"
#include "XData.h"
#include "ResultDlg.h"
#include "XSecFrCSDEx.h"

//
const double CProfileFrXLS::m_fTxtWidMF = 1.0;

CProfileFrXLS::CProfileFrXLS()
{
	Reset();
}
CProfileFrXLS::~CProfileFrXLS()
{
	Reset();
}
void CProfileFrXLS::Reset()
{
	if(m_RecArray.GetSize() > 0)
	{
		DeallocatePtrArray(m_RecArray);
	}
}
BOOL CProfileFrXLS::AddOrdinate(const XLSPROFILEORDINATE& Rec)
{
	XLSPROFILEORDINATE* pRec;
	
	pRec = new XLSPROFILEORDINATE;
	*pRec = Rec;
	m_RecArray.Add(pRec);
	
	return TRUE;
}

void CProfileFrXLS::DumpVars()
{
	return;
#ifdef _DEBUG
	int i;

	ads_printf("\n\n\nFrom CProfileFrXLS::DumpVars()->>");
	ads_printf("\n[BitCode][Cumulative Distance][Elevation][Northing][Easting][Text][Block]");
	for(i = 0; i < m_RecArray.GetSize(); i++)
	{
		XLSPROFILEORDINATE* pRec;

		pRec = m_RecArray.GetAt(i);

		ads_printf("\n[pRec->lBit][pRec->dCumDist][pRec->dElev][pRec->dNorth][pRec->dNorth][pRec->strTxt][pRec->strBlk]");
	}
	ads_printf("\n\n\n");
#endif //_DEBUG
}
void CProfileFrXLS::_SortByCumDist(CXLSProfileOrdinateArray& RecArrayParam)
{
	CXLSProfileOrdinateArray RecArray;
	int iIndex, iOrgLen;
	
	iOrgLen = RecArrayParam.GetSize();
	for(; RecArrayParam.GetSize() > 0;)
	{
		iIndex = _GetMinCumDistIndex(RecArrayParam);
		if(iIndex >= 0)
		{
			XLSPROFILEORDINATE* pRec;
			
			pRec = RecArrayParam.GetAt(iIndex);
			RecArrayParam.RemoveAt(iIndex);
			RecArray.Add(pRec);
		}
		else
		{
			ads_printf("\nERROR: Control should not reach here [CProfileFrXLS::_SortByCumDist() Tag=A]\n");
		}
	}
	if(RecArrayParam.GetSize() != 0)
	{
		ads_printf("\nERROR: Control should not reach here [CProfileFrXLS::_SortByCumDist()]\n");
		return;
	}
	RecArrayParam.Append(RecArray);
	if(iOrgLen != RecArrayParam.GetSize())
	{
		ads_printf("\nERROR: Control should not reach here [CProfileFrXLS::_SortByCumDist()]\n");
		return;
	}
}
int CProfileFrXLS::_GetMinCumDistIndex(const CXLSProfileOrdinateArray& RecArray)
{
	int i, iIndex;
	
	if(RecArray.GetSize() < 1)
		return -1;//Error
	if(RecArray.GetSize() == 1)
		return 0;

	for(i = 0, iIndex = 0; i < RecArray.GetSize(); i++)
	{
		XLSPROFILEORDINATE* pRecMin;
		XLSPROFILEORDINATE* pRecCur;

		pRecCur = RecArray[i];
		pRecMin = RecArray[iIndex];
		
		if(pRecCur->dCumDist < pRecMin->dCumDist)
			iIndex = i;
	}
	return iIndex;
}
//
//NOTE: Should be called after Sorting
//
double CProfileFrXLS::GetMinCumDist()
{
	if(m_RecArray.GetSize() <= 0)
	{
		ads_printf("\nERROR: Control should not reach here! [CProfileFrXLS::GetMinCumDist()]\n\n");
		return 0.0;
	}
	return (m_RecArray[0])->dCumDist;
}
//
//NOTE: Should be called after Sorting
//
double CProfileFrXLS::GetMaxCumDist()
{
	if(m_RecArray.GetSize() <= 0)
	{
		ads_printf("\nERROR: Control should not reach here! [CProfileFrXLS::GetMaxCumDist()]\n\n");
		return 0.0;
	}
	return (m_RecArray[m_RecArray.GetSize()-1])->dCumDist;
}
//
//NOTE: Should be called after Sorting
//
BOOL CProfileFrXLS::FindMinMaxCumDistElev(double& fMinCumDist, double& fMaxCumDist, double& fMaxElev)
{
	int i;
	if(m_RecArray.GetSize() <= 0)
		return FALSE;//Not Initialized Properly
	fMinCumDist = GetMinCumDist();
	fMaxCumDist = GetMaxCumDist();
	fMaxElev = 0.0;

	for(i = 1, fMaxElev = (m_RecArray[0])->dElev; i < m_RecArray.GetSize(); i++)
	{
		if((m_RecArray[i])->dElev > fMaxElev)
			fMaxElev = (m_RecArray[i])->dElev;
	}
	return TRUE;
}

//
//NOTE: Should be called after Sorting
//
BOOL CProfileFrXLS::GetExtents(const RDPFLNEZDATA& objDwgAttribs, double& fWid, double& fHt, AXISDETAILXLSPFL* pAxisDetRec)
{
	double fMinCumDist, fMaxCumDist, fMaxElev;

	if(pAxisDetRec == 0L)
	{
		ads_printf("\nERROR: Control should not reach here [CProfileFrXLS::GetExtents()]!!");
		return FALSE;
	}
	{//Initialize default values which are dependent to 'XLSXSECDWGPARAM'
		pAxisDetRec->fTxtWidMF		= 1.0;
		pAxisDetRec->fTitleXtn		= objDwgAttribs.dRowHt * 1.5;
		pAxisDetRec->fMargin		= objDwgAttribs.dRowAnnSize * 1.0;
		pAxisDetRec->iMaxDatumDigits= 7;
		
		pAxisDetRec->fRiteOfYAxis = 0.0;
	}

	{//Find overall Min/Max Offset, Elev
		
		this->FindMinMaxCumDistElev(fMinCumDist, fMaxCumDist, fMaxElev);
		
		pAxisDetRec->fRiteOfYAxis = (fMaxCumDist - fMinCumDist) * objDwgAttribs.dXScale;
	}
	{//Calculate Extents now(Horz)
		int iMaxLenOfTagStrs;

		iMaxLenOfTagStrs = __max (strlen(objDwgAttribs.szElevTag), strlen(objDwgAttribs.szDistTag));
		iMaxLenOfTagStrs = __max ((int)strlen(objDwgAttribs.szDatumTag) + (int)pAxisDetRec->iMaxDatumDigits, iMaxLenOfTagStrs);
		iMaxLenOfTagStrs = __max ((int)strlen(objDwgAttribs.szNorthingTag), iMaxLenOfTagStrs);
		iMaxLenOfTagStrs = __max ((int)strlen(objDwgAttribs.szEastingTag), iMaxLenOfTagStrs);

		fWid = 0.0;
		fWid += pAxisDetRec->fRiteOfYAxis;//Space for Ordinates
		fWid += (iMaxLenOfTagStrs * objDwgAttribs.dRowAnnSize * m_fTxtWidMF);// Space for Annoation
	
		fWid += (2.0 * pAxisDetRec->fMargin);
		
		pAxisDetRec->fLeftOfYAxis = (iMaxLenOfTagStrs * objDwgAttribs.dRowAnnSize * pAxisDetRec->fTxtWidMF);// Space for Annoation
	}
	{//Calculate Extents now(Vert)

		fHt = 0.0;
		fHt += ((fMaxElev - objDwgAttribs.dDatum) * objDwgAttribs.dYScale);//Space for Ordinates
		fHt += (objDwgAttribs.dRowHt * 4.0 + pAxisDetRec->fTitleXtn);// Space for Annoation
		
		fHt += (2.0 * pAxisDetRec->fMargin);

		pAxisDetRec->fTopOfXAxis = (fMaxElev - objDwgAttribs.dDatum) * objDwgAttribs.dYScale;//Space for Ordinates
		pAxisDetRec->fBotOfXAxis = (objDwgAttribs.dRowHt * 4.0 + pAxisDetRec->fTitleXtn);// Space for Annoation
	}
	return TRUE;
}
BOOL CProfileFrXLS::Draw(const AcGePoint3d& ptLLC, const RDPFLNEZDATA& objDwgAttribs)
{
	double fWid, fHt;
	AXISDETAILXLSPFL recAxisDet;
	AcGePoint2d ptGraphOrg;

	_SortByCumDist(m_RecArray);

	if(!GetExtents(objDwgAttribs, fWid, fHt, &recAxisDet))
		return FALSE;
	
	LAYER(objDwgAttribs.szLyrAnnotation, 1);
	LAYER(objDwgAttribs.szLyrTopSurface, 2);
	LAYER(objDwgAttribs.szLyrOrdinate, 3);
	LAYER(objDwgAttribs.szLyrAxis, 4);

	_DrawAxis(objDwgAttribs, ptLLC, ptGraphOrg, recAxisDet);
	_DrawOrdinates(objDwgAttribs, ptGraphOrg, recAxisDet);
	_DrawLabel(objDwgAttribs, ptLLC, recAxisDet);
	
	return TRUE;
}
void CProfileFrXLS::_DrawLabel(const RDPFLNEZDATA& RecParam, const AcGePoint3d& ptLLC, const AXISDETAILXLSPFL& recAxisDet)
{
	AcGePoint3d ptIns;

	ptIns = ptLLC;
	ptIns.x += (recAxisDet.fMargin + recAxisDet.fLeftOfYAxis + recAxisDet.fRiteOfYAxis * 0.5);
	ptIns.y += recAxisDet.fMargin;
	ptIns.y += RecParam.dRowAnnSize / 2.0;

	TEXTMID(ptIns, RecParam.szProfileTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
}
double CProfileFrXLS::GetMinElev()
{
	int i;
	double fMinElev;

	if(m_RecArray.GetSize() <= 0)
	{
		ads_printf("\nERROR: Control should not reach here [CProfileFrXLS::CalcArea() Tag=A]\n");
		return -1.0;
	}

	fMinElev = (m_RecArray[0])->dElev;
	for(i = 1; i < m_RecArray.GetSize(); i++)
	{
		if((m_RecArray[i])->dElev < fMinElev)
			fMinElev = m_RecArray[i]->dElev;
	}
	return fMinElev;
}
void CProfileFrXLS::_DrawOrdinates(const RDPFLNEZDATA& RecParam, const AcGePoint2d& ptGrphOrg, const AXISDETAILXLSPFL&/* recAxisDet*/)
{
	AcGePoint3dArray ProfTopPtsArray;
	AcGePoint3d PtStart, PtEnd, PtTxtOrg;
	double fOrdDist, fElev;
	int i, j;
	ads_name entLast;
	XLSPROFILEORDINATE* pRec;
	CString strTmp;

	//Draw Ords
	for(i = 0; i < m_RecArray.GetSize(); i++)
	{
		pRec = m_RecArray[i];
		fOrdDist = pRec->dCumDist * RecParam.dXScale;
		fElev = (pRec->dElev - RecParam.dDatum) * RecParam.dYScale;

		//Elev 
		PtTxtOrg.x = ptGrphOrg.x; PtTxtOrg.y = ptGrphOrg.y;
		PtTxtOrg.x  += fOrdDist;
		PtTxtOrg.y  -= (0.5 * RecParam.dRowHt);
		strTmp = RTOS(pRec->dElev);
		TEXTMID(PtTxtOrg, strTmp, DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
		if(RTNORM != ads_entlast (entLast))
		{
			ads_printf("\nERROR: Fatal error!");
			return;
		}

		if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszElevTag, pRec->dElev))
		{
			ads_printf("\nERROR: Fatal error!");
			return;
		}
		//Northing 
		PtTxtOrg.x = ptGrphOrg.x; PtTxtOrg.y = ptGrphOrg.y;
		PtTxtOrg.x  += fOrdDist;
		PtTxtOrg.y  -= (1.5 * RecParam.dRowHt);
		strTmp = RTOS(pRec->dNorth);
		TEXTMID(PtTxtOrg, strTmp, DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
		if(RTNORM != ads_entlast (entLast))
		{
			ads_printf("\nERROR: Fatal error!");
			return;
		}
		if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszNorthTag, pRec->dNorth))
		{
			ads_printf("\nERROR: Fatal error!");
			return;
		}
		//Easting 
		PtTxtOrg.x = ptGrphOrg.x; PtTxtOrg.y = ptGrphOrg.y;
		PtTxtOrg.x  += fOrdDist;
		PtTxtOrg.y  -= (2.5 * RecParam.dRowHt);
		strTmp = RTOS(pRec->dEast);
		TEXTMID(PtTxtOrg, strTmp, DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
		if(RTNORM != ads_entlast (entLast))
		{
			ads_printf("\nERROR: Fatal error!");
			return;
		}
		if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszEastTag, pRec->dEast))
		{
			ads_printf("\nERROR: Fatal error!");
			return;
		}

		//Cum Dist
		PtTxtOrg.x = ptGrphOrg.x; PtTxtOrg.y = ptGrphOrg.y;
		PtTxtOrg.x  += fOrdDist;
		PtTxtOrg.y  -= (3.5 * RecParam.dRowHt);
		strTmp = RTOS(pRec->dCumDist);
		TEXTMID(PtTxtOrg, strTmp, DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
		if(RTNORM != ads_entlast (entLast))
		{
			ads_printf("\nERROR: Fatal error!");
			return;
		}
		if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszChgTag, pRec->dCumDist))
		{
			ads_printf("\nERROR: Fatal error!");
			return;
		}

		{//Ord
			PtStart.x = ptGrphOrg.x;
			PtStart.y = ptGrphOrg.y;
			PtStart.x  += fOrdDist;

			PtEnd = PtStart;
			PtEnd.y  += fElev;
			
			if(pRec->lBit & 1)
			{
				LINE(PtStart, PtEnd, RecParam.szLyrOrdinate);
			}
			//Add to Vertex Array...
			ProfTopPtsArray.append(PtEnd);
			//Text Block Annotation
			//******************************************************
			//*Bit Code Specification for each Line of Station data
			//*	0 = No Text/Block Annotation & Ordinate Line
			//*	1 = Only Draws Ordinate Line
			//*	2 = Only Draws Text Annotation
			//*	4 = Only Inserts the specified Blockname
			//******************************************************
			if(pRec->lBit & 2)
			{
				PtStart = PtEnd;
				PtStart.y += RecParam.dNumAnnSize;
				TEXTLEFT(PtStart, pRec->strTxt, DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
			}
			if(pRec->lBit & 4)
			{
				PtStart = PtEnd;
				PtStart.y += RecParam.dNumAnnSize;
				if(!IsBlockExists(pRec->strBlk))
				{
					ads_printf("\nERROR: Block \"%s\" does not exists...Insertion failed\n", pRec->strBlk);
				}
				else
				{
					BLOCK(pRec->strBlk, PtStart, RecParam.szLyrAnnotation, 0.0, RecParam.dNumAnnSize, RecParam.dNumAnnSize);
				}
			}

			//Draw Ticks
			for(j = 0; j <= 4; j++)
			{
				switch(j)
				{
				case 0:
					PtStart.x = ptGrphOrg.x;
					PtStart.y = ptGrphOrg.y;
					PtStart.x  += fOrdDist;
					PtStart.y  -= ((j * RecParam.dRowHt)  + (RecParam.dNumAnnSize * 0.0));

					PtEnd = PtStart;
					PtEnd.y  -= (RecParam.dNumAnnSize * 1.0);
					break;
				case 4:
					PtStart.x = ptGrphOrg.x;
					PtStart.y = ptGrphOrg.y;
					PtStart.x  += fOrdDist;
					PtStart.y  -= ((j * RecParam.dRowHt)  + (RecParam.dNumAnnSize * 0.0));

					PtEnd = PtStart;
					PtEnd.y  += (RecParam.dNumAnnSize * 1.0);
					break;
				default:
					PtStart.x = ptGrphOrg.x;
					PtStart.y = ptGrphOrg.y;
					PtStart.x  += fOrdDist;
					PtStart.y  -= ((j * RecParam.dRowHt) - (RecParam.dNumAnnSize * 0.5));

					PtEnd = PtStart;
					PtEnd.y  -= (RecParam.dNumAnnSize * 1.0);

					break;
				}
				LINE(PtStart, PtEnd, RecParam.szLyrAnnotation);
			}
		}
	}
	POLYLINE(ProfTopPtsArray, RecParam.szLyrTopSurface, FALSE);
	if(strlen(RecParam.szXistTopSurfAnnBlk) > 0)
	{
		CXSecStn::AnnotateTopSurface(ProfTopPtsArray, RecParam.dXistTopSurfAnnBlkScale, (RecParam.dXistTopSurfAnnBlkSpaceMin * RecParam.dXScale), RecParam.szXistTopSurfAnnBlk, RecParam.szLyrTopSurface);
	}
}
void CProfileFrXLS::_DrawAxis(const RDPFLNEZDATA& RecParam, const AcGePoint3d& ptLLCParam, AcGePoint2d& ptGraphOrg, const AXISDETAILXLSPFL& recAxisDet)
{
	int i;
	double	dTotalXAxisLen;
	AcGePoint3d PtStart, PtEnd, ptLLCOfExtent;

	ptLLCOfExtent = ptLLCParam;
	ptLLCOfExtent.x += recAxisDet.fMargin;
	ptLLCOfExtent.y += (recAxisDet.fTitleXtn + recAxisDet.fMargin);

	dTotalXAxisLen = recAxisDet.fLeftOfYAxis + recAxisDet.fRiteOfYAxis;

	//Draw Axis..Horz Lines
	for(i = 0; i <= 4; i++)
	{
		if((i == 0) || (i == 4))
		{
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * RecParam.dRowHt);	PtStart.z	= 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * RecParam.dRowHt);	PtEnd.z		= 0.0;
			
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
		}
		switch(i)
		{
		case 0:
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * RecParam.dRowHt) + RecParam.dNumAnnSize;	PtStart.z	= 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * RecParam.dRowHt) + RecParam.dNumAnnSize;	PtEnd.z		= 0.0;

			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		case 4:
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y  = ptLLCOfExtent.y	+ (i * RecParam.dRowHt) - RecParam.dNumAnnSize;	PtStart.z	= 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y	+ (i * RecParam.dRowHt) - RecParam.dNumAnnSize;	PtEnd.z		= 0.0;

			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		default:
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * RecParam.dRowHt) - (RecParam.dNumAnnSize / 2.0);	PtStart.z = 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * RecParam.dRowHt) - (RecParam.dNumAnnSize / 2.0);	PtEnd.z = 0.0;

			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			
			PtStart[X]	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * RecParam.dRowHt) + (RecParam.dNumAnnSize / 2.0);	PtStart.z = 0.0;
			PtEnd[X]	= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * RecParam.dRowHt) + (RecParam.dNumAnnSize / 2.0);	PtEnd.z = 0.0;

			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		}

	}
	//Draw Axis..Vert Lines
	PtStart.x	= ptLLCOfExtent.x;	PtStart.y	= ptLLCOfExtent.y + (0.0 * RecParam.dRowHt);	PtStart.z = 0.0;
	PtEnd.x		= ptLLCOfExtent.x;	PtEnd.y		= ptLLCOfExtent.y + (4.0 * RecParam.dRowHt);	PtEnd.z = 0.0;

	LINE(PtStart, PtEnd, RecParam.szLyrAxis);

	//Text Labels...
	//Elev (Field)
	PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (3.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, RecParam.szElevTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	//Northing
	PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (2.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, RecParam.szNorthingTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	//Easting 
	PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (1.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, RecParam.szEastingTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	//Cumulative Distance
	PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (0.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, RecParam.szDistTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	
	{//Datum 
		CString strTmp;
		
		PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (4.0 * RecParam.dRowHt) + (1.25 * RecParam.dRowAnnSize);	PtStart.z = 0.0;
		
		strTmp.Format("%s %s", RecParam.szDatumTag, RTOS(RecParam.dDatum));

		TEXTLEFT(PtStart, strTmp, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	}
	ptGraphOrg.x = ptLLCOfExtent.x;
	ptGraphOrg.y = ptLLCOfExtent.y;

	ptGraphOrg.x += recAxisDet.fLeftOfYAxis;
	ptGraphOrg.y += (4.0 * RecParam.dRowHt);
}

