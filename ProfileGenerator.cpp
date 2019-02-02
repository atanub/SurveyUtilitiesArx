//------------------------ UNDER CONSTRUCTION ------------------------ 
//------------------------ UNDER CONSTRUCTION ------------------------ 
//------------------------ UNDER CONSTRUCTION ------------------------ 
//------------------------ UNDER CONSTRUCTION ------------------------ 
//------------------------ UNDER CONSTRUCTION ------------------------ 

//ProfileGenerator.cpp

#include "stdafx.h"
#include "ProfileGenerator.h"
#include "Util.h"
#include "DrawUtil.h"

//
//CProfileDrawingParams
////////////////////////////////////////////////////////////////////////////////////
//
CProfileDrawingParams::CProfileDrawingParams()
{
	m_dDatum = 0.0;
	m_dXScale = 0.0;
	m_dYScale = 0.0;
	m_dLabTxtSize = 0.0;
	m_dNumAnnTxtSize = 0.0;
	m_dRowHt = 0.0;
	m_fTxtAnnotationAngleInDeg = 0.0;
	m_fTxtAnnotationMargin = 0.0;
	m_dXistTopSurfAnnBlkSpaceMin = 0.0;
	m_dXistTopSurfAnnBlkScale = 0.0;
}
CProfileDrawingParams::CProfileDrawingParams(const CProfileDrawingParams& objParam)
{
	this->m_dDatum						= objParam.m_dDatum;
	this->m_dXScale						= objParam.m_dXScale;
	this->m_dYScale 						= objParam.m_dYScale;
	this->m_dLabTxtSize 					= objParam.m_dLabTxtSize;
	this->m_dNumAnnTxtSize 				= objParam.m_dNumAnnTxtSize;
	this->m_dRowHt 						= objParam.m_dRowHt;
	this->m_fTxtAnnotationAngleInDeg		= objParam.m_fTxtAnnotationAngleInDeg;
	this->m_fTxtAnnotationMargin			= objParam.m_fTxtAnnotationMargin;
	this->m_dXistTopSurfAnnBlkSpaceMin	= objParam.m_dXistTopSurfAnnBlkSpaceMin;
	this->m_dXistTopSurfAnnBlkScale		= objParam.m_dXistTopSurfAnnBlkScale;
}
CProfileDrawingParams::~CProfileDrawingParams()
{
}

//
//CProfileOrdinate
////////////////////////////////////////////////////////////////////////////////////
//
CProfileOrdinate::CProfileOrdinate()
{
}
CProfileOrdinate::CProfileOrdinate(const CProfileOrdinate&)
{
}

CProfileOrdinate::~CProfileOrdinate()
{
}

//
//CProfileOrdinateGroup
////////////////////////////////////////////////////////////////////////////////////
//
CProfileOrdinateGroup::CProfileOrdinateGroup()
{
}
CProfileOrdinateGroup::CProfileOrdinateGroup(const CProfileOrdinateGroup&)
{
}

CProfileOrdinateGroup::~CProfileOrdinateGroup()
{
}

//
//CProfileTag
////////////////////////////////////////////////////////////////////////////////////
//
CProfileTag::CProfileTag(const CProfileTag& objParam)
{
	this->m_strDescription		= objParam.m_strDescription;
	this->m_dOrdinateY			= objParam.m_dOrdinateY;
}
CProfileTag::CProfileTag(const char*)
{
	m_dOrdinateY = 0.0;
}
CProfileTag::~CProfileTag()
{
}

//facility to Draw Itself....
bool CProfileTag::Draw(const double&, const char*) const
{
	return false;
}

//
//CProfileGenerator
////////////////////////////////////////////////////////////////////////////////////
//
CProfileGenerator::CProfileGenerator()
{
}
CProfileGenerator::CProfileGenerator(const CProfileGenerator&)
{
}
CProfileGenerator::~CProfileGenerator()
{
}

bool CProfileGenerator::Draw()
{
	return false;
}

// bool CProfileGenerator::AddTag(const CProfileTag&)
// {
// 	return false;
// }
// 
// bool CProfileGenerator::AddTag(const char*)
// {
// 	return false;
// }
// 
bool CProfileGenerator::AddOrdinate(const char*, const double&, const double&, const double*)
{
	return false;
}


//Pseudo Code ...........
void Test(const RDPFL_CHG_Z_DWG_PARAM& recParam)
{
	int i;
	const char* pszTags[] = 
	{
		recParam.szXElevTag,
		recParam.szDsgnElevTag,
		recParam.szExistingDsgnElevTag,
		recParam.szDsgnElevDifferenceTag,
		recParam.szCutFillTag,
		recParam.szDatum,
		recParam.szChgTag
	};
	CProfileGenerator objGenerator;

	for(i = 0; i < 100; i++)
	{
		const double fChainage = i * 10.0;
		double fXElev = 100.8;
		double fDsgnElev = 100.8;

		objGenerator.AddOrdinate(recParam.szXElevTag, fChainage, fXElev, &fXElev);
		objGenerator.AddOrdinate(recParam.szDsgnElevTag, fChainage, fDsgnElev, &fDsgnElev);
	}
}
