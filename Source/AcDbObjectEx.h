//AcDbObjectEx.h
//////////////////////////////////////////////////////////////////////////////
#ifndef _ACDBOBJECTEX_H
#define _ACDBOBJECTEX_H

#include <acgi.h>
#include <AdsCodes.h>
#include <GEAssign.h>
#include <Math.h>

class AcDbObjectEx
{
public:
	AcDbObjectEx(AcDbObjectId objID) : m_objID(objID)
	{
		m_pObject = 0L;
	}
	~AcDbObjectEx()
	{
		if(m_pObject != 0L)
			m_pObject->close();
	}
public:
	AcDbObject* getObject(AcDb::OpenMode eOpenMode = AcDb::kForRead)
	{
		Acad::ErrorStatus errCode;
		
		if(0L != m_pObject)
			return m_pObject;

		errCode = acdbOpenObject(m_pObject, m_objID, eOpenMode);
		if(errCode != Acad::eOk)
		{
			ads_printf("\nERROR: AutoCAD System error ! Failed to open entity\n");
			return 0L;
		}
		return m_pObject;
	}

protected:
	const AcDbObjectId		m_objID;
	AcDbObject*				m_pObject;
};

#endif // _ACDBOBJECTEX_H

