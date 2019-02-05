//QueryEntityProperty.h
//////////////////////////////////////////////////////////////////////////////
#ifndef _QUERYENTITYPROPERTY_H
#define _QUERYENTITYPROPERTY_H

#include <acgi.h>
#include <AdsCodes.h>
#include <GEAssign.h>
#include <Math.h>

class CQueryEntityProperty
{
protected:
	CQueryEntityProperty() {;}

public:
	static bool Extract3dPolyVertices(AcDb3dPolyline* pObj, AcGePoint3dArray& ptArrayParam);
	static bool Extract3dPolyVertices(AcDbObjectId objID, AcGePoint3dArray& ptArrayParam);
	static bool ExtractLineVertices(AcDbObjectId objID, AcGePoint3dArray& ptArrayParam);
	static bool ExtractPointVertex(AcDbObjectId objID, AcGePoint3d& ptParam);
	static bool ExtractPointVertices(const AcDbObjectIdArray& objIDArray, AcGePoint3dArray& ptArrayParam);
};


#endif // _QUERYENTITYPROPERTY_H

