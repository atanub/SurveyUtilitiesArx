//Line2PLineEx.cpp
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include <stdlib.h>
#include <string.h>
#include <AdsCodes.h>
#include <Dbents.h>
#include "ExcelReaderGen.h"
#include "Line2PLine.h"
#include "Line2PLineEx.h"
#include "DrawUtil.h"
#include "Util.h"

static void GetUniqueLyrNames(AcDbObjectIdArray&, CStringArray&);
static void GetObjIDsOfLyr(const char*, AcDbObjectIdArray&, AcDbObjectIdArray&);
static BOOL CollectPtsFromObjIDs(const AcDbObjectIdArray&, CPointPair3DPtrArray&);

void L2PLayerSpecific()
{
	AcDbObjectIdArray arrayObjIDs;
	CStringArray arrayLyrNames;
	int i;

	{//Get all the Line Entities for conversion....
		struct  resbuf  Rb01;
		const char szEntName[] = "LINE";
		char* szprompts[] = {"Select line entities to convert :", "Remove entities :"};
		ads_name SelSet;
		// Entity type
		Rb01.restype = 0;
		Rb01.resval.rstring = (char*)szEntName;
		Rb01.rbnext = 0L; 
		
		if(ads_ssget(":$", szprompts, 0L, &Rb01, SelSet) != RTNORM)
		{
			ads_printf("\nNo entity(s) selected!");
			return;
		}
		if(!ExtratObjectIDs(SelSet, arrayObjIDs))
		{
			ads_printf("\nERROR: Failed to exract object IDs!");
			return;
		}
		ads_ssfree(SelSet);//Free the Sel set
	}
	GetUniqueLyrNames(arrayObjIDs, arrayLyrNames);
	//
	for(i = 0; i < arrayLyrNames.GetSize(); i++)
	{
		AcDbObjectIdArray arrayObjIDsFiltered;
		CPointPair3DPtrArray Array;
		
		GetObjIDsOfLyr(arrayLyrNames[i], arrayObjIDs, arrayObjIDsFiltered);
		if(!CollectPtsFromObjIDs(arrayObjIDsFiltered, Array))
		{
			ads_printf("\nERROR: Failed to extract Point Pairs! [from L2PLayerSpecific()]");
			DeallocatePtPairArray(Array);
			return;
		}
		//Draw Polylines!!
		if(!L2P(Array, arrayLyrNames[i]))
		{
			ads_printf("\nFATAL ERROR: Line to polyline conversion failed [for Layer:%s(%d/%d)]\nConversion process aborted!", arrayLyrNames[i], (i+1), arrayLyrNames.GetSize());
			DeallocatePtPairArray(Array);
			break;
		}
		DeallocatePtPairArray(Array);
		EraseEntities(arrayObjIDsFiltered);
		if(arrayObjIDs.length() <= 0)
			break;
	}
}
static void GetUniqueLyrNames(AcDbObjectIdArray& arrayObjIDs, CStringArray& arrayLyrNames)
{
	if((arrayObjIDs.length() <= 0) || (arrayLyrNames.GetSize() > 0))
	{
		ads_printf("\nERROR: Invalid parameter passed as argument [from GetUniqueLyrNames()]!");
		return;
	}
	int i;
	
	for(i = 0; i < arrayObjIDs.length(); i++)
	{
		CString strLyr;
		
		strLyr = GetLyrName(arrayObjIDs[i]);
		if(!CExcelReaderGen::FindInStrArray(strLyr, arrayLyrNames))
			arrayLyrNames.Add(strLyr);
	}
}
static void GetObjIDsOfLyr(const char* pszLyr, AcDbObjectIdArray& arrayObjIDsAll, AcDbObjectIdArray& arrayObjIDsFiltered)
{
	int i;
	AcDbObjectIdArray arrayObjIDsFromOtherLyr;

	if((arrayObjIDsAll.length() <= 0) || (arrayObjIDsFiltered.length() > 0))
	{
		ads_printf("\nERROR: Invalid parameter passed as argument [from GetObjIDsOfLyr()]!");
		return;
	}
	//
	for(i = 0; i < arrayObjIDsAll.length(); i++)
	{
		if(0 == GetLyrName(arrayObjIDsAll[i]).CompareNoCase(pszLyr))
		{
			arrayObjIDsFiltered.append(arrayObjIDsAll[i]);//Add to filtered list
		}
		else
		{
			arrayObjIDsFromOtherLyr.append(arrayObjIDsAll[i]);
		}
	}
	{//Remove all the elements !!
		while(arrayObjIDsAll.length() > 0)
			arrayObjIDsAll.removeFirst();
	}
	arrayObjIDsAll.append(arrayObjIDsFromOtherLyr);
}
static BOOL CollectPtsFromObjIDs(const AcDbObjectIdArray& arrayObjID, CPointPair3DPtrArray& Array)
{
	long i;
	
	if(Array.GetSize() != 0)
	{
		ads_printf("\nERROR: Invalid parameter passed as argument [from CollectPtsFromObjIDs()]!");
		return FALSE;
	}
	//
	for(i = 0; i < arrayObjID.length(); i++)
	{
		AcDbObject* pObj;

		if(Acad::eOk != acdbOpenObject(pObj, arrayObjID[i], AcDb::kForRead))
		{
			ads_printf("\nERROR: AutoCAD System error [from CollectPtsFromObjIDs()]!\n");
			return FALSE;
		}
		if(pObj->isA() != AcDbLine::desc())
		{
			pObj->close();
			ads_printf("\nERROR: Entity found is not a line [from CollectPtsFromObjIDs()]!\n");
			return FALSE;
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
	return TRUE;
}
