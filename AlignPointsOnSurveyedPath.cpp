//
// File Name: AlignPointsOnSurveyedPath.cpp
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DrawUtil.h"
#include "Util.h"
#include "QueryEntityProperty.h"
#include "AlignPointsOnSurveyedPath.h"
#include "EntitySelectors\EntitySelector.h"
#include "AcDbObjectEx.h"

//
//
// CAlignPointsOnSurveyedPath
//////////////////////////////////////////////////////////////////////////////////////////////
//
CAlignPointsOnSurveyedPath::CAlignPointsOnSurveyedPath(const AcGePoint3d& ptStart, const AcGePoint3d& ptEnd, bool bConsider3Doint/* = false*/)
		: m_bConsider3Doint(bConsider3Doint), m_ptStart(ptStart.x, ptStart.y, ((bConsider3Doint) ? ptStart.z:0.0)), m_ptEnd(ptEnd.x, ptEnd.y, ((bConsider3Doint) ? ptEnd.z:0.0))
{
}

bool CAlignPointsOnSurveyedPath::Move(const AcDbObjectIdArray* pObjIDArrayPoints)
{
	int i;

	for(i = 0; i < pObjIDArrayPoints->length(); i++)
	{
		AcDbObjectEx obj(pObjIDArrayPoints->at(i));
		AcDbObject* pObj;
		AcDbPoint* pPointEnt;
		AcGePoint3d ptPosition;

		pObj = obj.getObject(AcDb::kForWrite);
		if(0L == pObj)
		{
			ads_printf("\nERROR: Failed to access point entity!");
			return false;
		}

		pPointEnt = AcDbPoint::cast(pObj);
		ptPosition = pPointEnt->position();

		ptPosition = MovePointOnAlignment(ptPosition);
		if(Adesk::kTrue == ptPosition.isEqualTo(pPointEnt->position()))
		{
			ads_printf("\nWarning: Perpendicular point is not on the surveyed path, ignoring point # %d of %d!", (i + 1), pObjIDArrayPoints->length());
			continue;
		}
		
		pPointEnt->setPosition(ptPosition);
	}
	return true;
}

AcGePoint3d CAlignPointsOnSurveyedPath::MovePointOnAlignment(const AcGePoint3d& ptParam) const
{
	AcGePoint3d ptResult, ptTmp(ptParam.x, ptParam.y, ((m_bConsider3Doint) ? ptParam.z:0.0));

	if(!FindPerpPointOnLine(ptTmp, m_ptStart, m_ptEnd, ptResult, true))
		return ptParam;


	ptResult.z = ptParam.z;
	return ptResult;
}

void CAlignPointsOnSurveyedPath::Align()
{
	AcGePoint3dArray arrayLineVertices;
	AcGePoint3dArray arrayPoints;
	bool bConsider3Doint;
	CEntitySelectorEx lineSelector(CEntitySelectorFactory::SelectorType::SingleLine);
	CEntitySelectorEx multiPointSelector(CEntitySelectorFactory::SelectorType::Points);
	
		
	const AcDbObjectIdArray* pObjIDArrayLines = lineSelector->Select("\nSelect surveyed path on which the points needs to be shifted:");
	if((0L == pObjIDArrayLines) || (pObjIDArrayLines->length() != 1))
	{
		ads_printf("\nERROR: No entity selected as surveyed path!");
		return;
	}
	if(!CQueryEntityProperty::ExtractLineVertices(pObjIDArrayLines->first(), arrayLineVertices))
	{
		ads_printf("\nERROR: Failed to extract line vertices!");
		return;
	}
	
	const AcDbObjectIdArray* pObjIDArrayPoints = multiPointSelector->Select("\nSelect points which needs to be shifted on the surveyed path:", "\nRemove points :");
	if((0L == pObjIDArrayPoints) || (pObjIDArrayPoints->length() <= 0))
	{
		ads_printf("\nERROR: No point(s) selected for shifting!");
		return;
	}
	bConsider3Doint = false;
	{//Ask Confirmation 
		char szReply[32];
		ads_initget(1, "Yes No");
		ads_getkword("\nConsider 3D point <Y/N> ?", szReply);
		if((szReply[0] == 'y') || (szReply[0] == 'Y'))
		{
			bConsider3Doint = true;
		}
	}

	{//Process
		CAlignPointsOnSurveyedPath obj(arrayLineVertices[0], arrayLineVertices[1], bConsider3Doint);

		obj.Move(pObjIDArrayPoints);
	}
}
