// XSecFrCSDEx.cpp : implementation file
//
#include "stdafx.h"
#include <dbents.h>
#include <Dbsymtb.h>
#include <Math.h>
#include "Util.h"
#include "DrawUtil.h"
#include "XSecFrXLS.h"
#include "XData.h"
#include "ResultDlg.h"

//
const double CXSecFrXLS::m_fTxtWidMF = 1.0;

CXSecFrXLS::CXSecFrXLS()
{
	Reset();
}
CXSecFrXLS::~CXSecFrXLS()
{
	Reset();
}
void CXSecFrXLS::Reset()
{
	if(m_RecArray.GetSize() > 0)
	{
		DeallocatePtrArray(m_RecArray);
	}
	m_fChg = 0.0;
	m_fDatum = 0.0;
}
BOOL CXSecFrXLS::AddOrdinate(const NEZXSECORDINATE& Rec)
{
	NEZXSECORDINATE* pRec;
	
	pRec = new NEZXSECORDINATE;
	*pRec = Rec;
	m_RecArray.Add(pRec);
	
	return TRUE;
}

void CXSecFrXLS::DumpVars()
{
	return;
#ifdef _DEBUG
	int i;

	ads_printf("\nFrom CXSecFrXLS::DumpVars()->>");
	ads_printf("\nChg = %f,  Datum = %f [Total %d# of data]", m_fChg, m_fDatum, m_RecArray.GetSize());
	for(i = 0; i < m_RecArray.GetSize(); i++)
	{
		NEZXSECORDINATE* pRec;

		pRec = m_RecArray.GetAt(i);

		ads_printf("\nBitCode = %d, dOffset = %f, dElev = %f, strTxt = %s, strBlk = %s", pRec->lBit, pRec->dOffset, pRec->dElev, pRec->strTxt, pRec->strBlk);
	}
	ads_printf("\n\n\n");
#endif //_DEBUG
}
void CXSecFrXLS::_SortByOffset(CNEZXSecOrdinateArray& RecArrayParam)
{
	CNEZXSecOrdinateArray RecArray;
	int iIndex, iOrgLen;
	
	iOrgLen = RecArrayParam.GetSize();
	for(; RecArrayParam.GetSize() > 0;)
	{
		iIndex = _GetMinOffsetDistIndex(RecArrayParam);
		if(iIndex >= 0)
		{
			NEZXSECORDINATE* pRec;
			
			pRec = RecArrayParam.GetAt(iIndex);
			RecArrayParam.RemoveAt(iIndex);
			RecArray.Add(pRec);
		}
		else
		{
			ads_printf("\nERROR: Control should not reach here [CXSecFrXLS::_SortByOffset() Tag=A]\n");
		}
	}
	if(RecArrayParam.GetSize() != 0)
	{
		ads_printf("\nERROR: Control should not reach here [CXSecFrXLS::_SortByOffset()]\n");
		return;
	}
	RecArrayParam.Append(RecArray);
	if(iOrgLen != RecArrayParam.GetSize())
	{
		ads_printf("\nERROR: Control should not reach here [CXSecFrXLS::_SortByOffset()]\n");
		return;
	}
}
int CXSecFrXLS::_GetMinOffsetDistIndex(const CNEZXSecOrdinateArray& RecArray)
{
	int i, iIndex;
	
	if(RecArray.GetSize() < 1)
		return -1;//Error
	if(RecArray.GetSize() == 1)
		return 0;

	for(i = 0, iIndex = 0; i < RecArray.GetSize(); i++)
	{
		NEZXSECORDINATE* pRecMin;
		NEZXSECORDINATE* pRecCur;

		pRecCur = RecArray[i];
		pRecMin = RecArray[iIndex];
		
		if(pRecCur->dOffset < pRecMin->dOffset)
			iIndex = i;
	}
	return iIndex;
}
double CXSecFrXLS::GetMinOffset()
{
	if(m_RecArray.GetSize() <= 0)
	{
		ads_printf("\nERROR: Control should not reach here! [CXSecFrXLS::GetMinOffset()]\n\n");
		return 0.0;
	}
	return (m_RecArray[0])->dOffset;
}
double CXSecFrXLS::GetMaxOffset()
{
	if(m_RecArray.GetSize() <= 0)
	{
		ads_printf("\nERROR: Control should not reach here! [CXSecFrXLS::GetMaxOffset()]\n\n");
		return 0.0;
	}
	return (m_RecArray[m_RecArray.GetSize()-1])->dOffset;
}
BOOL CXSecFrXLS::FindMinMaxOffElev(double& fMinOff, double& fMaxOff, double& fMaxElev)
{
	int i;
	if(m_RecArray.GetSize() <= 0)
		return FALSE;//Not Initialized Properly
	fMinOff = GetMinOffset();
	fMaxOff = GetMaxOffset();
	fMaxElev = 0.0;

	for(i = 1, fMaxElev = (m_RecArray[0])->dElev; i < m_RecArray.GetSize(); i++)
	{
		if((m_RecArray[i])->dElev > fMaxElev)
			fMaxElev = (m_RecArray[i])->dElev;
	}

	return TRUE;
}
BOOL CXSecFrXLS::GetExtents(const XLSXSECDWGPARAM& objDwgAttribs, double& fWid, double& fHt, AXISDETAIL* pAxisDetRec)
{
	double fMinOff, fMaxOff, fMaxElev;

	if(pAxisDetRec == 0L)
	{
		ads_printf("\nERROR: Control should not reach here [CXSecFrXLS::GetExtents()]!!");
		return FALSE;
	}
	{//Initialize default values which are dependent to 'XLSXSECDWGPARAM'
		pAxisDetRec->fTxtWidMF		= 1.0;
		pAxisDetRec->fTitleXtn		= objDwgAttribs.dCSLabelAnnSize * 2.25;
		pAxisDetRec->fMargin		= objDwgAttribs.dRowAnnSize * 2.0;
		pAxisDetRec->iMaxDatumDigits= 7;
		
		pAxisDetRec->fRiteOfYAxisA = 0.0;
		pAxisDetRec->fRiteOfYAxisB = 0.0;
	}

	{//Find overall Min/Max Offset, Elev
		
		this->FindMinMaxOffElev(fMinOff, fMaxOff, fMaxElev);
		
		pAxisDetRec->fRiteOfYAxisA = (fabs(fMinOff) * objDwgAttribs.dXScale);//Space for Ordinates (Left of Graph Org)
		pAxisDetRec->fRiteOfYAxisB = (fMaxOff * objDwgAttribs.dXScale);//Space for Ordinates (Rite of Graph Org)
	}
	{//Calculate Extents now(Horz)
		int iMaxLenOfTagStrs;

		iMaxLenOfTagStrs = __max (strlen(objDwgAttribs.szElevTag), strlen(objDwgAttribs.szOffsetTag));
		iMaxLenOfTagStrs = __max ((int)strlen(objDwgAttribs.szDatumTag) + (int)pAxisDetRec->iMaxDatumDigits, iMaxLenOfTagStrs);

		fWid = 0.0;
		fWid += (pAxisDetRec->fRiteOfYAxisA + pAxisDetRec->fRiteOfYAxisB);//Space for Ordinates
		fWid += (iMaxLenOfTagStrs * objDwgAttribs.dRowAnnSize * m_fTxtWidMF);// Space for Annoation
	
		fWid += (2.0 * pAxisDetRec->fMargin);
		
		pAxisDetRec->fLeftOfYAxis = (iMaxLenOfTagStrs * objDwgAttribs.dRowAnnSize * pAxisDetRec->fTxtWidMF);// Space for Annoation
	}
	{//Calculate Extents now(Vert)

		fHt = 0.0;
		fHt += ((fMaxElev - m_fDatum) * objDwgAttribs.dYScale);//Space for Ordinates
		fHt += (objDwgAttribs.dRowHt * 4.0 + pAxisDetRec->fTitleXtn);// Space for Annoation
		
		fHt += (2.0 * pAxisDetRec->fMargin);

		pAxisDetRec->fTopOfXAxis = (fMaxElev - m_fDatum) * objDwgAttribs.dYScale;//Space for Ordinates
		pAxisDetRec->fBotOfXAxis = (objDwgAttribs.dRowHt * 4.0 + pAxisDetRec->fTitleXtn);// Space for Annoation
	}
	return TRUE;
}
BOOL CXSecFrXLS::Draw(const AcGePoint3d& ptLLC, const XLSXSECDWGPARAM& objDwgAttribs)
{
	double fWid, fHt;
	AXISDETAIL recAxisDet;
	AcGePoint2d ptGraphOrg;

	_SortByOffset(m_RecArray);

	if(!GetExtents(objDwgAttribs, fWid, fHt, &recAxisDet))
		return FALSE;
	
	CXSecStn::RectangleLL(ptLLC, fWid, fHt, objDwgAttribs.szLyrDwgSheet);//Border

	_DrawAxis(objDwgAttribs, ptLLC, ptGraphOrg, recAxisDet);
	_DrawOrdinates(objDwgAttribs, ptGraphOrg, recAxisDet);
	_DrawLabel(objDwgAttribs, ptLLC, recAxisDet);
	
	return TRUE;
}
void CXSecFrXLS::_DrawLabel(const XLSXSECDWGPARAM& RecParam, const AcGePoint3d& ptLLC, const AXISDETAIL& recAxisDet)
{
	CString strCaption;
	AcGePoint3d ptIns;

	strCaption.Format("%s %.3f", RecParam.szXSecLabelTag, m_fChg);
	ptIns = ptLLC;
	ptIns.x += recAxisDet.fMargin + (recAxisDet.fLeftOfYAxis + recAxisDet.fRiteOfYAxisA + recAxisDet.fRiteOfYAxisB) * 0.5;
	ptIns.y += recAxisDet.fMargin;
	ptIns.y += RecParam.dCSLabelAnnSize / 2.0;

	TEXTMID(ptIns, strCaption, 0.0, RecParam.dCSLabelAnnSize, RecParam.szLyrAxis);
}
double CXSecFrXLS::GetMinElev()
{
	int i;
	double fMinElev;

	if(m_RecArray.GetSize() <= 0)
	{
		ads_printf("\nERROR: Control should not reach here [CXSecFrXLS::CalcArea() Tag=A]\n");
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
void CXSecFrXLS::_DrawOrdinates(const XLSXSECDWGPARAM& RecParam, const AcGePoint2d& ptGrphOrg, const AXISDETAIL& /*recAxisDet*/)
{
	AcGePoint3dArray ProfTopPtsArray;
	AcGePoint3d PtStart, PtEnd, PtTxtOrg;
	double fOrdDist, fElev;
	int i, j;
	CString strTmp;
	ads_name entLast;
	NEZXSECORDINATE* pRec;

	//Draw Ords
	for(i = 0; i < m_RecArray.GetSize(); i++)
	{
		pRec = m_RecArray[i];
		fOrdDist = pRec->dOffset * RecParam.dXScale;
		fElev = (pRec->dElev - m_fDatum) * RecParam.dYScale;

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

		//Offset
		PtTxtOrg.x = ptGrphOrg.x; PtTxtOrg.y = ptGrphOrg.y;
		PtTxtOrg.x  += fOrdDist;
		PtTxtOrg.y  -= (3.5 * RecParam.dRowHt);
		strTmp = RTOS(pRec->dOffset);
		TEXTMID(PtTxtOrg, strTmp, DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);

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
void CXSecFrXLS::_DrawAxis(const XLSXSECDWGPARAM& RecParam, const AcGePoint3d& ptLLCParam, AcGePoint2d& ptGraphOrg, const AXISDETAIL& recAxisDet)
{
	int i;
	double	dTotalXAxisLen;
	AcGePoint3d PtStart, PtEnd, ptLLCOfExtent;

	ptLLCOfExtent = ptLLCParam;
	ptLLCOfExtent.x += recAxisDet.fMargin;
	ptLLCOfExtent.y += (recAxisDet.fTitleXtn + recAxisDet.fMargin);

	dTotalXAxisLen = recAxisDet.fLeftOfYAxis + recAxisDet.fRiteOfYAxisA + recAxisDet.fRiteOfYAxisB;

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
	//Offset
	PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (0.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, RecParam.szOffsetTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	
	{//Datum 
		CString strTmp;
		
		PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (4.0 * RecParam.dRowHt) + (1.25 * RecParam.dRowAnnSize);	PtStart.z = 0.0;
		
		strTmp.Format("%s %s", RecParam.szDatumTag, RTOS(m_fDatum));
		TEXTLEFT(PtStart, strTmp, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	}
	ptGraphOrg.x = ptLLCOfExtent.x;
	ptGraphOrg.y = ptLLCOfExtent.y;

	ptGraphOrg.x += (recAxisDet.fLeftOfYAxis + recAxisDet.fRiteOfYAxisA);
	ptGraphOrg.y += (4.0 * RecParam.dRowHt);
}

// class -> CXSecFrXLSDwgGenerator **************************************************************************************************
CXSecFrXLSDwgGenerator::CXSecFrXLSDwgGenerator(const XLSXSECDWGPARAM& Rec) : m_DwgAttribs(Rec)
{
}
CXSecFrXLSDwgGenerator::~CXSecFrXLSDwgGenerator()
{
	Reset();
}
void CXSecFrXLSDwgGenerator::Reset()
{
	DeallocatePtrArray(m_ArrayOrgData);
}
BOOL CXSecFrXLSDwgGenerator::Add(const CXSecFrXLS& objXSec)
{
	CXSecFrXLS* pObjXSec;
	int i;
	
	pObjXSec = new CXSecFrXLS();
	pObjXSec->SetChainage(objXSec.GetChainage());
	pObjXSec->SetDatum(objXSec.GetDatum());
	
	const CNEZXSecOrdinateArray& objOrdinates = objXSec.GetOrdinates();
	for(i = 0; i < objOrdinates.GetSize(); i++)
	{
		const NEZXSECORDINATE* pRec = objOrdinates[i];
		pObjXSec->AddOrdinate(*pRec);
	}

	m_ArrayOrgData.Add(pObjXSec);

	return TRUE;
}

BOOL CXSecFrXLSDwgGenerator::Draw(const AcGePoint3d& ptSheetULCParam, CStringArray* pStrArrayErrs/* = 0L*/)
{
	const double fSheetToSheetDist = m_DwgAttribs.dSheetWid * 10.0 / 100.0;
	BOOL bIsUserBreak;
	int i, j;
	AcGePoint3d ptSheetULC;
	
	//Prepare Drawing Settings
	LAYER(m_DwgAttribs.szLyrAnnotation, 1);
	LAYER(m_DwgAttribs.szLyrTopSurface, 2);
	LAYER(m_DwgAttribs.szLyrOrdinate, 3);
	LAYER(m_DwgAttribs.szLyrAxis, 4);

	LAYER(m_DwgAttribs.szLyrDwgSheet, 8);

	ptSheetULC = ptSheetULCParam;
	CXSecStn::RectangleUL(ptSheetULC, m_DwgAttribs.dSheetWid, m_DwgAttribs.dSheetHeight, m_DwgAttribs.szLyrDwgSheet);
	
	double fCumHt, fRowHeight, fCumWid;
	bIsUserBreak = FALSE;
	for(i = 0, fCumHt = 0.0; i < m_ArrayOrgData.GetSize();)
	{
		int iEndIndex;
		
		if(!_GetNextSectionRange(fCumHt, i, iEndIndex, fRowHeight, bIsUserBreak, pStrArrayErrs))
		{
			return TRUE;//End 
		}
		if(bIsUserBreak)
		{
			CString strTmp;

			if(pStrArrayErrs != 0L)
			{
				strTmp.Format("User break encountered\r\nTotal %d# of section(s) drawn\n", i);
				pStrArrayErrs->Add(strTmp);
			}
			else
			{
				ads_printf("\n\nUser break encountered\r\nTotal %d# of section(s) drawn\n", i);
			}
			return TRUE;//User break 
		}
		if(fRowHeight > m_DwgAttribs.dSheetHeight)
		{
			ads_printf("\nERROR: Failed to draw C/S\nPossible cause: Sheet height is too small\nPlease Check C/S data from Chainage %.3f to %.3f", (m_ArrayOrgData[i])->GetChainage(), (m_ArrayOrgData[iEndIndex])->GetChainage());
			return FALSE;//End 
		}
		//
		if((fCumHt + fRowHeight) > m_DwgAttribs.dSheetHeight)
		{
			ptSheetULC.x += m_DwgAttribs.dSheetWid + fSheetToSheetDist;
			ptSheetULC.y = ptSheetULCParam.y;
			fCumHt = 0.0;
			CXSecStn::RectangleUL(ptSheetULC, m_DwgAttribs.dSheetWid, m_DwgAttribs.dSheetHeight, m_DwgAttribs.szLyrDwgSheet);
		}

		//Draw...
		for(j = i, fCumWid = 0.0; j <= iEndIndex; j++)
		{
			bIsUserBreak = (ads_usrbrk() == 1);
			if(bIsUserBreak)
			{
				CString strTmp;

				if(pStrArrayErrs != 0L)
				{
					strTmp.Format("User break encountered\r\nTotal %d# of section(s) drawn\n", i);
					pStrArrayErrs->Add(strTmp);
				}
				else
				{
					ads_printf("\n\nUser break encountered\r\nTotal %d# of section(s) drawn\n", i);
				}
				return TRUE;//User break 
			}

			CXSecFrXLS* pRec;
			AcGePoint3d ptLLCOfCS;
			
			ptLLCOfCS.z = 0.0;
			ptLLCOfCS.x = ptSheetULC.x + fCumWid;
			ptLLCOfCS.y = ptSheetULC.y - (fCumHt + fRowHeight);

			pRec = m_ArrayOrgData[j];
			
			//{//DEBUG
			//	char szTest[132];
			//	CString strTemp;
			//
			//	strTemp.Format("\nDEBUG: about to draw C/S at Chainage : %f", pRec->GetChainage());
			//	ads_getstring(0, strTemp, szTest);
			//}

			pRec->SetDatum(m_DwgAttribs.dDatum);
			if(!pRec->Draw(ptLLCOfCS, m_DwgAttribs))
			{
				ads_printf("\nERROR: Failed to draw C/S at Chainage : %f", pRec->GetChainage());
			}
			else
			{
				AXISDETAIL recAxisDet;
				double fWid, fHt;

				if(!pRec->GetExtents(m_DwgAttribs, fWid, fHt, &recAxisDet))
				{
					ads_printf("\nFATAL ERROR: Control should not reach here [CXSecFrXLSDwgGenerator::Draw]!!");
					return FALSE;
				}
				else
				{
					fCumWid += fWid;
				}
				ads_printf("\rCross Section #%d at chainage %.3f drawn successfully!", (i+1), pRec->GetChainage());
			}
		}
		
		//
		fCumHt += fRowHeight;
		i = iEndIndex + 1;
	}
	ads_printf("\r\nTotal %d # of Cross Sections drawn", i);
	return TRUE;
}
BOOL CXSecFrXLSDwgGenerator::_GetMaxCSBoundRectHt(const int& iStartIndex, int& iEndIndex, double& fMaxHeight)
{
	int i;
	CXSecFrXLS *pRec;
	double fWid, fHt;
	BOOL bFlag;
	AXISDETAIL recAxisDet;
	
	for(i = iStartIndex, bFlag = FALSE; i <= iEndIndex; i++)
	{
		pRec = m_ArrayOrgData[i];
		if(pRec->GetExtents(m_DwgAttribs, fWid, fHt, &recAxisDet))
		{
			bFlag = TRUE;
			if(i == iStartIndex)
			{
				fMaxHeight = fHt;
			}
			else
			{
				if(fHt > fMaxHeight)
					fMaxHeight = fHt;
			}
		}
	}
	return bFlag;
}
BOOL CXSecFrXLSDwgGenerator::_GetNextSectionRange(const double& /*fCurRowTopDepth*/, const int& iStartIndex, int& iEndIndex, double& fMaxHeight, BOOL& bIsUserBreak, CStringArray* pStrArrayErrs/* = 0L*/)
{
	int i;
	double fWid, fHt, fCumWid;
	CXSecFrXLS *pRec;
	AXISDETAIL recAxisDet;

	iEndIndex = iStartIndex;
	fMaxHeight = 0.0;
	for(i = iStartIndex, fCumWid = 0.0; i < m_ArrayOrgData.GetSize(); i++)
	{
		bIsUserBreak = (ads_usrbrk() == 1);
		if(bIsUserBreak)
		{
			return TRUE;
		}

		pRec = m_ArrayOrgData[i];
		pRec->SetDatum(m_DwgAttribs.dDatum);
		if(!pRec->GetExtents(m_DwgAttribs, fWid, fHt, &recAxisDet))
		{
			ads_printf("\nWarning: Failed to draw C/S at Chainage : %f", pRec->GetChainage());
		}
		else
		{
			if((fWid > m_DwgAttribs.dSheetWid) || (fHt > m_DwgAttribs.dSheetHeight))
			{//C/S Extent too big to fit in Sheet
				CString strTmp;

				if(pStrArrayErrs != 0L)
				{
					strTmp.Format("ERROR: Cross Section at Section %.3f can't be fit into the drawing sheet", pRec->GetChainage());
					pStrArrayErrs->Add(strTmp);
				}
				else
				{
					ads_printf("\n\nERROR: Cross Section at Section %.3f can't be fit into the drawing sheet\n", pRec->GetChainage());
				}
				return FALSE;//ERROR 
			}

			if((fCumWid + fWid) >= m_DwgAttribs.dSheetWid)
			{
				if(!_GetMaxCSBoundRectHt(iStartIndex, iEndIndex, fMaxHeight))
				{
					return FALSE;
				}
				return TRUE;
			}
			fCumWid += fWid;
			iEndIndex = i;
		}
	}
	if(!_GetMaxCSBoundRectHt(iStartIndex, iEndIndex, fMaxHeight))
	{
		return FALSE;
	}
	return TRUE;
}
