// DrawUtil.cpp
/////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DrawUtil.h"
#include "Util.h"
#include <string.h>
#include "Structures.h"
#include "AcDbObjectEx.h"

static Acad::ErrorStatus IsLyrExists(const char* pszLyr, AcDbObjectId* = 0L);
static int FindNearestPoint(const AcGePoint3d&, const AcGePoint3dArray&);

int PROXYNOTICE(const short& iNewVal/*= -1*/)
{
	struct resbuf rb;
	if(iNewVal < 0)
	{
		ads_getvar("PROXYNOTICE", &rb);
		return rb.resval.rint;
	}
	rb.restype = RTSHORT;
	rb.resval.rint = iNewVal;
	ads_setvar("PROXYNOTICE", &rb);
	return 1;
}
int EXPERT()
{
	struct resbuf rb;
	ads_getvar("EXPERT", &rb);
	return rb.resval.rint;
}

int LUPREC(const int& iNewSetting /*= -1*/)
{
	short iPrec;
	AcDbDatabase*	pDb;

	pDb = acdbCurDwg();
	if(pDb == 0L)
		return -1;
	
	iPrec = pDb->luprec();
	if((iNewSetting >= 0) && (iNewSetting != iPrec))
		pDb->setLuprec((short)iNewSetting);
	
	return iPrec;
}
double TEXTSIZE(const double& fVal/*= -1.0*/)
{
	double fCurVal;
	AcDbDatabase*	pDb;

	pDb = acdbCurDwg();
	if(pDb == 0L)
		return -1;
	
	fCurVal = pDb->textsize();
	if((fVal > 0.0) && (fCurVal != fVal))
		pDb->setTextsize(fVal);
	
	return fCurVal;
}
BOOL TEXTSTYLE(AcDbObjectId& ID)
{
	AcDbDatabase*	pDb;

	pDb = acdbCurDwg();
	if(pDb == 0L)
		return FALSE;
	
	ID = pDb->textstyle();
	return TRUE;
}
BOOL CLAYER(CString& strCurLyr, const char* pszNewLyr /* = 0L*/)
{
	AcDbDatabase*	pDb;
	Acad::ErrorStatus		ErrCode;

	pDb = acdbCurDwg();
	if(pDb == 0L)
		return FALSE;
    
	AcDbObjectId LyrID;
	
	LyrID = pDb->clayer();
	
	{//Extract the name of the layer....
		AcDbLayerTable*			pLyrTable;

		ErrCode = pDb->getLayerTable(pLyrTable, AcDb::kForRead);
		if(ErrCode != Acad::eOk)
			return ErrCode;//ERROR!!

		{//
			AcDbLayerTableIterator* pIterator;
			AcDbLayerTableRecord*	pLyrDet;
			const char* pszLyr;
		
			pLyrTable->newIterator(pIterator);
			pIterator->seek(LyrID);
			pIterator->getRecord(pLyrDet, AcDb::kForRead);
			delete pIterator;
			
		    ErrCode = pLyrDet->getName(pszLyr);
			if(ErrCode != Acad::eOk)
			{
				ads_printf("\nERROR: AutoCAD system error occured");
			}
			strCurLyr = pszLyr;
			pLyrDet->close();
    	}
		pLyrTable->close();
	}
	if(pszNewLyr != 0L)
	{
		AcDbObjectId DbObjId;
		
		ErrCode = IsLyrExists(pszNewLyr, &DbObjId);
		if(Acad::eOk == ErrCode)
		{//Just Set
		}
		else
		{//Create ...
			ErrCode = LAYER(pszNewLyr, -1, &DbObjId);
			if(Acad::eOk != ErrCode)
			{
				ads_printf("\nERROR: \"%s\" layer creation failed", pszNewLyr);
				return FALSE;
			}
		}
		pDb->setClayer(DbObjId);
	}
	return TRUE;
}

Acad::ErrorStatus LAYER(const char* pszLyr, const int& iColor/* = -1*/, AcDbObjectId* pDbObjId /* = 0L*/)
{
	Acad::ErrorStatus ErrCode;
	
	ErrCode = IsLyrExists(pszLyr, pDbObjId);
	if(Acad::eOk == ErrCode)
		return ErrCode;//Layer already exists....

	{//Layer doesn't exists.. so create..
		AcDbDatabase*	pDb;
		AcDbLayerTable* pLyrTable;
		AcDbLayerTableRecord* pLyrDet;

		pDb = acdbCurDwg();

		ErrCode = pDb->getLayerTable(pLyrTable, AcDb::kForWrite);
		if(ErrCode != Acad::eOk)
			return ErrCode;//ERROR!!
		//Initialize Record....
		pLyrDet = new AcDbLayerTableRecord;
		ErrCode = pLyrDet->setName(pszLyr);
		if(iColor != -1)
		{
			AcCmColor Color;
			
			Color.setColorIndex((short)iColor);
			pLyrDet->setColor(Color);
		}
		//Initialize Record....
		if(ErrCode == Acad::eOk)
		{
			ErrCode = pLyrTable->add(pLyrDet);
			if(ErrCode != Acad::eOk)
				delete pLyrDet;

		}
		pLyrTable->close();
		pLyrDet->close();
		return ErrCode;
	}
}
static Acad::ErrorStatus IsLyrExists(const char* pszLyr, AcDbObjectId* pDbObjId /* = 0L*/)
{
	Acad::ErrorStatus ErrCode;
	AcDbDatabase*	pDb;
	AcDbLayerTable* pLyrTable;

	pDb = acdbCurDwg();

	ErrCode = pDb->getLayerTable(pLyrTable, AcDb::kForRead);
	if(Acad::eOk != ErrCode)
	{
		pLyrTable->close();
		return ErrCode;//ERROR!!
	}
	if(pDbObjId != 0L)
	{
		ErrCode = pLyrTable->getAt(pszLyr, *pDbObjId, Adesk::kTrue);
	}
	else
	{
		AcDbObjectId DbObjId;
		ErrCode = pLyrTable->getAt(pszLyr, DbObjId, Adesk::kTrue);
	}
	pLyrTable->close();
	return ErrCode;
}
int DRAWPOINTS(const AcGePoint3dArray& Array, const char* pszLyr)
{
	int i;

	for(i = 0; i < Array.length(); i++)
	{
		if(!DRAWPOINT(Array[i], pszLyr))
			return -1;
	}
	return Array.length();
}
BOOL DRAWPOINT(const AcGePoint3d& Pt, const char* pszLyr)
{
	Acad::ErrorStatus ErrCode;
	
	ErrCode = IsLyrExists(pszLyr);
	if(Acad::eOk != ErrCode)
		return FALSE;//Layer doesn't exists....

    AcDbDatabase* pDb;
    AcDbBlockTable* pBtbl;
    AcDbBlockTableRecord* pBtblRcd;
	AcDbPoint* pPoint;

	pDb = acdbCurDwg();
	//Initialize Point data..
	pPoint = new AcDbPoint();
	pPoint->setPosition(Pt);
	pPoint->setLayer(pszLyr);
	//Initialize Point data..

    pDb->getBlockTable(pBtbl, AcDb::kForRead);
 
    pBtbl->getAt(ACDB_MODEL_SPACE, pBtblRcd, AcDb::kForWrite);
    pBtbl->close();
 
    pBtblRcd->appendAcDbEntity(pPoint);
    pPoint->close();
    
    pBtblRcd->close();

	return TRUE;
}
void LINES(const AcGePoint3dArray& Array, const char* pszLyr)
{
	Acad::ErrorStatus ErrCode;
	int i;
	
	ErrCode = IsLyrExists(pszLyr);
	if(Acad::eOk != ErrCode)
		return;//Layer doesn't exists....
	
	for(i = 1; i < Array.length(); i++)
	{
		LINE(Array[i - 1], Array[i - 0], pszLyr);
	}
}
void LINE(const AcGePoint3d& PtStart, const AcGePoint3d& PtEnd, const char* pszLyr)
{
	Acad::ErrorStatus ErrCode;
	
	ErrCode = IsLyrExists(pszLyr);
	if(Acad::eOk != ErrCode)
		return;//Layer doesn't exists....
	/////
    AcDbDatabase* pDb;
    AcDbBlockTable* pBtbl;
    AcDbBlockTableRecord* pBtblRcd;
	AcDbLine* pEntity;

	pDb = acdbCurDwg();
	//Initialize Line data..
	pEntity = new AcDbLine();
	pEntity->setStartPoint(PtStart);
	pEntity->setEndPoint(PtEnd);
	pEntity->setLayer(pszLyr);
	//Initialize Line data..

    pDb->getBlockTable(pBtbl, AcDb::kForRead);
 
    pBtbl->getAt(ACDB_MODEL_SPACE, pBtblRcd, AcDb::kForWrite);
    pBtbl->close();
 
    pBtblRcd->appendAcDbEntity(pEntity);
    pEntity->close();
    
    pBtblRcd->close();
}
void CIRCLE(const AcGePoint3d& ptParam, const double& fRad, const char* pszLyr)
{
	Acad::ErrorStatus ErrCode;
	
	ErrCode = IsLyrExists(pszLyr);
	if(Acad::eOk != ErrCode)
		return;//Layer doesn't exists....
	/////
    AcDbDatabase* pDb;
    AcDbBlockTable* pBtbl;
    AcDbBlockTableRecord* pBtblRcd;
	AcDbCircle* pEntity;

	pDb = acdbCurDwg();
	//Initialize Circle data..
	pEntity = new AcDbCircle();
	pEntity->setCenter(ptParam);
	pEntity->setRadius(fRad);
	pEntity->setLayer(pszLyr);
	//Initialize Circle data..

    pDb->getBlockTable(pBtbl, AcDb::kForRead);
 
    pBtbl->getAt(ACDB_MODEL_SPACE, pBtblRcd, AcDb::kForWrite);
    pBtbl->close();
 
    pBtblRcd->appendAcDbEntity(pEntity);
    pEntity->close();
    
    pBtblRcd->close();
}
void BLOCK(const char* pszBlkName, const AcGePoint3d& Pt, const char* pszLyr /* = 0L*/, const double& fRotation /* = 0.0*/, const double& fX /* = 1.0*/, const double& fY /* = 1.0*/, const double& fZ /* = 1.0*/)
{
	Acad::ErrorStatus ErrCode;
	AcDbObjectId ObjectId;

	if(pszLyr != 0L)
	{
		ErrCode = IsLyrExists(pszLyr);
		if(Acad::eOk != ErrCode)
		{
			ads_printf("\nERROR: Layer \"%s\" doesn't exists\n", pszLyr);
			return;//Layer doesn't exists....
		}
	}

	if(!IsBlockExists(pszBlkName, &ObjectId))
	{
		ads_printf("\nERROR: Block \"%s\" doesn't exists\n", pszBlkName);
		return;//Block doesn't exists....
	}
	/////
    AcDbDatabase* pDb;
    AcDbBlockTable* pBtbl;
    AcDbBlockTableRecord* pBtblRcd;
	AcDbBlockReference* pEntity;

	pDb = acdbCurDwg();
	//Initialize INSERT data..
	pEntity = new AcDbBlockReference();
	pEntity->setBlockTableRecord(ObjectId);
	pEntity->setPosition(Pt);
	if(pszLyr != 0L)
		pEntity->setLayer(pszLyr);
	pEntity->setScaleFactors(AcGeScale3d(fX, fY, fZ));
	pEntity->setRotation(fRotation);
	//Initialize INSERT data..

    pDb->getBlockTable(pBtbl, AcDb::kForRead);
 
    pBtbl->getAt(ACDB_MODEL_SPACE, pBtblRcd, AcDb::kForWrite);
    pBtbl->close();
 
    pBtblRcd->appendAcDbEntity(pEntity);
    pEntity->close();
    
    pBtblRcd->close();
}
BOOL IsBlockExists(const char* pszBlk, AcDbObjectId* pObjectId /* = 0L*/)
{
	Acad::ErrorStatus ErrCode;
    AcDbDatabase* pDb;
    AcDbBlockTable* pBtbl;
	AcDbBlockTableIterator* pIterator;
	BOOL bFound;
	
	pDb = acdbCurDwg();

    pDb->getBlockTable(pBtbl, AcDb::kForRead);
    ErrCode = pBtbl->newIterator(pIterator);
	pBtbl->close();
	if(Acad::eOk != ErrCode)
		return FALSE;//Some errors occured....
 
	for(pIterator->start(), bFound = FALSE; (pIterator->done() == Adesk::kFalse); pIterator->step())
	{
		AcDbBlockTableRecord* pBlkRec;
		const char* pszName;
	    
		ErrCode = pIterator->getRecord(pBlkRec, AcDb::kForRead);
 		if(Acad::eOk != ErrCode)
		{
			delete pIterator;
			return FALSE;//Some errors occured....
		}
		pBlkRec->getName(pszName);
		pBlkRec->close();
		if(strcmp(pszName, pszBlk) == 0)
		{//Found....
			bFound = TRUE;
			if(pObjectId != 0L)
				*pObjectId = pBlkRec->objectId();
			break;
		}
	}
	delete pIterator;
	return bFound;
}
BOOL GetBlockNames(CStringArray& BlkNameArray)
{
	Acad::ErrorStatus ErrCode;
    AcDbDatabase* pDb;
    AcDbBlockTable* pBtbl;
	AcDbBlockTableIterator* pIterator;
	BOOL bFound;
	
	pDb = acdbCurDwg();

    pDb->getBlockTable(pBtbl, AcDb::kForRead);
    ErrCode = pBtbl->newIterator(pIterator);
	pBtbl->close();
	if(Acad::eOk != ErrCode)
		return FALSE;//Some errors occured....
 
	for(pIterator->start(), bFound = FALSE; (pIterator->done() == Adesk::kFalse); pIterator->step())
	{
		AcDbBlockTableRecord* pBlkRec;
		const char* pszName;
	    
		ErrCode = pIterator->getRecord(pBlkRec, AcDb::kForRead);
 		if(Acad::eOk != ErrCode)
		{
			delete pIterator;
			return FALSE;//Some errors occured....
		}
		if(!pBlkRec->isAnonymous() && !pBlkRec->hasAttributeDefinitions())
		{
			pBlkRec->getName(pszName);
			if(*(pszName+0) != '*')
				BlkNameArray.Add(pszName);
		}
		pBlkRec->close();
	}
	delete pIterator;
	return bFound;
}

BOOL DrawSpotLevel(const AcGePoint3d& Pt, const char* pszBlkName, const char* pszLyr, const double& fRotation, const double& fScale, const double& fBlkScaleFac)
{
	Acad::ErrorStatus ErrCode;
	char szTmp[_MAX_PATH];
	const double fSpacingMFac = 1.05;
	const char chDot = '.';
	ads_point ptIns;
	int iIndex, iLuPrec;

	if(!IsBlockExists(pszBlkName))
	{
		ads_printf("\nERROR: Block \"%s\" doesn't exists\n", pszBlkName);
		return FALSE;//Block doesn't exists....
	}
	ErrCode = IsLyrExists(pszLyr);
	if(Acad::eOk != ErrCode)
	{//Create
		ErrCode = LAYER(pszLyr);
		if(Acad::eOk != ErrCode)
		{
			ads_printf("\nERROR: \"%s\" layer creation failed", pszLyr);
			return FALSE;
		}
	}
	iLuPrec = LUPREC();
	if(iLuPrec <= 0) iLuPrec = 1;

	BLOCK(pszBlkName, Pt, pszLyr, fRotation, (fScale * fBlkScaleFac), (fScale * fBlkScaleFac), (fScale * fBlkScaleFac));
	{//Draw Left String....
		CString strTmp;

		strTmp = RTOS(Pt.z);
		iIndex = strTmp.Find(chDot);
		strTmp = strTmp.Mid(0, iIndex);
		
		ads_polar(asDblArray(Pt), (fRotation + PI), (fScale * fBlkScaleFac * fSpacingMFac), ptIns);
		TEXTRIGHT(asPnt3d(ptIns), strTmp, fRotation, fScale, pszLyr);
	}
	{//Draw Right String....
		CString strTmp;

		if(iLuPrec <= 0)
		{
			ads_rtos(Pt.z, 2, 1, szTmp);
			strTmp = szTmp;
		}
		else
		{
			strTmp = RTOS(Pt.z);
		}
		iIndex = strTmp.Find(chDot);
		strTmp = strTmp.Mid(iIndex + 1);
		
		//Remove Trailing Zeros....
		//for(i = strlen(strTmp) - 1; i >= 0; i--)
		//{
		//	if(strTmp[i] != '0')
		//	{
		//		strTmp = strTmp.Mid(0, i+1);
		//		break;
		//	}
		//}
		//if(i < 0)
		//{
		//	strTmp = "0";
		//}
		ads_polar(asDblArray(Pt), fRotation, (fScale * fBlkScaleFac * fSpacingMFac), ptIns);
		TEXTLEFT(asPnt3d(ptIns), strTmp, fRotation, fScale, pszLyr);
	}
	return TRUE;
}


void TEXTMID(const AcGePoint3d& Pt, const char* pszTxt, const double& fRotation, const double& fHt, const char* pszLyr)
{
	Acad::ErrorStatus ErrCode;
	
	ErrCode = IsLyrExists(pszLyr);
	if(Acad::eOk != ErrCode)
		return;//Layer doesn't exists....
	/////
    AcDbDatabase* pDb;
    AcDbBlockTable* pBtbl;
    AcDbBlockTableRecord* pBtblRcd;
	AcDbText* pEntity;

	pDb = acdbCurDwg();
	//Initialize data..
	pEntity = new AcDbText();

	pEntity->setPosition(Pt);
	pEntity->setRotation(fRotation);
	pEntity->setHeight(fHt);
	pEntity->setTextString(pszTxt);
	pEntity->setHorizontalMode(AcDb::TextHorzMode::kTextCenter);
	pEntity->setVerticalMode(AcDb::TextVertMode::kTextVertMid);
	pEntity->setAlignmentPoint(Pt);
	pEntity->setLayer(pszLyr);
	//Initialize data..

    pDb->getBlockTable(pBtbl, AcDb::kForRead);
 
    pBtbl->getAt(ACDB_MODEL_SPACE, pBtblRcd, AcDb::kForWrite);
    pBtbl->close();
 
    pBtblRcd->appendAcDbEntity(pEntity);
    pEntity->close();
    
    pBtblRcd->close();
}
void TEXTLEFT(const AcGePoint3d& Pt, const char* pszTxt, const double& fRotation, const double& fHt, const char* pszLyr)
{
	Acad::ErrorStatus ErrCode;
	
	ErrCode = IsLyrExists(pszLyr);
	if(Acad::eOk != ErrCode)
		return;//Layer doesn't exists....
	/////
    AcDbDatabase* pDb;
    AcDbBlockTable* pBtbl;
    AcDbBlockTableRecord* pBtblRcd;
	AcDbText* pEntity;

	pDb = acdbCurDwg();
	//Initialize data..
	pEntity = new AcDbText();
	pEntity->setHorizontalMode(AcDb::TextHorzMode::kTextLeft);
	pEntity->setVerticalMode(AcDb::TextVertMode::kTextVertMid);
	pEntity->setRotation(fRotation);
	pEntity->setHeight(fHt);
	pEntity->setTextString(pszTxt);
	pEntity->setLayer(pszLyr);
	pEntity->setAlignmentPoint(Pt);
	//Initialize data..

    pDb->getBlockTable(pBtbl, AcDb::kForRead);
 
    pBtbl->getAt(ACDB_MODEL_SPACE, pBtblRcd, AcDb::kForWrite);
    pBtbl->close();
 
    pBtblRcd->appendAcDbEntity(pEntity);
    pEntity->close();
    
    pBtblRcd->close();
}
void TEXTRIGHT(const AcGePoint3d& Pt, const char* pszTxt, const double& fRotation, const double& fHt, const char* pszLyr)
{
	Acad::ErrorStatus ErrCode;
	
	ErrCode = IsLyrExists(pszLyr);
	if(Acad::eOk != ErrCode)
		return;//Layer doesn't exists....
	/////
    AcDbDatabase* pDb;
    AcDbBlockTable* pBtbl;
    AcDbBlockTableRecord* pBtblRcd;
	AcDbText* pEntity;

	pDb = acdbCurDwg();
	//Initialize data..
	pEntity = new AcDbText();
	pEntity->setHorizontalMode(AcDb::TextHorzMode::kTextRight);
	pEntity->setVerticalMode(AcDb::TextVertMode::kTextVertMid);
	pEntity->setRotation(fRotation);
	pEntity->setHeight(fHt);
	pEntity->setTextString(pszTxt);
	pEntity->setLayer(pszLyr);
	pEntity->setAlignmentPoint(Pt);
	//Initialize data..

    pDb->getBlockTable(pBtbl, AcDb::kForRead);
 
    pBtbl->getAt(ACDB_MODEL_SPACE, pBtblRcd, AcDb::kForWrite);
    pBtbl->close();
 
    pBtblRcd->appendAcDbEntity(pEntity);
    pEntity->close();
    
    pBtblRcd->close();
}
void POLYLINE(AcGePoint3dArray& Array, const char* pszLyr, const BOOL& bIs3D/* = FALSE*/)
{
	Acad::ErrorStatus ErrCode;
	
	ErrCode = IsLyrExists(pszLyr);
	if(Acad::eOk != ErrCode)
		return;//Layer doesn't exists....

	if(Array.length() <= 0)
		return;//Drawing not possible

    AcDbDatabase* pDb;
    AcDbBlockTable* pBtbl;
    AcDbBlockTableRecord* pBtblRcd;
 
	pDb = acdbCurDwg();

	if(!bIs3D)
	{
		int i;
		AcDbPolyline* pNewEnt;
		pNewEnt = new AcDbPolyline();
		pNewEnt->setLayer(pszLyr);

		for(i = 0; i < Array.length(); i++)
		{
			pNewEnt->addVertexAt(i, AcGePoint2d(Array[i].x, Array[i].y));
		}

		pDb->getBlockTable(pBtbl, AcDb::kForRead);
		pBtbl->getAt(ACDB_MODEL_SPACE, pBtblRcd, AcDb::kForWrite);
		pBtbl->close();
		pBtblRcd->appendAcDbEntity(pNewEnt);
		pNewEnt->close();
		pBtblRcd->close();
		//Initialize data..
		/*
		AcDb2dPolyline* pNewEnt;
		pNewEnt = new AcDb2dPolyline(AcDb::Poly2dType::k2dSimplePoly, Array);
		pNewEnt->setLayer(pszLyr);

		pDb->getBlockTable(pBtbl, AcDb::kForRead);
		pBtbl->getAt(ACDB_MODEL_SPACE, pBtblRcd, AcDb::kForWrite);
		pBtbl->close();
		pBtblRcd->appendAcDbEntity(pNewEnt);
		pNewEnt->close();
		pBtblRcd->close();
		*/
	}
	else
	{
		AcDb3dPolyline* pNewEnt;
		pNewEnt = new AcDb3dPolyline(AcDb::Poly3dType::k3dSimplePoly, Array);
		pNewEnt->setLayer(pszLyr);

		pDb->getBlockTable(pBtbl, AcDb::kForRead);
		pBtbl->getAt(ACDB_MODEL_SPACE, pBtblRcd, AcDb::kForWrite);
		pBtbl->close();
		pBtblRcd->appendAcDbEntity(pNewEnt);
		pNewEnt->close();
		pBtblRcd->close();
	}
}
void POLYLINE(const CPointPair3DPtrArray& Array, const char* pszLyr)
{
	Acad::ErrorStatus ErrCode;
	
	ErrCode = IsLyrExists(pszLyr);
	if(Acad::eOk != ErrCode)
		return;//Layer doesn't exists....

	if(Array.GetSize() < 1)
		return;//Drawing not possible

    AcDbDatabase* pDb;
    AcDbBlockTable* pBtbl;
    AcDbBlockTableRecord* pBtblRcd;
	Adesk::Boolean bIsClosed;
	AcGePoint3dArray Vertices;
	AcDb3dPolyline* pNewEnt;
 
	pDb = acdbCurDwg();

	bIsClosed = FALSE;
	ConvertPtPairsToVertices(Array, Vertices, &bIsClosed);
	//Initialize data..
	pNewEnt = new AcDb3dPolyline(AcDb::Poly3dType::k3dSimplePoly, Vertices, bIsClosed);
	pNewEnt->setLayer(pszLyr);
	//Initialize data..

    pDb->getBlockTable(pBtbl, AcDb::kForRead);
    pBtbl->getAt(ACDB_MODEL_SPACE, pBtblRcd, AcDb::kForWrite);
    pBtbl->close();
    pBtblRcd->appendAcDbEntity(pNewEnt);
    pNewEnt->close();
    pBtblRcd->close();
}
//NB: This func. frees the selset ...
BOOL EraseEntitiesInSelSet(ads_name& SelSet)
{
	//Erase Entity(s)....
	long i, iNoOfEnt;
	ads_name Entity;
	
	{//Erase all Lines
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
			if(ads_entdel(Entity) != RTNORM)
			{
				ads_printf("\nERROR: AutoCAD System error !\n");
				ads_ssfree(SelSet);//Free Selset
				return FALSE;//ERROR
			}
		}
	}
	ads_ssfree(SelSet);//Free Selset
	return TRUE;
}


//
static int FindNearestPoint(const AcGePoint3d& PtBase, const AcGePoint3dArray& PtArray)
{
	int i, iIndex;
	
	if(PtArray.length() <= 0)
		return -1;
	
	for(i = 1, iIndex = 0; i < PtArray.length(); i++)
	{
		if(Dist2D(PtBase, (PtArray[i])) < Dist2D(PtBase, (PtArray[iIndex])))
		{
			iIndex = i;
		}
	}
	return iIndex;
}
BOOL SortPointsOnLine(const AcGePoint3d& PtBase, AcGePoint3dArray& PtArray)
{
	int iIndex;
	AcGePoint3dArray PtArrayResult;
	AcGePoint3d PtBaseNew;

	if(PtArray.length() <= 0)
		return FALSE;

	PtBaseNew = PtBase;
	for(; PtArray.length() > 0;)
	{
		if(ads_usrbrk() == 1)
		{
			ads_printf("\nUser break\n");
			return FALSE;
		}
		iIndex = FindNearestPoint(PtBaseNew, PtArray);
		if(iIndex >= 0)
		{
			PtArrayResult.append(PtArray[iIndex]);
			PtBaseNew = PtArray[iIndex];
			PtArray.removeAt(iIndex);
		}
	}
	if(PtArray.length() != 0)
	{
		ads_printf("\nERROR: in 'SortPointsOnLine()'\n");
		return FALSE;
	}
	PtArray.append(PtArrayResult);
	return TRUE;
}
//Note: Function doesn't frees the parameter 'objSelSet'
BOOL ExtractPtsFromSelSet(ads_name& objSelSet, AcGePoint3dArray& arrayParam)
{
	long i, iNoOfEnt;

	if(arrayParam.length() > 0)
	{
		ads_printf("\nERROR: Invalid parameter passed TRACE: From >> ExtractPtsFromSelSet()!\n");
		return FALSE;
	}
	if(ads_sslength(objSelSet, &iNoOfEnt) != RTNORM)
	{
		ads_printf("\nERROR: AutoCAD System error !\n");
		return FALSE;
	}
	for(i = 0; i < iNoOfEnt; i++)
	{
		ads_name Entity;

		if(ads_ssname(objSelSet, i, Entity) != RTNORM)
		{
			ads_printf("\nERROR: AutoCAD System error !\n");
			return FALSE;
		}
		{//
			AcDbObjectId objId;
			Acad::ErrorStatus ErrCode;
			AcGePoint3d ptTmp;
			AcDbPoint* pPointEntity;

			ErrCode = acdbGetObjectId(objId, Entity);
			if(ErrCode != Acad::eOk)
			{
				ads_printf("\nERROR: AutoCAD System error !\n");
				return FALSE;
			}
			AcDbObjectEx obj(objId);

			pPointEntity = AcDbPoint::cast(obj.getObject());
			if(pPointEntity == 0L)
			{
				ads_printf("\nERROR: Entity selected is not a point\n");
				return FALSE;
			}
			arrayParam.append(pPointEntity->position());
		}
	}
	return TRUE;
}
BOOL SelectPoints(ads_name& objSelSet, const char* pszPromptForAdd/* = 0L*/, const char* pszPromptForRem/* = 0L*/)
{
	const char pszEntName[] = "POINT";
	struct  resbuf  Rb01;
	int iRet;
	
	// Entity type
	Rb01.restype = 0;
	Rb01.resval.rstring = (char*)pszEntName;
	Rb01.rbnext = 0L;
	
	iRet = RTERROR;
	if((0L != pszPromptForAdd) && (0L != pszPromptForAdd))
	{
		const char* szprompts[] = {pszPromptForAdd, pszPromptForRem};
		iRet = ads_ssget(":$", (const void*)szprompts, 0L, &Rb01, objSelSet);
	}
	else
	{
		const char* szprompts[] = {"Select points :", "Remove points"};
		iRet = ads_ssget(":$", (const void*)szprompts, 0L, &Rb01, objSelSet);
	}
	return (iRet == RTNORM);
}

//NB: This func. DOES NOT FREE the selset ...
BOOL ExtratObjectIDs(ads_name& SelSet, AcDbObjectIdArray& arrayResult)
{
	//Erase Entity(s)....
	long i, iNoOfEnt;
	ads_name Entity;
	
	{//Erase all Lines
		iNoOfEnt = 0;
		if(ads_sslength(SelSet, &iNoOfEnt) != RTNORM)
		{
			return FALSE;//ERROR
		}
		for(i = 0; i < iNoOfEnt; i++)
		{
			AcDbObjectId objId;

			if(ads_ssname(SelSet, i, Entity) != RTNORM)
			{
				ads_printf("\nERROR: AutoCAD System error -> ExtratObjectIDs() !\n");
				return FALSE;//ERROR
			}
			if(Acad::eOk != acdbGetObjectId(objId, Entity))
			{
				ads_printf("\nERROR: AutoCAD System error -> ExtratObjectIDs() !\n");
				return FALSE;
			}
			arrayResult.append(objId);
		}
	}
	ads_ssfree(SelSet);//Free Selset
	return TRUE;
}
BOOL EraseEntities(const AcDbObjectIdArray& ObjIdArray)
{
	int i;
	AcDbObject* pObj;

	for(i = 0; i < ObjIdArray.length(); i++)
	{
		AcDbObjectEx obj(ObjIdArray[i]);
		
		pObj = obj.getObject(AcDb::kForWrite);
		if(0L == pObj)
		{
			return FALSE;
		}
		pObj->erase();
	}
	return TRUE;
}
