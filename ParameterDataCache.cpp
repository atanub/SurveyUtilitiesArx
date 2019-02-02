// ParameterDataCache.cpp : implementation file
//

#include "stdafx.h"
#include "ParameterDataCache.h"
#include "DrawUtil.h"
#include "Util.h"


CParameterDataCache::CParameterDataCache()
{
	m_pRdPflNEZDataRec = 0L;
	m_pPflOffZDWGParam = 0L;
	m_pRdXSecOffElevData = 0L;
	m_pCSDDwgParam = 0L;
	m_pCSDDwgParamEx = 0L;
	m_pDrawParamRDPlanFRTxtFile = 0L;
	m_pInsBlkFrFileParams = 0L;
	m_pFBookDwgParam = 0L;
	m_pXlsXSecDwgParam = 0L;
	m_pRDPflChgZDwgParam = 0L;
	m_pTravDwgParam = 0L;
}
CParameterDataCache::~CParameterDataCache()
{
	if(0L != m_pRdPflNEZDataRec)
		delete m_pRdPflNEZDataRec;
	if(0L != m_pPflOffZDWGParam)
		delete m_pPflOffZDWGParam;
	if(0L != m_pRdXSecOffElevData)
		delete m_pRdXSecOffElevData;
	if(0L != m_pCSDDwgParam)
		delete m_pCSDDwgParam;
	if(0L != m_pCSDDwgParamEx)
		delete m_pCSDDwgParamEx;
	if(0L != m_pDrawParamRDPlanFRTxtFile)
		delete m_pDrawParamRDPlanFRTxtFile;
	if(0L != m_pInsBlkFrFileParams)
		delete m_pInsBlkFrFileParams;
	if(0L != m_pFBookDwgParam)
		delete m_pFBookDwgParam;
	if(0L != m_pXlsXSecDwgParam)
		delete m_pXlsXSecDwgParam;
	if(0L != m_pRDPflChgZDwgParam)
		delete m_pRDPflChgZDwgParam;
	if(0L != m_pTravDwgParam)
		delete m_pTravDwgParam;
}
const RDPFLNEZDATA* CParameterDataCache::GetRdPflNEZDataRec()
{
	if(0L == m_pRdPflNEZDataRec)
	{
		m_pRdPflNEZDataRec = new RDPFLNEZDATA;
		{//Initialize default data...
			m_pRdPflNEZDataRec->bConsider3DPts = FALSE;
			m_pRdPflNEZDataRec->dXScale = 1.0;
			m_pRdPflNEZDataRec->dYScale = 10.0;
			m_pRdPflNEZDataRec->dDatum = 0.0;
			m_pRdPflNEZDataRec->dNumAnnSize = 2.0;
			m_pRdPflNEZDataRec->dRowAnnSize = 4.0;
			m_pRdPflNEZDataRec->dRowHt = 15.0;
			
			m_pRdPflNEZDataRec->dOrdInterval = -1.0;//-Ve means the feature is not public
			
			strcpy(m_pRdPflNEZDataRec->szProfileTag,	"Profile");
			strcpy(m_pRdPflNEZDataRec->szElevTag,		"Elevation");
			strcpy(m_pRdPflNEZDataRec->szNorthingTag,	"Northing");
			strcpy(m_pRdPflNEZDataRec->szEastingTag,	"Easting");
			strcpy(m_pRdPflNEZDataRec->szCumDistTag,	"Cumulative Distance");
			strcpy(m_pRdPflNEZDataRec->szDistTag,		"Distance");
			strcpy(m_pRdPflNEZDataRec->szDatumTag,		"Datum = ");
			
			strcpy(m_pRdPflNEZDataRec->szLyrAnnotation,	"PROFILE_ANNOTATION");
			strcpy(m_pRdPflNEZDataRec->szLyrTopSurface,	"PROFILE_TOP_SURFACE");
			strcpy(m_pRdPflNEZDataRec->szLyrOrdinate,	"PROFILE_ORDINATE");
			strcpy(m_pRdPflNEZDataRec->szLyrAxis,		"PROFILE_AXIS");
			
			strcpy(m_pRdPflNEZDataRec->szOutFile,		"");//Valid only if (m_pRdPflNEZDataRec->dOrdInterval > 0.0)

			//Top Surface Related.......
			strcpy(m_pRdPflNEZDataRec->szXistTopSurfAnnBlk,			"");
			m_pRdPflNEZDataRec->dXistTopSurfAnnBlkScale = 1.0;
			m_pRdPflNEZDataRec->dXistTopSurfAnnBlkSpaceMin = 1.0;

			m_pRdPflNEZDataRec->bDrawGeneratedPoints = TRUE;//Valid only if (m_pRdPflNEZDataRec->dOrdInterval > 0.0)
		}
		return m_pRdPflNEZDataRec;
	}
	return m_pRdPflNEZDataRec;
}
void CParameterDataCache::SetRdPflNEZDataRec(const RDPFLNEZDATA* pRec)
{
	GetRdPflNEZDataRec();//Create object, if not already created!!
	*(m_pRdPflNEZDataRec) = *pRec;
}
const PFLOFFZDWGPARAM* CParameterDataCache::GetPflOffZDWGParam()
{
	if(0L == m_pPflOffZDWGParam)
	{
		m_pPflOffZDWGParam = new PFLOFFZDWGPARAM;
		{//Initialize default data...
			m_pPflOffZDWGParam->dXScale = 1.0;
			m_pPflOffZDWGParam->dYScale = 10.0;
			m_pPflOffZDWGParam->dDatum = 0.0;

			m_pPflOffZDWGParam->dNumAnnSize			= 2.0;
			m_pPflOffZDWGParam->dRowAnnSize			= 4.0;
			m_pPflOffZDWGParam->dRowHt				= 20.0;
			m_pPflOffZDWGParam->fOrdAnnInterval		= -1.0;//-Ve means the feature is not public

			strcpy(m_pPflOffZDWGParam->szProfileTag,	"Profile");
			strcpy(m_pPflOffZDWGParam->szElevTag,		"Elevation");
			strcpy(m_pPflOffZDWGParam->szDistTag,		"Chainage:");
			strcpy(m_pPflOffZDWGParam->szCumDistTag,	"Dist. from Last Station");
			strcpy(m_pPflOffZDWGParam->szDatumTag,		"Datum = ");

			strcpy(m_pPflOffZDWGParam->szLyrAnnotation,		"PFL_ANNOTATION");
			strcpy(m_pPflOffZDWGParam->szLyrTopSurface,		"PFL_TOP_SURFACE_EXISTING");
			strcpy(m_pPflOffZDWGParam->szLyrOrdinate,		"PFL_ORDINATE");
			strcpy(m_pPflOffZDWGParam->szLyrAxis,			"PFL_AXIS");
			
			strcpy(m_pPflOffZDWGParam->szOutDataFile,		"");//Not always To be Saved
			strcpy(m_pPflOffZDWGParam->szDataFile,			"");
#ifdef _DEBUG
			strcpy(m_pPflOffZDWGParam->szDataFile,			"C:/temp/SurveyData/Sample Data/Profile/profile.prd");
#endif //_DEBUG

			//Top Surface Related.......
			strcpy(m_pPflOffZDWGParam->szXistTopSurfAnnBlk,			"");
			m_pPflOffZDWGParam->dXistTopSurfAnnBlkScale = 1.0;
			m_pPflOffZDWGParam->dXistTopSurfAnnBlkSpaceMin = 1.0;
		}
		return m_pPflOffZDWGParam;
	}
	return m_pPflOffZDWGParam;
}
void CParameterDataCache::SetPflOffZDWGParam(const PFLOFFZDWGPARAM* pRec)
{
	GetPflOffZDWGParam();//Create object, if not already created!!

	*(m_pPflOffZDWGParam) = *pRec;
}
void CParameterDataCache::SetRdXSecOffElevDataParam(const RDXSECOFFELEVDATA* pRec)
{
	GetRdXSecOffElevDataParam();//Create object, if not already created!!
	*(m_pRdXSecOffElevData) = *pRec;
}
const RDXSECOFFELEVDATA* CParameterDataCache::GetRdXSecOffElevDataParam()
{
	if(0L == m_pRdXSecOffElevData)
	{//Initialize default data...
		m_pRdXSecOffElevData = new RDXSECOFFELEVDATA;
		
		m_pRdXSecOffElevData->dXScale = 1.0;
		m_pRdXSecOffElevData->dYScale = 10.0;
		m_pRdXSecOffElevData->dNumAnnSize = 2.0;
		m_pRdXSecOffElevData->dRowAnnSize = 3.0;
		m_pRdXSecOffElevData->dCSLabelAnnSize = 4.0;
		m_pRdXSecOffElevData->dRowHt = 20.0;

		m_pRdXSecOffElevData->dStartChg = 0.0;
		m_pRdXSecOffElevData->iDrawPathWIthMinMaxElev = 0;//0 = Don't draw, Ask user for PID
										//1 = Draw PID having Min Elevs
										//2 = Calculate PID having Min Elevs and Draw C/S 
										//4 = Draw PID having Max Elevs
										//8 = Calculate PID having Max Elevs and Draw C/S 

		m_pRdXSecOffElevData->fOrdInterval = -1.0;
#ifdef _DEBUG
		m_pRdXSecOffElevData->fOrdInterval = 5.0;
#endif//_DEBUG
		m_pRdXSecOffElevData->bDrawGeneratedPoints = TRUE;
		m_pRdXSecOffElevData->bDrawNorthingEastingAnnSection = TRUE;

		strcpy(m_pRdXSecOffElevData->szElevTag, "Elevation");
		strcpy(m_pRdXSecOffElevData->szNorthingTag, "Northing");
		strcpy(m_pRdXSecOffElevData->szEastingTag, "Easting");
		strcpy(m_pRdXSecOffElevData->szOffsetTag, "Offset");
		strcpy(m_pRdXSecOffElevData->szDatumTagPrefix, "Datum");
		strcpy(m_pRdXSecOffElevData->szDatumTagSuffix, "m above MSL");
		strcpy(m_pRdXSecOffElevData->szXSecLabelTag, "C/S at Ch:");

		strcpy(m_pRdXSecOffElevData->szLyrAnnotation,	"XSECN_ANNOTATION");
		strcpy(m_pRdXSecOffElevData->szLyrTopSurface,	"XSECN_TOP_SURFACE");
		strcpy(m_pRdXSecOffElevData->szLyrOrdinate,		"XSECN_ORDINATE");
		strcpy(m_pRdXSecOffElevData->szLyrAxis,			"XSECN_AXIS");
		strcpy(m_pRdXSecOffElevData->szLyrMinMaxPIDPath,"MINMAX_PID_PATH");
		
		strcpy(m_pRdXSecOffElevData->szOutFileCSD,		"");
		strcpy(m_pRdXSecOffElevData->szOutFilePID,		"");

		m_pRdXSecOffElevData->dSheetWid = 1000.0;
		m_pRdXSecOffElevData->dSheetHeight = 800.0;
		m_pRdXSecOffElevData->dNPlateWid = 250.0;
		m_pRdXSecOffElevData->dNPlateHeight = 150.0;

#ifdef _DEBUG
		m_pRdXSecOffElevData->arrayDatumRanges.append(AcGePoint3d(0.0, 1.0E4, 365.0));
#endif//_DEBUG
		m_pRdXSecOffElevData->fDatumDenominator = 5.0;
		m_pRdXSecOffElevData->fMinOrdinateHt = 10.0;
	}
	return m_pRdXSecOffElevData;
}
const CSDDWGPARAM* CParameterDataCache::GetCSDDwgParam()
{
	if(0L == m_pCSDDwgParam)
	{//Initialize default data...
		m_pCSDDwgParam = new CSDDWGPARAM;

		m_pCSDDwgParam->dXScale = 1.0;
		m_pCSDDwgParam->dYScale = 10.0;

		m_pCSDDwgParam->dNumAnnSize = 2.0;
		m_pCSDDwgParam->dRowAnnSize = 3.0;
		m_pCSDDwgParam->dRowHt = 15.0;
		m_pCSDDwgParam->dCSLabelAnnSize = 4.0;

		m_pCSDDwgParam->dStartChg = 0.0;
		m_pCSDDwgParam->dEndChg = 0.0;

		m_pCSDDwgParam->fOrdInterval = -1.0;
#ifdef _DEBUG
		m_pCSDDwgParam->fOrdInterval = 5.0;
#endif//_DEBUG

		m_pCSDDwgParam->dSheetWid = 1000.0;
		m_pCSDDwgParam->dSheetHeight = 900.0;
		m_pCSDDwgParam->dNPlateWid = 250.0;
		m_pCSDDwgParam->dNPlateHeight = 150.0;

		strcpy(m_pCSDDwgParam->szElevTag, "R.L. in Metre");
		strcpy(m_pCSDDwgParam->szOffsetTag, "Distance in Metre");
		strcpy(m_pCSDDwgParam->szDatumTag, "Datum = ");
		strcpy(m_pCSDDwgParam->szDataFile, "");
		strcpy(m_pCSDDwgParam->szXSecLabelTag, "C/S at CH:");

		strcpy(m_pCSDDwgParam->szLyrAnnotation, "XSECN_ANNOTATION");
		strcpy(m_pCSDDwgParam->szLyrTopSurface, "XSECN_TOP_SURFACE");
		strcpy(m_pCSDDwgParam->szLyrOrdinate,	"XSECN_ORDINATE");
		strcpy(m_pCSDDwgParam->szLyrAxis,		"XSECN_AXIS");

		//Existing Top Surface Related.......
		strcpy(m_pCSDDwgParam->szXistTopSurfAnnBlk,			"");
		m_pCSDDwgParam->dXistTopSurfAnnBlkScale = 1.0;
		m_pCSDDwgParam->dXistTopSurfAnnBlkSpaceMin = 1.0;
	}
	return m_pCSDDwgParam;
}
void CParameterDataCache::SetCSDDwgParam(const CSDDWGPARAM* pRec)
{
	GetCSDDwgParam();//Create object, if not already created!!

	*(m_pCSDDwgParam) = *pRec;
}
const CSDDWGPARAMEX* CParameterDataCache::GetCSDDwgParamEx()
{
	if(0L == m_pCSDDwgParamEx)
	{//Initialize default data...
		m_pCSDDwgParamEx = new CSDDWGPARAMEX;

		m_pCSDDwgParamEx->dXScale = 1.0;
		m_pCSDDwgParamEx->dYScale = 10.0;

		m_pCSDDwgParamEx->dNumAnnSize = 2.0;
		m_pCSDDwgParamEx->dRowAnnSize = 3.0;
		m_pCSDDwgParamEx->dRowHt = 15.0;
		m_pCSDDwgParamEx->dCSLabelAnnSize = 4.0;

		m_pCSDDwgParamEx->dStartChg = 0.0;
		m_pCSDDwgParamEx->dEndChg = 0.0;

		m_pCSDDwgParamEx->dSheetWid = 1000.0;
		m_pCSDDwgParamEx->dSheetHeight = 900.0;
		m_pCSDDwgParamEx->dNPlateWid = 250.0;
		m_pCSDDwgParamEx->dNPlateHeight = 150.0;

		strcpy(m_pCSDDwgParamEx->szElevTag, "Existing R.L.(in M)");
		strcpy(m_pCSDDwgParamEx->szElevTagDsgn, "Proposed R.L.(in M)");
		
		strcpy(m_pCSDDwgParamEx->szOffsetTag, "Distance in Metre");
		strcpy(m_pCSDDwgParamEx->szDatumTag, "Datum = ");
		strcpy(m_pCSDDwgParamEx->szXSecLabelTag, "C/S at Ch:");

		strcpy(m_pCSDDwgParamEx->szDataFile, "");
		strcpy(m_pCSDDwgParamEx->szDataFileDsgn, "");
		strcpy(m_pCSDDwgParamEx->szDataFileOut, "");

		strcpy(m_pCSDDwgParamEx->szDwgSheet,				"XSECN_SHEET");
		strcpy(m_pCSDDwgParamEx->szLyrAxis,				"XSECN_AXIS");
		strcpy(m_pCSDDwgParamEx->szLyrAnnotation,			"XSECN_ANNOTATION_EXISTING");
		strcpy(m_pCSDDwgParamEx->szLyrTopSurface,			"XSECN_TOP_SURFACE_EXISTING");
		strcpy(m_pCSDDwgParamEx->szLyrOrdinate,			"XSECN_ORDINATE_EXISTING");

		strcpy(m_pCSDDwgParamEx->szLyrAnnotationDsgn,			"XSECN_ANNOTATION_DSGN");	
		strcpy(m_pCSDDwgParamEx->szLyrTopSurfaceDsgn,			"XSECN_TOP_SURFACE_DSGN");	
		strcpy(m_pCSDDwgParamEx->szLyrOrdinateDsgn,			"XSECN_ORDINATE_DSGN");	
		//Existing Top Surface Related.......
		strcpy(m_pCSDDwgParamEx->szXistTopSurfAnnBlk,			"");
		m_pCSDDwgParamEx->dXistTopSurfAnnBlkScale = 1.0;
		m_pCSDDwgParamEx->dXistTopSurfAnnBlkSpaceMin = 1.0;
		//Proposed Top Surface Related.......UNUSED...KEPT FOR FUTURE.......
		strcpy(m_pCSDDwgParamEx->szPropTopSurfAnnBlk,			"");
		m_pCSDDwgParamEx->dPropTopSurfAnnBlkScale = 1.0;
		m_pCSDDwgParamEx->dPropTopSurfAnnBlkScaleMin = 1.0;

#ifdef _DEBUG
		m_pCSDDwgParamEx->dYScale = 1.0;
		m_pCSDDwgParamEx->dXScale = 2.0;
		strcpy(m_pCSDDwgParamEx->szDataFile,		"D:\\Sample Data\\CSecn\\New Version/Field Data.csd");
		strcpy(m_pCSDDwgParamEx->szDataFileDsgn,	"D:\\Sample Data\\CSecn\\New Version/Design Data.csd");
		strcpy(m_pCSDDwgParamEx->szDataFileOut,		"D:\\Sample Data\\CSecn\\New Version/junk.txt");
		m_pCSDDwgParamEx->dNumAnnSize = 1.5;
		m_pCSDDwgParamEx->dRowAnnSize = 3.0;
		m_pCSDDwgParamEx->dRowHt = 15.0;
		m_pCSDDwgParamEx->dCSLabelAnnSize = 3.0;
#endif //_DEBUG
	}
	return m_pCSDDwgParamEx;
}
void CParameterDataCache::SetCSDDwgParamEx(const CSDDWGPARAMEX* pRec)
{
	GetCSDDwgParamEx();//Create object, if not already created!!
#ifdef _DEBUG //Not published yet!!
	*(m_pCSDDwgParamEx) = *pRec;
#endif//_DEBUG
}

const DRAWPARAMRDPLANFRTXTFILE* CParameterDataCache::GetDrawParamRDPlanFRTxtFile()
{
	if(0L == m_pDrawParamRDPlanFRTxtFile)
	{//Initialize default data...
		m_pDrawParamRDPlanFRTxtFile = new DRAWPARAMRDPLANFRTXTFILE;

		memset(m_pDrawParamRDPlanFRTxtFile, 0, sizeof(DRAWPARAMRDPLANFRTXTFILE));

		m_pDrawParamRDPlanFRTxtFile->bConsiderZ = FALSE;
		m_pDrawParamRDPlanFRTxtFile->dNumAnnSize = TEXTSIZE();
		m_pDrawParamRDPlanFRTxtFile->dBlkScaleX = m_pDrawParamRDPlanFRTxtFile->dNumAnnSize / 2.0;
		m_pDrawParamRDPlanFRTxtFile->dBlkScaleY = m_pDrawParamRDPlanFRTxtFile->dNumAnnSize / 2.0;
	}
	return m_pDrawParamRDPlanFRTxtFile;
}
void CParameterDataCache::SetDrawParamRDPlanFRTxtFile(const DRAWPARAMRDPLANFRTXTFILE* pRec)
{
	GetDrawParamRDPlanFRTxtFile();//Create object, if not already created!!
#ifdef _DEBUG //Not published yet!!
	*(m_pDrawParamRDPlanFRTxtFile) = *pRec;
#endif//_DEBUG
}
//
const INSBLKFRFILEPARAMS* CParameterDataCache::GetInsBlkFrFileParam()
{
	if(0L == m_pInsBlkFrFileParams)
	{//Initialize default data...
		m_pInsBlkFrFileParams = new INSBLKFRFILEPARAMS;

		m_pInsBlkFrFileParams->dBlkScaleX = 1.0;
		m_pInsBlkFrFileParams->dBlkScaleY = 1.0;
		m_pInsBlkFrFileParams->dBlkRotAngle = DTOR(45.0);
		m_pInsBlkFrFileParams->bInsConsideringZValue = TRUE;

		m_pInsBlkFrFileParams->dTxtSize = 2.0;
		m_pInsBlkFrFileParams->dTxtRotation = DTOR(45.0);

		m_pInsBlkFrFileParams->bHasDataZValue = TRUE;

		strcpy(m_pInsBlkFrFileParams->szBlkName,		"");
		strcpy(m_pInsBlkFrFileParams->szLyrAnnotation,	"INSERTED_BLKS");
		strcpy(m_pInsBlkFrFileParams->szDataFile,		"");
	}
	return m_pInsBlkFrFileParams;
}
void CParameterDataCache::SetInsBlkFrFileParam(const INSBLKFRFILEPARAMS* pRec)
{
	GetInsBlkFrFileParam();//Create object, if not already created!!
#ifdef _DEBUG //Not published yet!!
	*(m_pInsBlkFrFileParams) = *pRec;
#endif//_DEBUG
}

//
const FBDWGPARAM* CParameterDataCache::GetFBookDwgParam()
{
	if(0L == m_pFBookDwgParam)
	{//Initialize default data...
		m_pFBookDwgParam = new FBDWGPARAM;

		m_pFBookDwgParam->fTxtSize = TEXTSIZE();
		m_pFBookDwgParam->fBlkScale = TEXTSIZE();
		strcpy(m_pFBookDwgParam->szLyr, "FB_SHEET");
	}
	return m_pFBookDwgParam;
}
void CParameterDataCache::SetFBookDwgParam(const FBDWGPARAM* pRec)
{
	GetFBookDwgParam();//Create object, if not already created!!
#ifdef _DEBUG //Not published yet!!
	*(m_pFBookDwgParam) = *pRec;
#endif//_DEBUG
}
//
const XLSXSECDWGPARAM* CParameterDataCache::GetXlsXSecDwgParam()
{
	if(0L == m_pXlsXSecDwgParam)
	{//Initialize default data...
		m_pXlsXSecDwgParam = new XLSXSECDWGPARAM;

		m_pXlsXSecDwgParam->dXScale = 1.0;
		m_pXlsXSecDwgParam->dYScale = 10.0;

		m_pXlsXSecDwgParam->dNumAnnSize = 2.0;
		m_pXlsXSecDwgParam->dRowAnnSize = 3.0;
		m_pXlsXSecDwgParam->dRowHt = 15.0;
		m_pXlsXSecDwgParam->dCSLabelAnnSize = 4.0;

		m_pXlsXSecDwgParam->dDatum = 0.0;

		m_pXlsXSecDwgParam->dSheetWid = 1000.0;
		m_pXlsXSecDwgParam->dSheetHeight = 900.0;
		m_pXlsXSecDwgParam->dNPlateWid = 250.0;
		m_pXlsXSecDwgParam->dNPlateHeight = 150.0;

		strcpy(m_pXlsXSecDwgParam->szElevTag,		"R.L.(in M)");
		strcpy(m_pXlsXSecDwgParam->szNorthingTag,	"Northing (in M)");
		strcpy(m_pXlsXSecDwgParam->szEastingTag,	"Easting (in M)");
		strcpy(m_pXlsXSecDwgParam->szOffsetTag,		"Distance in Metre");
		
		strcpy(m_pXlsXSecDwgParam->szDatumTag,		"Datum = ");
		strcpy(m_pXlsXSecDwgParam->szXSecLabelTag,	"C/S at Ch:");

		strcpy(m_pXlsXSecDwgParam->szLyrDwgSheet,			"XSECN_SHEET");
		strcpy(m_pXlsXSecDwgParam->szLyrAxis,				"XSECN_AXIS");
		strcpy(m_pXlsXSecDwgParam->szLyrAnnotation,			"XSECN_ANNOTATION");
		strcpy(m_pXlsXSecDwgParam->szLyrTopSurface,			"XSECN_TOP_SURFACE");
		strcpy(m_pXlsXSecDwgParam->szLyrOrdinate,			"XSECN_ORDINATE");
		
		//Top Surface Related.......
		strcpy(m_pXlsXSecDwgParam->szXistTopSurfAnnBlk,			"");
		m_pXlsXSecDwgParam->dXistTopSurfAnnBlkScale = 1.0;
		m_pXlsXSecDwgParam->dXistTopSurfAnnBlkSpaceMin = 1.0;
	}
	return m_pXlsXSecDwgParam;
}
void CParameterDataCache::SetXlsXSecDwgParam(const XLSXSECDWGPARAM* pRec)
{
	GetXlsXSecDwgParam();//Create object, if not already created!!
#ifdef _DEBUG //Not published yet!!
	*(m_pXlsXSecDwgParam) = *pRec;
#endif//_DEBUG
}
//
const RDPFL_CHG_Z_DWG_PARAM* CParameterDataCache::GetRDPflChgZDwgParam()
{
	if(0L == m_pRDPflChgZDwgParam)
	{//Initialize default data...
		m_pRDPflChgZDwgParam = new RDPFL_CHG_Z_DWG_PARAM;
		memset(m_pRDPflChgZDwgParam, 0, sizeof(RDPFL_CHG_Z_DWG_PARAM));

		m_pRDPflChgZDwgParam->dXScale = 1.0;
		m_pRDPflChgZDwgParam->dYScale = 10.0;

		m_pRDPflChgZDwgParam->dNumAnnTxtSize		= 2.0;
		m_pRDPflChgZDwgParam->dLabTxtSize			= 4.0;
		m_pRDPflChgZDwgParam->dRowHt				= 15.0;
		m_pRDPflChgZDwgParam->dDatum				= 0.0;

		m_pRDPflChgZDwgParam->fOrdInterval					= 5.0;
		m_pRDPflChgZDwgParam->fTxtAnnotationAngleInDeg		= 45.0;
		m_pRDPflChgZDwgParam->fTxtAnnotationMargin			= 10.0;

		strcpy(m_pRDPflChgZDwgParam->szXElevTag,	"Existing Elevation");
		strcpy(m_pRDPflChgZDwgParam->szDsgnElevTag,	"Proposed Elevation");
		strcpy(m_pRDPflChgZDwgParam->szCutFillTag,	"Cut/Fill");
		strcpy(m_pRDPflChgZDwgParam->szDatum,		"Datum = ");
		strcpy(m_pRDPflChgZDwgParam->szChgTag,		"Chainage:");
		strcpy(m_pRDPflChgZDwgParam->szExistingDsgnElevTag,		"Existing Design Elevation");
		strcpy(m_pRDPflChgZDwgParam->szDsgnElevDifferenceTag,	"Design Elevation Difference");

		strcpy(m_pRDPflChgZDwgParam->szLyrAnnotation,					"PFL_ANNOTATION");
		strcpy(m_pRDPflChgZDwgParam->szLyrTopSurfaceOrg,				"PFL_TOP_SURFACE_EXISTING");
		strcpy(m_pRDPflChgZDwgParam->szLyrTopSurfaceDsgn,				"PFL_TOP_SURFACE_DESIGN");
		strcpy(m_pRDPflChgZDwgParam->szLyrOrdinate,						"PFL_ORDINATE");
		strcpy(m_pRDPflChgZDwgParam->szLyrAxis,							"PFL_AXIS");
		strcpy(m_pRDPflChgZDwgParam->szLyrAnnotationEx,					"PFL_ANNOTATION_TOP_SURFACE");
		strcpy(m_pRDPflChgZDwgParam->szLyrTopSurfaceDsgnExisting,		"PFL_TOP_SURFACE_EXISTING_DESIGN");
		
		//Top Surface Related.......
		strcpy(m_pRDPflChgZDwgParam->szXistTopSurfAnnBlk,			"");
		m_pRDPflChgZDwgParam->dXistTopSurfAnnBlkScale = 1.0;
		m_pRDPflChgZDwgParam->dXistTopSurfAnnBlkSpaceMin = 1.0;
	}
	return m_pRDPflChgZDwgParam;
}
void CParameterDataCache::SetRDPflChgZDwgParam(const RDPFL_CHG_Z_DWG_PARAM* pRec)
{
	GetRDPflChgZDwgParam();//Create object, if not already created!!

	*(m_pRDPflChgZDwgParam) = *pRec;
}
//
//
const TRAVDWGPARAM* CParameterDataCache::GetTravDwgParam()
{
	if(0L == m_pTravDwgParam)
	{//Initialize default data...
		m_pTravDwgParam = new TRAVDWGPARAM;

		m_pTravDwgParam->fStnMarkTxtSize = TEXTSIZE();
		m_pTravDwgParam->ptTravBase		= AcGePoint3d(0.0, 0.0, 0.0);
		m_pTravDwgParam->fTableTxtSize	= TEXTSIZE();
		m_pTravDwgParam->fTableMargin	= TEXTSIZE() * 1.5;
		m_pTravDwgParam->ptTableBase		= AcGePoint3d(0.0, 0.0, 0.0);
		strcpy(m_pTravDwgParam->szOutFile, "");
		strcpy(m_pTravDwgParam->szInFile, "");
		
		strcpy(m_pTravDwgParam->szLyrCalcTable,		"TRAV_CALC_TABLE");
		strcpy(m_pTravDwgParam->szLyrTraverseOrg,	"TRAV_DWG_FIELD");
		strcpy(m_pTravDwgParam->szLyrTraverseAdj,	"TRAV_DWG_ADJUSTED");

#ifdef _DEBUG	
		strcpy(m_pTravDwgParam->szInFile, "C:/temp/SurveyData/TraverseAdj.txt");
		strcpy(m_pTravDwgParam->szOutFile, "C:/temp/SurveyData/Junk");

		strcpy(m_pTravDwgParam->szInFile, "C:/temp/SurveyData/TraverseAdj.txt");
		strcpy(m_pTravDwgParam->szOutFile, "C:/temp/SurveyData/junk.txt");

		strcpy(m_pTravDwgParam->szInFile, "C:/temp/SurveyData/XLS Sample/Open Traverse.xls");
		strcpy(m_pTravDwgParam->szOutFile, "C:/temp/SurveyData/junk.txt");
#endif //_DEBUG	
	}
	return m_pTravDwgParam;
}
void CParameterDataCache::SetTravDwgParam(const TRAVDWGPARAM* pRec)
{
	GetTravDwgParam();//Create object, if not already created!!
#ifdef _DEBUG //Not published yet!!
	*(m_pTravDwgParam) = *pRec;
#endif//_DEBUG
}


