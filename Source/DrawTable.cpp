//DrawTable.cpp
#include "stdafx.h"
#include "DrawTable.h"
#include "DrawUtil.h"



CDrawTable::CDrawTable(const CPtrArray* pPtrArray, const double& fTxtHt, const double& fMargin, const BOOL& bDrawGrid) : m_pPtrArray(pPtrArray), m_fTxtHt(fTxtHt), m_fMargin(fMargin), m_bDrawGrid(bDrawGrid)
{
}
CDrawTable::~CDrawTable()
{
}

BOOL CDrawTable::FindMaxColWids(const CPtrArray* pArray, const double& fTxtSize, AcGeDoubleArray& Array)
{
	int i, j, iCount;
	if(pArray == 0L)
		return FALSE;
	
	//Check the # of columns 1st
	for(i = 0, iCount = -1; i < pArray->GetSize(); i++)
	{
		CStringArray* pStrArray;

		pStrArray = (CStringArray*)pArray->GetAt(i);
		if(iCount < 0)
		{
			iCount = pStrArray->GetSize();
		}
		else
		{
			if(iCount != pStrArray->GetSize())
			{
				ads_printf("\nFATAL ERROR: #of columns mismatch in row #%d, while drawing table, (iCount[%d] != pStrArray->GetSize()[%d])", i, iCount, pStrArray->GetSize());
				return FALSE;
			}
		}
	}
	{//Find max col widths..........
		AcDbObjectId ObjIDTxtStyle;
		AcGiTextStyle Style;
		AcGePoint2d ptExtent;

		if(!TEXTSTYLE(ObjIDTxtStyle))
			return FALSE;
		//
		if(fromAcDbTextStyle(Style, ObjIDTxtStyle) != Acad::eOk)
		{
			return FALSE;
		}
		Style.setTextSize(fTxtSize);


		for(j = 0; j < iCount; j++)
		{
			double fMaxLen, fCurLen;
			
			for(i = 0, fMaxLen = -1.0; i < pArray->GetSize(); i++)
			{
				CStringArray* pStrArray;
				
				pStrArray = (CStringArray*)pArray->GetAt(i);

				CString& strRef = (*pStrArray)[j];
				//
				ptExtent = Style.extents(strRef, Adesk::kFalse, strlen(strRef), Adesk::kTrue);
				fCurLen = ptExtent.x;
				//
				if(fMaxLen < 0)
					fMaxLen = fCurLen;
				else
				{
					if(fCurLen > fMaxLen)
						fMaxLen = fCurLen;
				}
			}
			Array.append(fMaxLen);
		}
	}
	return TRUE;
}
BOOL CDrawTable::Draw(const AcGePoint3d& ptBase, const CString& strLyr)
{
	AcGeDoubleArray ArrayColWids;
	
	if(!CDrawTable::FindMaxColWids(m_pPtrArray, m_fTxtHt, ArrayColWids))
		return FALSE;

	if(m_bDrawGrid)
	{
		if(!CDrawTable::DrawGrid(m_pPtrArray, ArrayColWids, ptBase, m_fTxtHt, m_fMargin, strLyr))
			return FALSE;
	}

	if(!CDrawTable::DrawText(m_pPtrArray, ArrayColWids, ptBase, m_fTxtHt, m_fMargin, strLyr))
		return FALSE;
	
	return FALSE;
}
BOOL CDrawTable::DrawGrid(const CPtrArray* pArray, const AcGeDoubleArray& Array, const AcGePoint3d& ptBase, const double& fTxtHt, const double& fMargin, const CString& strLyr)
{
	int i;
	double fRowLen, fColDepth, fCumDist;

	fRowLen = 0;
	for(i = 0; i < Array.length(); i++)
	{
		fRowLen += Array[i];
	}
	fRowLen = fRowLen + (Array.length() + 1) * fMargin;
	fColDepth = fTxtHt * pArray->GetSize() + (pArray->GetSize() + 1) * fMargin;
	
	//Draw Vertical Lines
	for(i = 0, fCumDist = 0.0; i < Array.length(); i++)
	{
		const double fColLen = Array[i];
		const AcGePoint3d ptStart	(ptBase.x + fCumDist, ptBase.y,				ptBase.z);
		const AcGePoint3d ptEnd		(ptBase.x + fCumDist, ptBase.y - fColDepth, ptBase.z);

		if((i == 0) || (i == (Array.length() - 1)))
		{
			fCumDist = fCumDist + fColLen + fMargin + fMargin / 2.0;
			if(i == (Array.length() - 1))
			{//Last Row
				const double fColLenA = Array[i];
				const AcGePoint3d ptStartA	(ptBase.x + fCumDist, ptBase.y,				ptBase.z);
				const AcGePoint3d ptEndA	(ptBase.x + fCumDist, ptBase.y - fColDepth, ptBase.z);
				
				LINE(ptStartA, ptEndA, strLyr);
			}
		}
		else
		{
			fCumDist = fCumDist + fColLen + fMargin;
		}
		//
		LINE(ptStart, ptEnd, strLyr);

		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encounterd\n");
			return FALSE;
		}
	}
	//Draw Horz Lines
	for(i = 0, fCumDist = 0.0; i < pArray->GetSize(); i++)
	{
		const AcGePoint3d ptStart	(ptBase.x + 0.0,	 ptBase.y - fCumDist, ptBase.z);
		const AcGePoint3d ptEnd		(ptBase.x + fRowLen, ptBase.y - fCumDist, ptBase.z);

		if((i == 0) || (i == (pArray->GetSize() - 1)))
		{
			fCumDist = fCumDist + fTxtHt + fMargin + fMargin / 2.0;
			if(i == (pArray->GetSize() - 1))
			{//Last Row
				const AcGePoint3d ptStartA	(ptBase.x + 0.0,	 ptBase.y - fCumDist, ptBase.z);
				const AcGePoint3d ptEndA	(ptBase.x + fRowLen, ptBase.y - fCumDist, ptBase.z);
				
				LINE(ptStartA, ptEndA, strLyr);
			}
		}
		else
		{
			fCumDist = fCumDist + fTxtHt + fMargin;
		}
		//
		LINE(ptStart, ptEnd, strLyr);
		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break encounterd\n");
			return FALSE;
		}
	}
	return TRUE;
}
BOOL CDrawTable::DrawText(const CPtrArray* pArray, const AcGeDoubleArray& Array, const AcGePoint3d& ptBase, const double& fTxtHt, const double& fMargin, const CString& strLyr)
{
	const double fCharWidFac = 0.9;
	int i, j;
	AcGePoint3d ptTxtInsert, ptURCOfCell;

	//Draw Texts Row by Row 
	ptURCOfCell = ptBase;
	for(i = 0; i < pArray->GetSize(); i++)
	{
		const CStringArray* pStrArray = (CStringArray*)pArray->GetAt(i);

		for(j = 0, ptURCOfCell.x = ptBase.x; j < Array.length(); j++)
		{
			if(ads_usrbrk() == 1)
			{
				ads_printf("\nUser break encounterd\n");
				return FALSE;
			}
			const double fColLen = Array[j];
			if(j == 0)
			{
				ptTxtInsert.x = ptURCOfCell.x + fMargin;
				ptTxtInsert.y = ptURCOfCell.y - fMargin / 2.0 - (fTxtHt / 2.0);
				ptTxtInsert.z = 0.0;

				ptURCOfCell.x = ptURCOfCell.x + fMargin + fColLen + fMargin / 2.0;
			}
			else
			{
				ptTxtInsert.x = ptURCOfCell.x + fMargin / 2.0;
				ptTxtInsert.y = ptURCOfCell.y - fMargin / 2.0 - (fTxtHt / 2.0);
				ptTxtInsert.z = 0.0;
			
				ptURCOfCell.x = ptURCOfCell.x + fColLen + fMargin;
			}
			TEXTLEFT(ptTxtInsert, (LPCSTR)(pStrArray->GetAt(j)), 0.0, fTxtHt, (LPCSTR)strLyr);
		}
		if(i == 0)
		{
			ptURCOfCell.y = ptURCOfCell.y - fMargin - fTxtHt - fMargin / 2.0;
		}
		else
		{
			ptURCOfCell.y = ptURCOfCell.y - fMargin - fTxtHt;
		}
	}
	return TRUE;
}

