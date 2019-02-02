// QueryEntityProperty.cpp
/////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "QueryEntityProperty.h"
#include "AcDbObjectEx.h"

bool CQueryEntityProperty::Extract3dPolyVertices(AcDbObjectId objID, AcGePoint3dArray& ptArrayParam)
{
	AcDbObjectEx Obj(objID);
	AcDbObject* pObj;

	pObj = Obj.getObject();
	if(0L == pObj)
		return false;
	const bool bFlag = Extract3dPolyVertices(AcDb3dPolyline::cast(pObj), ptArrayParam);
	return bFlag;
}

bool CQueryEntityProperty::Extract3dPolyVertices(AcDb3dPolyline* pObj, AcGePoint3dArray& ptArrayParam)
{//Extract points...
	Acad::ErrorStatus errCode;
	AcDbObjectIterator* pIterator;
	
	pIterator = pObj->vertexIterator();
	for(pIterator->start(); (pIterator->done() == Adesk::kFalse); pIterator->step())
	{
		AcDbObjectId objectIdTmp;
		AcDb3dPolylineVertex* pVert;
		
		objectIdTmp = pIterator->objectId();

		errCode = pObj->openVertex(pVert, objectIdTmp, AcDb::kForRead);
		if(errCode != Acad::eOk)
		{
			delete pIterator;
			ads_printf("\nERROR: Error occured while iterating vertexes!\n");
			return false;
		}
		ptArrayParam.append(pVert->position());
		pVert->close();
	}
	delete pIterator;
	return (ptArrayParam.length() > 0);
}
bool CQueryEntityProperty::ExtractLineVertices(AcDbObjectId objID, AcGePoint3dArray& ptArrayParam)
{
	AcDbObjectEx Obj(objID);
	AcDbObject* pObj;
	AcDbLine* pLineObj;

	while(ptArrayParam.length() > 0)
		ptArrayParam.removeLast();

	pObj = Obj.getObject();
	if(0L == pObj)
		return false;

	pLineObj = AcDbLine::cast(pObj);
	ptArrayParam.append(pLineObj->startPoint());
	ptArrayParam.append(pLineObj->endPoint());
	return (ptArrayParam.length() > 0);
}

bool CQueryEntityProperty::ExtractPointVertex(AcDbObjectId objID, AcGePoint3d& ptParam)
{
	AcDbObjectEx Obj(objID);
	AcDbObject* pObj;

	pObj = Obj.getObject();
	if(0L == pObj)
		return false;
	
	ptParam = AcDbPoint::cast(pObj)->position();
	return true;
}
bool CQueryEntityProperty::ExtractPointVertices(const AcDbObjectIdArray& objIDArray, AcGePoint3dArray& ptArrayParam)
{
	int i;

	while(ptArrayParam.length() > 0)
		ptArrayParam.removeLast();
	
	for(i = 0; i < objIDArray.length(); i++)
	{
		AcGePoint3d pt;
		
		if(!ExtractPointVertex(objIDArray[i], pt))
			return false;
	
		ptArrayParam.append(pt);
	}
	return false;
}
