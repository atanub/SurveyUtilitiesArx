//Line2PLine.cpp
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Line2PLine.h"

static BOOL L2P(const BOOL& = TRUE);
static BOOL CollectPtsFromSelLines(CPointPair3DPtrArray&, ads_name&);
static long FindStartLine(const CPointPair3DPtrArray&, short&);

static BOOL FindNextSegment(const AcGePoint3d&, CPointPair3DPtrArray&, CPointPair3DPtrArray&);
static BOOL	FindOccuranceInArray(const CPointPair3DPtrArray&, const AcGePoint3d&, const POINTPAIR3D*);
static int	FindInArray(const CPointPair3DPtrArray&, const AcGePoint3d&, short&);

void Lines2PLine()
{
	L2P(TRUE);
	ads_printf("\nDone\n");
}
BOOL GetPtsFrLinesForQPfl(AcGePoint3dArray& pt3DArrayParam)
{
	CPointPair3DPtrArray MainArray, Array, ArrayToDraw;
	ads_name SelSet;
	CString strLyr;
	long i, iIndex;
	short iElementNo;
	BOOL bFlag;

	bFlag = CollectPtsFromSelLines(MainArray, SelSet);
	if(!bFlag)
		return bFlag;
	bFlag = (pt3DArrayParam.length() == 0);
	if(!bFlag)
		return bFlag;

	Array.Append(MainArray);

	ads_printf("\nTotal %d# of points to be verified....", Array.GetSize());
	for(i = 0; Array.GetSize() != 0;)
	{
		iIndex = FindStartLine(Array, iElementNo);
		if(iIndex >= 0)
		{
			CPointPair3DPtrArray ArrayToDraw;
			POINTPAIR3D* pRec;

			pRec = Array[iIndex];
			ArrayToDraw.Add(pRec);
			Array.RemoveAt(iIndex);
			if(iElementNo == 1)
				SwapPoints(pRec);

			if(FindNextSegment(pRec->pt02, Array, ArrayToDraw))
			{
				if(i > 0)
				{
					ads_printf("\nERROR: Multiple segments found !");
					bFlag = FALSE;
					goto CLEANUP;
				}
				else
				{
					int j;
					for(j = 0; j < ArrayToDraw.GetSize(); j++)
					{
						if(j == 0)
							pt3DArrayParam.append(ArrayToDraw[j]->pt01);
						
						pt3DArrayParam.append(ArrayToDraw[j]->pt02);
					}
				}
				i++;
			}
		}
		else
		{
			break;
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	goto CLEANUP;

	CLEANUP :
	{
		ads_ssfree(SelSet);//Free Selset
		for(i = 0; i < MainArray.GetSize(); i++)
		{
			POINTPAIR3D* pRec;
			pRec = MainArray[i];
			delete pRec;
		}
		bFlag = bFlag && (pt3DArrayParam.length() > 0);
		return bFlag;
	}
	bFlag = bFlag && (pt3DArrayParam.length() > 0);
	return bFlag;
}
BOOL L2P(const CPointPair3DPtrArray& MainArray, const char* pszLyr)
{
	CPointPair3DPtrArray Array, ArrayToDraw;
	long i, iIndex;
	short iElementNo;

	Array.Append(MainArray);

	ads_printf("\rTotal %d# of lines (from Layer: \"%s\") to be verified....", (Array.GetSize() / 2), pszLyr);
	for(i = 0; Array.GetSize() != 0;)
	{
		iIndex = FindStartLine(Array, iElementNo);
		if(iIndex >= 0)
		{
			CPointPair3DPtrArray ArrayToDraw;
			POINTPAIR3D* pRec;

			pRec = Array[iIndex];
			ArrayToDraw.Add(pRec);
			Array.RemoveAt(iIndex);
			if(iElementNo == 1)
				SwapPoints(pRec);

			if(FindNextSegment(pRec->pt02, Array, ArrayToDraw))
			{
				i++;
				POLYLINE(ArrayToDraw, pszLyr);
				ads_printf("\rTotal %d# of polylines drawn (Layer: \"%s\", # of Segments:%d) ...", i, pszLyr, ArrayToDraw.GetSize());
			}
		}
		else
		{
			ads_printf("\nFATAL ERROR: Line to Polyline conversion Failed while processing point pair# %d[Layer:%s]!!!\nPossible cause: Start Segment detection failed!\n\n...", i, pszLyr);
			return FALSE;
		}
	}
	ads_printf("\nTotal %d# of polylines drawn in Layer: \"%s\"...", i, pszLyr);
	////////////////////////////////////////////////////////////////////////////////
	return TRUE;
}
static BOOL L2P(const BOOL& bEraseLines /*= TRUE*/)
{
	CPointPair3DPtrArray MainArray, Array, ArrayToDraw;
	ads_name SelSet;
	CString strLyr;
	BOOL bFlag;

	bFlag = CollectPtsFromSelLines(MainArray, SelSet);
	if(!bFlag)
		return bFlag;

	Array.Append(MainArray);

	CLAYER(strLyr);
	//
	L2P(MainArray, strLyr);//Draw Polylines!!
	////////////////////////////////////////////////////////////////////////////////
	goto CLEANUP;

	CLEANUP :
	{
		if(bEraseLines)
		{
			EraseEntitiesInSelSet(SelSet);
		}
		ads_ssfree(SelSet);//Free Selset
		DeallocatePtPairArray(MainArray);
		return bFlag;
	}
	return TRUE;
}
static BOOL CollectPtsFromSelLines(CPointPair3DPtrArray& Array, ads_name& SelSet)
{
	if(Array.GetSize() != 0)
		return 0;
	
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
					pRec->pt02 = ((AcDbLine*)pObj)->endPoint();
			
					Array.Add(pRec);
				}
				pObj->close();
			}
		}
		return TRUE;
	}
	//Don't Free Selset....
	return TRUE;
}

static long FindStartLine(const CPointPair3DPtrArray& Array, short& iElementSlNo)
{
	long i;
	
	for(i = 0; i < Array.GetSize(); i++)
	{
		if(FindOccuranceInArray(Array, (Array[i])->pt01, Array[i]))
		{
			iElementSlNo = 0;
			return i;//Success
		}
		if(FindOccuranceInArray(Array, (Array[i])->pt02, Array[i]))
		{
			iElementSlNo = 1;
			return i;//Success
		}
	}
	iElementSlNo = 0;
	return 0;//Success
}
static BOOL FindOccuranceInArray(const CPointPair3DPtrArray& Array, const AcGePoint3d& Pt, const POINTPAIR3D* pRecToIgnore)
{
	long iOccurance, i;
	
	for(i = 0, iOccurance = 0; i < Array.GetSize(); i++)
	{
		const POINTPAIR3D* pRecToVerify;
		
		pRecToVerify = Array[i];
		if(pRecToVerify == pRecToIgnore)
			continue;

		if((pRecToVerify->pt01 == Pt) || (pRecToVerify->pt02== Pt))
			return TRUE;
	}
	return FALSE;
}

static int FindInArray(const CPointPair3DPtrArray& Array, const AcGePoint3d& Pt, short& iPtIndex)
{
	long i;
	
	for(i = 0, iPtIndex = 0; i < Array.GetSize(); i++)
	{
		const POINTPAIR3D* pRecToVerify;
		
		pRecToVerify = Array[i];
		if((pRecToVerify->pt01 == Pt) || (pRecToVerify->pt02== Pt))
		{
			iPtIndex = (pRecToVerify->pt01 == Pt) ? (short)0 : (short)1;
			return i;
		}
	}
	return -1;
}
static BOOL FindNextSegment(const AcGePoint3d& PtParam, CPointPair3DPtrArray& ArrayMain, CPointPair3DPtrArray& ArrayOfVertexes)
{
	BOOL bCheckedBothEnds;
	long iIndex;
	short iPtIndex;
	AcGePoint3d PtToFind;
	
	for(PtToFind = PtParam, bCheckedBothEnds = FALSE;;)
	{
		iIndex = FindInArray(ArrayMain, PtToFind, iPtIndex);
		if(iIndex < 0)
		{
			if(bCheckedBothEnds)
				break;
			RevertPtPairArray(ArrayOfVertexes);
			PtToFind = ArrayOfVertexes[ArrayOfVertexes.GetSize() - 1]->pt02;
			bCheckedBothEnds = TRUE;
		}
		else
		{
			POINTPAIR3D* pRec;

			pRec = ArrayMain[iIndex];
			if(iPtIndex == 1)
			{//Swap...
				SwapPoints(pRec);
			}
			ArrayOfVertexes.Add(pRec);
			ArrayMain.RemoveAt(iIndex);
			PtToFind = pRec->pt02;
		}
	}
	return (ArrayOfVertexes.GetSize() > 0);
}
