#include "..\stdafx.h"
#include "..\Util.h"
#include "EntitySelector.h"
//#include <GeAssign.h>

/////////////////////////////////////////////////////////////////////////////
// CEntitySelector
//
CEntitySelector::CEntitySelector() : m_pszPromptAdd(0L), m_pszPromptRemove(0L)
{
}

CEntitySelector::~CEntitySelector()
{
}

const AcDbObjectIdArray* CEntitySelector::Select(const char* pszPromptAdd /* = 0L*/, const char* pszPromptRemove /* = 0L*/)
{
	m_pszPromptAdd = pszPromptAdd;
	m_pszPromptRemove = pszPromptRemove;

	while(m_arrayObjIDs.length() > 0)
	{
		m_arrayObjIDs.removeFirst();
	}

	if(getUserSelectedEntities())
	{
		convertSelSetToObjIDArray();
		ads_ssfree(m_selectionSet);//Free Selset
	}
	
	return (m_arrayObjIDs.length() > 0) ? &m_arrayObjIDs : 0L;
}
bool CEntitySelector::isValidEntity(const AcDbObject*)
{
	//Not implemented....Derived classes may override
	return true;
}

bool CEntitySelector::getUserSelectedEntities()
{
	long iRet;

	if((0L != m_pszPromptAdd) && (0L != m_pszPromptRemove))
	{
		const char* szprompts[] = {m_pszPromptAdd, m_pszPromptRemove};
		iRet = ads_ssget(":$", szprompts, 0L, 0L, m_selectionSet);
	}
	else
	{
		iRet = ads_ssget(0L, 0L, 0L, 0L, m_selectionSet);
	}
	return (iRet == RTNORM);
}

bool CEntitySelector::convertSelSetToObjIDArray()
{
	ads_name Entity;
	long i, iNoOfEnt;


	if(ads_sslength(m_selectionSet, &iNoOfEnt) != RTNORM)
	{
		return false;
	}
	for(i = 0; i < iNoOfEnt; i++)
	{
		if(ads_ssname(m_selectionSet, i, Entity) != RTNORM)
		{
			ads_printf("\nERROR: AutoCAD System error !\n");
			return false;
		}
		{//
			AcDbObjectId objId;
			Acad::ErrorStatus errCode;
			bool bIsValidEntity;

			errCode = acdbGetObjectId(objId, Entity);
			if(errCode != Acad::eOk)
			{
				ads_printf("\nERROR: AutoCAD System error !\n");
				return false;
			}
			{//Check if Valid Entity...
				AcDbObject* pObj;

				errCode = acdbOpenObject(pObj, objId, AcDb::kForRead);
				if(errCode != Acad::eOk)
				{
					ads_printf("\nERROR: AutoCAD System error !\n");
					return FALSE;
				}
				bIsValidEntity = isValidEntity(pObj);
				pObj->close();
			}

			if(bIsValidEntity)
			{
				m_arrayObjIDs.append(objId);
			}
		}
	}
	return (m_arrayObjIDs.length() > 0);
}

/////////////////////////////////////////////////////////////////////////////
// CEntSelectorForExportPts
//
bool CEntSelectorForExportPts::getUserSelectedEntities()
{
	long iRet;
	struct  resbuf*  pRb;

	pRb = ads_buildlist(-4, "<or", RTDXF0, "CIRCLE", RTDXF0, "POINT", RTDXF0, "INSERT", RTDXF0, "LINE", -4, "or>", 0);

	if((0L != m_pszPromptAdd) && (0L != m_pszPromptRemove))
	{
		const char* szprompts[] = {m_pszPromptAdd, m_pszPromptRemove};
		iRet = ads_ssget(":$", szprompts, 0L, pRb, m_selectionSet);
	}
	else
	{
		iRet = ads_ssget(0L, 0L, 0L, pRb, m_selectionSet);
	}
	ads_relrb(pRb);
	return (iRet == RTNORM);
}
bool CEntSelectorForExportPts::isValidEntity(const AcDbObject* pObj)
{
	//ads_printf("\nType: [%s]", pObj->isA()->name());
	return
		(
			(pObj->isA() == AcDbPoint::desc()) ||
			(pObj->isA() == AcDbLine::desc()) ||
			(pObj->isA() == AcDbCircle::desc()) ||
			(pObj->isA() == AcDbBlockReference::desc()) ||
			(pObj->isA() == AcDbPoint::desc())
		);	
}
/////////////////////////////////////////////////////////////////////////////
// CEntSelectorForExportPtsMXRoads
//
bool CEntSelectorForExportPtsMXRoads::getUserSelectedEntities()
{
	long iRet;
	struct  resbuf*  pRb;

	pRb = ads_buildlist(-4, "<or", RTDXF0, "CIRCLE", RTDXF0, "POINT", RTDXF0, "INSERT", RTDXF0, "LINE", RTDXF0, "POLYLINE", -4, "or>", 0);

	if((0L != m_pszPromptAdd) && (0L != m_pszPromptRemove))
	{
		const char* szprompts[] = {m_pszPromptAdd, m_pszPromptRemove};
		iRet = ads_ssget(":$", szprompts, 0L, pRb, m_selectionSet);
	}
	else
	{
		iRet = ads_ssget(0L, 0L, 0L, pRb, m_selectionSet);
	}
	ads_relrb(pRb);
	return (iRet == RTNORM);
}
bool CEntSelectorForExportPtsMXRoads::isValidEntity(const AcDbObject* pObj)
{
	//ads_printf("\nType: [%s]", pObj->isA()->name());
	return
		(
			(pObj->isA() == AcDbPoint::desc()) ||
			(pObj->isA() == AcDbLine::desc()) ||
			(pObj->isA() == AcDb3dPolyline::desc()) ||
			(pObj->isA() == AcDbCircle::desc()) ||
			(pObj->isA() == AcDbBlockReference::desc()) ||
			(pObj->isA() == AcDbPoint::desc())
		);	
}
//
// CEntSelectorFor3DPolyline
//
bool CEntSelectorFor3DPolyline::getUserSelectedEntities()
{
	long iRet;
	struct  resbuf*  pRb;

	pRb = ads_buildlist(-4, "<or", RTDXF0, "POLYLINE", -4, "or>", 0);

	if((0L != m_pszPromptAdd) && (0L != m_pszPromptRemove))
	{
		const char* szprompts[] = {m_pszPromptAdd, m_pszPromptRemove};
		iRet = ads_ssget(":$", szprompts, 0L, pRb, m_selectionSet);
	}
	else
	{
		iRet = ads_ssget(0L, 0L, 0L, pRb, m_selectionSet);
	}
	ads_relrb(pRb);
	return (iRet == RTNORM);
}
bool CEntSelectorFor3DPolyline::isValidEntity(const AcDbObject* pObj)
{
	return (pObj->isA() == AcDb3dPolyline::desc());
}

bool CEntSelectorForSingle3DPolyline::getUserSelectedEntities()
{
	AcGePoint3d pt3D;
	ads_name selset;
	
	if(RTNORM != ads_entsel(((m_pszPromptAdd) ? m_pszPromptAdd: "Select a 3D Polyline:"), selset, asDblArray(pt3D)))
		return false;

	if(RTNORM != ads_ssadd(selset, 0L, m_selectionSet))
		return false;

	return true;
}

bool CEntSelectorSingleLine::isValidEntity(const AcDbObject* pObj)
{
	return (pObj->isA() == AcDbLine::desc());
}

bool CEntSelectorSingleLine::getUserSelectedEntities()
{
	AcGePoint3d pt3D;
	ads_name selset;
	
	if(RTNORM != ads_entsel(((m_pszPromptAdd) ? m_pszPromptAdd: "Select a 3D Polyline:"), selset, asDblArray(pt3D)))
		return false;

	if(RTNORM != ads_ssadd(selset, 0L, m_selectionSet))
		return false;

	return true;
}
//
bool CEntSelectorMultiplePoints::isValidEntity(const AcDbObject* pObj)
{
	return (pObj->isA() == AcDbPoint::desc());
}

bool CEntSelectorMultiplePoints::getUserSelectedEntities()
{
	long iRet;
	struct  resbuf*  pRb;

	pRb = ads_buildlist(-4, "<or", RTDXF0, "POINT", -4, "or>", 0);

	if((0L != m_pszPromptAdd) && (0L != m_pszPromptRemove))
	{
		const char* szprompts[] = {m_pszPromptAdd, m_pszPromptRemove};
		iRet = ads_ssget(":$", szprompts, 0L, pRb, m_selectionSet);
	}
	else
	{
		iRet = ads_ssget(0L, 0L, 0L, pRb, m_selectionSet);
	}
	ads_relrb(pRb);
	return (iRet == RTNORM);
}
//
// CEntitySelectorFactory
//
CEntitySelector* CEntitySelectorFactory::GetEntitySelector(SelectorType eType)
{
	switch(eType)
	{
	case ExportPointsMXRoads:
		return new CEntSelectorForExportPtsMXRoads();
	case ExportPoints:
		return new CEntSelectorForExportPts();
	case SinglePolyline3D:
		return new CEntSelectorForSingle3DPolyline();
	case SingleLine:
		return new CEntSelectorSingleLine();
	case Points:
		return new CEntSelectorMultiplePoints();
	default:
		break;
	}
	return 0L;
}
