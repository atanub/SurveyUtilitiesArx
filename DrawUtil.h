// DrawUtil.h
/////////////////////////////////////////////////////////
#ifndef _DRAWUTIL_H_
#define _DRAWUTIL_H_

#include "Structures.h"
#include "AcDbObjectEx.h"

Acad::ErrorStatus	LAYER(const char*, const int& = -1, AcDbObjectId* = 0L);
BOOL				DRAWPOINT(const AcGePoint3d&, const char*);
int					DRAWPOINTS(const AcGePoint3dArray&, const char*);
void				LINE(const AcGePoint3d&, const AcGePoint3d&, const char*);
void				LINES(const AcGePoint3dArray&, const char*);

void CIRCLE(const AcGePoint3d&, const double&, const char*);

BOOL IsBlockExists(const char*, AcDbObjectId* = 0L);
BOOL GetBlockNames(CStringArray&);
void BLOCK(const char*, const AcGePoint3d&, const char* = 0L, const double& = 0.0, const double& = 1.0, const double& = 1.0, const double& = 1.0);

void TEXTMID(const AcGePoint3d&, const char*, const double&, const double&, const char*);
void TEXTLEFT(const AcGePoint3d&, const char*, const double&, const double&, const char*);
void TEXTRIGHT(const AcGePoint3d&, const char*, const double&, const double&, const char*);

void POLYLINE(AcGePoint3dArray&, const char*, const BOOL& = FALSE);
void POLYLINE(const CPointPair3DPtrArray&, const char*);

BOOL DrawSpotLevel(const AcGePoint3d&, const char*, const char*, const double&, const double&, const double&);

BOOL EraseEntitiesInSelSet(ads_name&);

int		PROXYNOTICE(const short& = -1);
int		EXPERT();
int		LUPREC(const int& = -1);
double	TEXTSIZE(const double& = -1.0);
BOOL	CLAYER(CString&, const char* pszNewLyr = 0L);
BOOL	TEXTSTYLE(AcDbObjectId&);

BOOL SortPointsOnLine(const AcGePoint3d&, AcGePoint3dArray&);

BOOL SelectPoints(ads_name&, const char* = 0L, const char* = 0L);
BOOL ExtractPtsFromSelSet(ads_name&, AcGePoint3dArray&);

BOOL EraseEntities(const AcDbObjectIdArray&);
BOOL ExtratObjectIDs(ads_name&, AcDbObjectIdArray&);


inline  int DIMZIN(const int& iNewSetting = 0)
{
	short iPrec;
	AcDbDatabase*	pDb;

	pDb = acdbCurDwg();
	if(pDb == 0L)
		return -1;
	
	iPrec = (short)pDb->dimzin();
	if((iNewSetting >= 0) && (iNewSetting != iPrec))
		pDb->setDimzin((short)iNewSetting);
	
	return iPrec;
}
inline CString RTOS(const double& fValue)
{
	CString strToRet;
	char szVal[64];
	int iPrec;
	
	iPrec = DIMZIN(0);
	ads_rtos(fValue, 2, LUPREC(), szVal);
	strToRet.Format("%s", szVal);
	
	return strToRet;
}
inline CString GetLyrName(const AcDbEntity* pObj)
{
	char* pszLyr;
	CString strToRet;

	if(Adesk::kTrue == pObj->isReadEnabled())
	{
		pszLyr = pObj->layer();
		strToRet = pszLyr;
		delete []pszLyr;
		return strToRet;
	}
	ads_printf("\nERROR: Access denied..Failed to extract layer name...[TRACE: GetLyrName()] !\n");
	
	return strToRet;
}
inline CString GetLyrName(const AcDbObjectId& objId)
{
	AcDbObject* pObj;
	CString strToRet;

	AcDbObjectEx obj(objId);
	
	pObj = obj.getObject(AcDb::kForRead);
	if(0L == pObj)
	{
		ads_printf("\nERROR: Failed to open object...[TRACE: GetLyrName()] !\n");
		return strToRet;
	}
	strToRet = GetLyrName((AcDbEntity*)pObj);
	return strToRet;
}
inline void ConvertPtPairsToVertices(const CPointPair3DPtrArray& Array, AcGePoint3dArray& arrayResult, BOOL* pbIsClosed /*= 0L*/)
{
	if(0L != pbIsClosed)
		*pbIsClosed = FALSE;
	
	if(Array.GetSize() > 1)
	{
		int i;

		for(i = 0; i < Array.GetSize(); i++)
		{
			arrayResult.append((Array[i])->pt01);
		}
		if(Array.GetSize() > 1)
		{
			if((Array[0])->pt01 == (Array[Array.GetSize() - 1])->pt02)
			{
				arrayResult.append((Array[0])->pt01);//Need to check this line of code!!>> May 17'2003 
				if(0L != pbIsClosed)
					*pbIsClosed = TRUE;

			}
			else
			{
				arrayResult.append((Array[Array.GetSize() - 1])->pt02);
			}
		}
	}
	else
	{//
		arrayResult.append((Array[0])->pt01);
		arrayResult.append((Array[0])->pt02);
	}
}

#endif // _DRAWUTIL_H_
