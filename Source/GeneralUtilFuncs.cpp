// File Name: GeneralUtilFuncs.cpp
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GeneralUtilFuncs.h"
#include "Resource.h"
#include "ResourceHelper.h"
#include "ResultDlg.h"
#include "JoinSelPtsDlg.h"
#include "QProfile.h"
#include "Line2PLine.h"
#include "DrawUtil.h"
#include "Util.h"
#include "UtilDoubleInpDlg.h"
#include "InsBlkFrFileDlg.h"
#include "EntitySelectors\EntitySelector.h"

#include "ParameterDataCache.h"

extern "C" HWND adsw_acadMainWnd();

/////////////////////////////////////////////////////////////////////////////
// CGeneralUtilFuncs
BOOL CGeneralUtilFuncs::InsBlocksFromDatFile()
{
	INSBLKFRFILEPARAMS recDwgParam;
	CStringArray strArrayErrs;
	BOOL bIsErr;

	recDwgParam = *(CSurvUtilApp::GetDataCache()->GetInsBlkFrFileParam());
	bIsErr = FALSE;
	{//
		CStringArray BlkNameArray;

		GetBlockNames(BlkNameArray);
		if(BlkNameArray.GetSize() <= 0)
		{
			AfxMessageBox("No block definition found in current drawing\nCreate block first & then try again", MB_ICONSTOP);
			return FALSE;
		}
		if(!GetInsBlkFrFileParams(BlkNameArray, recDwgParam))
			return FALSE;
	}
	CSurvUtilApp::GetDataCache()->SetInsBlkFrFileParam(&recDwgParam);
	{//
		CStdioFile	FileInp;
		CString strTmp;
		long iLineNo, iEntCount;
		const int iMinNoOfParams = (recDwgParam.bHasDataZValue) ? 3:2;

		bIsErr = FALSE;
		if(!FileInp.Open(recDwgParam.szDataFile, CFile::modeRead|CFile::typeText))
		{
			ads_printf("\nERROR: Unable to open input file %s", recDwgParam.szDataFile);
			return FALSE;
		}
		//Layer Setup
		if(strlen(recDwgParam.szLyrAnnotation) <= 0)
		{
			CLAYER(strTmp);//Get Cur Layer
			strcpy(recDwgParam.szLyrAnnotation, strTmp);//
		}
		else
		{
			LAYER(recDwgParam.szLyrAnnotation, 1);//Create New Layer
		}

		ads_printf("\nPlease wait... reading data file %s", recDwgParam.szDataFile);
		for(iLineNo = 0, iEntCount = 0;;)
		{
			CStringArray strWordArray;

			if(!ReadNextValidLine(FileInp, strWordArray, iLineNo))
			{//EOF reached...
				bIsErr = FALSE;
				goto FINAL;
			}
			if(strWordArray.GetSize() < iMinNoOfParams)
			{//ERROR
				strTmp.Format("ERROR: Invalid no of parameter found at line# %d, hence ignored", iLineNo);
				strArrayErrs.Add(strTmp);
			}
			else
			{//Draw...
				AcGePoint3d ptToDraw(0.0, 0.0, 0.0);

				ptToDraw.x = atof(strWordArray[1]);//Easting
				ptToDraw.y = atof(strWordArray[0]);//Northing
				if(recDwgParam.bHasDataZValue)
				{
					ptToDraw.z = atof(strWordArray[2]);//Elev
				}
				if(!recDwgParam.bInsConsideringZValue)
					ptToDraw.z = 0.0;

				BLOCK(recDwgParam.szBlkName, ptToDraw, recDwgParam.szLyrAnnotation, recDwgParam.dBlkRotAngle, recDwgParam.dBlkScaleX, recDwgParam.dBlkScaleY);
				
				if(strWordArray.GetSize() > iMinNoOfParams)
				{//Draw Text
					AcGePoint3d ptTmp;

					ads_polar(asDblArray(ptToDraw), recDwgParam.dBlkRotAngle, recDwgParam.dBlkScaleX, asDblArray(ptTmp));
					TEXTLEFT(ptTmp, strWordArray[iMinNoOfParams], recDwgParam.dTxtRotation, recDwgParam.dTxtSize, recDwgParam.szLyrAnnotation);
				}
				iEntCount++;
				ads_printf("\rTotal %d# of line(s) read & %d# of blocks inserted", iLineNo, iEntCount);
			}
			if(ads_usrbrk() == 1)
			{
				strTmp.Format("User break encountered\r\nTotal %d# of line(s) read & %d# of blocks inserted", iLineNo, iEntCount);
				strArrayErrs.Add(strTmp);

				ads_printf("User break encountered\nTotal %d# of line(s) read & %d# of blocks inserted", iLineNo, iEntCount);
				bIsErr = TRUE;
				goto FINAL;
			}
		}
		ads_printf("\n\nTotal %d# of line(s) read & %d# of blocks inserted", iLineNo, iEntCount);
	}
	FINAL :
	{
		if(strArrayErrs.GetSize() > 0)
		{//Show userfeedback
			::ShowResultDlg(strArrayErrs);
		}
	}
	return bIsErr;
}
BOOL CGeneralUtilFuncs::SetZValueOfSelEnts()
{
	ads_name SelSet;
	double fValNewZ;
	
	{//Get all the imported points....
		char* szprompts[] = {"Select entities to set Z value :", "Remove entities :"};

		if(ads_ssget(":$", szprompts, 0L, 0L, SelSet) != RTNORM)
		{
			return FALSE;
		}
	}
	const char* pszPmt = "Enter Z value for selected entities";
	fValNewZ = 0.0;
	if(!GetPositiveDoubleVal(pszPmt, CUtilDoubleInpDlg::ValidationType::Any, fValNewZ))
	{
		return FALSE;
	}
	

	{//
		ads_name Entity;
		long i, iNoOfEnt, iCountModified;

		if(ads_sslength(SelSet, &iNoOfEnt) != RTNORM)
		{
			ads_ssfree(SelSet);//Free Selset
			return FALSE;
		}
		for(i = 0, iCountModified = 0; i < iNoOfEnt; i++)
		{
			if(ads_ssname(SelSet, i, Entity) != RTNORM)
			{
				ads_printf("\nERROR: AutoCAD System error !\n");
				ads_ssfree(SelSet);//Free Selset
				return FALSE;
			}
			{//
				AcDbObject* pObj;
				AcDbObjectId objId;
				Acad::ErrorStatus ErrCode;

				ErrCode = acdbGetObjectId(objId, Entity);
				if(ErrCode != Acad::eOk)
				{
					ads_ssfree(SelSet);//Free Selset
					ads_printf("\nERROR: AutoCAD System error !\n");
					return FALSE;
				}
				ErrCode = acdbOpenObject(pObj, objId, AcDb::kForWrite);
				if(ErrCode == Acad::eOnLockedLayer)
				{
					ads_printf("\nERROR: Failed to update entity #%d, because the entity belongs to a locked layer!\n", i);
					continue;
				}
				if(ErrCode != Acad::eOk)
				{
					ads_ssfree(SelSet);//Free Selset
					ads_printf("\nERROR: AutoCAD System error !\n");
					return FALSE;
				}
				{//
					BOOL bFlag;
					AcGePoint3d ptTmp;

					bFlag = TRUE;
					if(pObj->isKindOf(AcDbLine::desc()) == Adesk::kTrue)
					{//Line
						ptTmp = AcDbLine::cast(pObj)->startPoint();
						ptTmp.z = fValNewZ;
						AcDbLine::cast(pObj)->setStartPoint(ptTmp); 

						ptTmp = AcDbLine::cast(pObj)->endPoint();
						ptTmp.z = fValNewZ;
						AcDbLine::cast(pObj)->setEndPoint(ptTmp); 
					}
					else if(pObj->isKindOf(AcDbText::desc()) == Adesk::kTrue)
					{//Text
						ptTmp = AcDbText::cast(pObj)->position();
						ptTmp.z = fValNewZ;
						AcDbText::cast(pObj)->setPosition(ptTmp); 
					}
					else if(pObj->isKindOf(AcDbPoint::desc()) == Adesk::kTrue)
					{//Point
						ptTmp = AcDbPoint::cast(pObj)->position();
						ptTmp.z = fValNewZ;
						AcDbPoint::cast(pObj)->setPosition(ptTmp); 
					}
					else if(pObj->isKindOf(AcDbCircle::desc()) == Adesk::kTrue)
					{//Circle
						ptTmp = AcDbCircle::cast(pObj)->center();
						ptTmp.z = fValNewZ;
						AcDbCircle::cast(pObj)->setCenter(ptTmp); 
					}
					else if(pObj->isKindOf(AcDbArc::desc()) == Adesk::kTrue)
					{//Arc
						ptTmp = AcDbArc::cast(pObj)->center();
						ptTmp.z = fValNewZ;
						AcDbArc::cast(pObj)->setCenter(ptTmp); 
					}
					else if(pObj->isKindOf(AcDbBlockReference::desc()) == Adesk::kTrue)
					{//Block
						ptTmp = AcDbBlockReference::cast(pObj)->position();
						ptTmp.z = fValNewZ;
						AcDbBlockReference::cast(pObj)->setPosition(ptTmp); 
						//
						//NOTE: Special case for Block "POINTBLK" not considered
						//Ref: SurvUtil.lsp R:590C:33
						//(defun ModifyAttrDef(entName fZValueNew / entMain entName entDet ptIns ptInsNew)
						//
					}
					else if(pObj->isKindOf(AcDb2dPolyline::desc()) == Adesk::kTrue)
					{//2dPolyline
						AcDb2dPolyline::cast(pObj)->setElevation(fValNewZ); 
					}
					else if(pObj->isKindOf(AcDb3dPolyline::desc()) == Adesk::kTrue)
					{//3dPolyline
						AcDbObjectIterator* pIterator;
						
						pIterator = AcDb3dPolyline::cast(pObj)->vertexIterator();
						for(pIterator->start(); (pIterator->done() == Adesk::kFalse); pIterator->step())
						{
							AcDbObjectId objectIdTmp;
							AcDb3dPolylineVertex* pVert;
							
							objectIdTmp = pIterator->objectId();

							ErrCode = ((AcDb3dPolyline*)pObj)->openVertex(pVert, objectIdTmp, AcDb::kForWrite);
							if(ErrCode != Acad::eOk)
							{
								ads_printf("\n\nERROR: Error occured while iterating 3DPolyline vertices!\n");
								AfxMessageBox("Error occured while iterating 3DPolyline vertices!", MB_ICONINFORMATION);
							}
							else
							{
								ptTmp = pVert->position();
								ptTmp.z = fValNewZ;
								pVert->setPosition(ptTmp); 
							}
							pVert->close();
						}
						delete pIterator;
					}
					else
					{
						bFlag = FALSE;
					}
						
					if(bFlag)
					{
						iCountModified++;
						ads_printf("\r%d of %d # of entities modified successfully", iCountModified, (i+1));
					}
				}
				pObj->close();
				if(ads_usrbrk() == 1)
				{
					ads_printf("\n\nUser break encountered\nTotal %d of %d # of entities modified successfully\n", iCountModified, (i+1));
					ads_ssfree(SelSet);//Free Selset
					return FALSE;
				}
			}
		}
		ads_printf("\n%d of %d # of entities modified successfully\n", iCountModified, iNoOfEnt);
	}
	ads_ssfree(SelSet);//Free Selset
	return TRUE;
}
BOOL CGeneralUtilFuncs::ModifyTxtSize()
{
	ads_name SelSet;
	double fNexTxtSize;
	
	{//Get all the imported points....
		struct  resbuf  Rb01;
		const char szEntName[] = "TEXT";
		char* szprompts[] = {"Select text entities to set text size :", "Remove entities :"};

		// Entity type
		Rb01.restype = 0;
		Rb01.resval.rstring = (char*)szEntName;
		Rb01.rbnext = 0L;
		
		if(ads_ssget(":$", szprompts, 0L, &Rb01, SelSet) != RTNORM)
		{
			return FALSE;
		}
	}
	const char* pszPmt = "Enter new text size for selected entities";
	fNexTxtSize = 0.0;
	const CUtilDoubleInpDlg::ValidationType iValidationType = (CUtilDoubleInpDlg::ValidationType)(CUtilDoubleInpDlg::ValidationType::ZeroNotAllowed|CUtilDoubleInpDlg::ValidationType::NegNotAllowed);

	if(!GetPositiveDoubleVal(pszPmt, iValidationType, fNexTxtSize))
	{
		return FALSE;
	}
	
	{//
		ads_name Entity;
		long i, iNoOfEnt;

		if(ads_sslength(SelSet, &iNoOfEnt) != RTNORM)
		{
			ads_ssfree(SelSet);//Free Selset
			return FALSE;
		}
		for(i = 0; i < iNoOfEnt; i++)
		{
			if(ads_ssname(SelSet, i, Entity) != RTNORM)
			{
				ads_printf("\nERROR: AutoCAD System error !\n");
				ads_ssfree(SelSet);//Free Selset
				return FALSE;
			}
			{//
				AcDbObject* pObj;
				AcDbObjectId objId;
				Acad::ErrorStatus ErrCode;

				ErrCode = acdbGetObjectId(objId, Entity);
				if(ErrCode != Acad::eOk)
				{
					ads_ssfree(SelSet);//Free Selset
					ads_printf("\nERROR: AutoCAD System error !\n");
					return FALSE;
				}
				ErrCode = acdbOpenObject(pObj, objId, AcDb::kForWrite);
				if(ErrCode == Acad::eOnLockedLayer)
				{
					ads_printf("\nERROR: Failed to update entity #%d, because the entity belongs to a locked layer!\n", i);
					continue;
				}
				if(ErrCode != Acad::eOk)
				{
					ads_ssfree(SelSet);//Free Selset
					ads_printf("\nERROR: AutoCAD System error !\n");
					return FALSE;
				}
				{//
					AcDbText::cast(pObj)->setHeight(fNexTxtSize);
				}
				ads_printf("\r%d# of text entities modified successfully", (i+1));
				pObj->close();
				if(ads_usrbrk() == 1)
				{
					ads_printf("\n\nUser break encountered\nTotal %d# of text entities modified successfully\n", (i+1));
					ads_ssfree(SelSet);//Free Selset
					return FALSE;
				}
			}
		}
		ads_printf("\n%d# of text entities modified successfully\n", iNoOfEnt);
	}
	ads_ssfree(SelSet);//Free Selset
	return TRUE;
}
BOOL CGeneralUtilFuncs::ModifyZVal()
{
	ads_name SelSet;
	double fValIncr;
	
	{//Get all the imported points....
		char* szprompts[] = {"Select entities to modify (increment/decrement Z value) :", "Remove entities :"};

		if(ads_ssget(":$", szprompts, 0L, 0L, SelSet) != RTNORM)
		{
			return FALSE;
		}
	}
	const char* pszPmt = "Enter Z increment\r\nProvide -ve value for decrement";
	fValIncr = 0.0;
	const CUtilDoubleInpDlg::ValidationType iValidationType = (CUtilDoubleInpDlg::ValidationType)(CUtilDoubleInpDlg::ValidationType::ZeroNotAllowed);
	if(!GetPositiveDoubleVal(pszPmt, iValidationType, fValIncr))
	{
		return FALSE;
	}
	
	{//
		ads_name Entity;
		long i, iNoOfEnt, iNoOfModifiedEnts;

		if(ads_sslength(SelSet, &iNoOfEnt) != RTNORM)
		{
			ads_ssfree(SelSet);//Free Selset
			return FALSE;
		}
		for(i = 0, iNoOfModifiedEnts = 0; i < iNoOfEnt; i++)
		{
			if(ads_ssname(SelSet, i, Entity) != RTNORM)
			{
				ads_printf("\nERROR: AutoCAD System error !\n");
				ads_ssfree(SelSet);//Free Selset
				return FALSE;
			}
			{//
				AcDbObject* pObj;
				AcDbObjectId objId;
				Acad::ErrorStatus ErrCode;
				BOOL bCanTransform;

				bCanTransform = FALSE;
				ErrCode = acdbGetObjectId(objId, Entity);
				if(ErrCode != Acad::eOk)
				{
					ads_ssfree(SelSet);//Free Selset
					ads_printf("\nERROR: AutoCAD System error !\n");
					return FALSE;
				}
				ErrCode = acdbOpenObject(pObj, objId, AcDb::kForWrite);
				if(ErrCode == Acad::eOnLockedLayer)
				{
					ads_printf("\nERROR: Failed to update entity #%d, because the entity belongs to a locked layer!\n", i);
					continue;
				}
				if(ErrCode != Acad::eOk)
				{
					ads_ssfree(SelSet);//Free Selset
					ads_printf("\nERROR: AutoCAD System error !\n");
					pObj->close();
					return FALSE;
				}
				{//Decide whether the object can be modified or not...
					if(pObj->isA() == AcDbPoint::desc())
					{
						bCanTransform = (((AcDbPoint*)pObj)->position().z != 0.0);
					}
					else if(pObj->isA() == AcDbLine::desc())
					{
						bCanTransform = ((((AcDbLine*)pObj)->startPoint().z != 0.0) || (((AcDbLine*)pObj)->endPoint().z != 0.0));
					}
					else if(pObj->isA() == AcDb2dPolyline::desc())
					{
						bCanTransform = (((AcDb2dPolyline*)pObj)->elevation() != 0.0);
					}
					else if(pObj->isA() == AcDbPolyline::desc())
					{
						bCanTransform = (((AcDbPolyline*)pObj)->elevation() != 0.0);
					}
					else if(pObj->isA() == AcDbCircle::desc())
					{
						bCanTransform = (((AcDbCircle*)pObj)->center().z != 0.0);
					}
					else if(pObj->isA() == AcDbArc::desc())
					{
						bCanTransform = (((AcDbArc*)pObj)->center().z != 0.0);
					}
					else if(pObj->isA() == AcDbEllipse::desc())
					{
						bCanTransform = (((AcDbEllipse*)pObj)->center().z != 0.0);
					}
					else if(pObj->isA() == AcDbBlockReference::desc())
					{
						bCanTransform = (((AcDbBlockReference*)pObj)->position().z != 0.0);
					}
					else if(pObj->isA() == AcDbMText::desc())
					{
						bCanTransform = (((AcDbMText*)pObj)->location().z != 0.0);
					}
					else if(pObj->isA() == AcDbText::desc())
					{
						bCanTransform = (((AcDbText*)pObj)->position().z != 0.0);
					}
					else if(pObj->isA() == AcDb3dPolyline::desc())
					{
						AcDbObjectIterator* pIterator;
						
						bCanTransform = FALSE;
						pIterator = AcDb3dPolyline::cast(pObj)->vertexIterator();
						for(pIterator->start(); (pIterator->done() == Adesk::kFalse); pIterator->step())
						{
							AcDbObjectId objectIdTmp;
							AcDb3dPolylineVertex* pVert;
							
							objectIdTmp = pIterator->objectId();

							ErrCode = ((AcDb3dPolyline*)pObj)->openVertex(pVert, objectIdTmp, AcDb::kForRead);
							if(ErrCode != Acad::eOk)
							{
								ads_printf("\n\nERROR: Error occured while iterating 3DPolyline vertices!\n");
								//AfxMessageBox("Error occured while iterating 3DPolyline vertices!", MB_ICONINFORMATION);
							}
							else
							{
								bCanTransform = (pVert->position().z != 0.0);
							}
							pVert->close();
							//
							if(bCanTransform)
								break;
						}
					}
					else
					{
						ads_printf("\n\nERROR: Failed to modify entity:\"%s\"", pObj->isA()->name());
					}
				}
				if(bCanTransform)
				{//Transform
					const AcGeVector3d vec3D(0, 0, fValIncr);

					AcDbEntity::cast(pObj)->transformBy(AcGeMatrix3d::translation(vec3D));
					//
					//NOTE: Special case for Block "POINTBLK" not considered
					//Ref: SurvUtil.lsp R:427C:29
					//(defun ModifyAttrDef(entName fZIncriment / entMain entName entDet ptIns fZValue ptInsNew)
					//
					iNoOfModifiedEnts++;
					ads_printf("\r%d# of entities modified successfully", iNoOfModifiedEnts);
				}
				pObj->close();
				if(ads_usrbrk() == 1)
				{
					ads_printf("\n\nUser break encountered\nTotal %d# of entities modified successfully\n", (i+1));
					ads_ssfree(SelSet);//Free Selset
					return FALSE;
				}
			}
		}
		//
		ads_printf("\n%d of %d # of entities modified successfully\n", iNoOfModifiedEnts, iNoOfEnt);
	}
	ads_ssfree(SelSet);//Free Selset
	return TRUE;
}
BOOL CGeneralUtilFuncs::InsBlocks(const CString& strLyr, const CString& strBlk, const double& dX, const double& dY, const double& dZ, const double& dRotAngle, const BOOL& bConsiderZ)
{
	ads_name SelSet;
	
	{//Get all the imported points....
		struct  resbuf  Rb01;
		const char szEntName[] = "POINT";
		char* szprompts[] = {"Select points (Survey Utility Generated) for Block Insertion :", "Remove Points :"};
		// Entity type
		Rb01.restype = 0;
		Rb01.resval.rstring = (char*)szEntName;
		Rb01.rbnext = 0L;
		
		if(ads_ssget(":$", szprompts, 0L, &Rb01, SelSet) != RTNORM)
		{
			return FALSE;
		}
	}
	{//
		ads_name Entity;
		long i, iNoOfEnt;

		if(ads_sslength(SelSet, &iNoOfEnt) != RTNORM)
		{
			ads_ssfree(SelSet);//Free Selset
			return FALSE;
		}
		for(i = 0; i < iNoOfEnt; i++)
		{
			if(ads_ssname(SelSet, i, Entity) != RTNORM)
			{
				ads_printf("\nERROR: AutoCAD System error !\n");
				ads_ssfree(SelSet);//Free Selset
				return FALSE;
			}
			{//
				AcDbObject* pObj;
				AcDbObjectId objId;
				Acad::ErrorStatus ErrCode;
				AcGePoint3d ptTmp;

				ErrCode = acdbGetObjectId(objId, Entity);
				if(ErrCode != Acad::eOk)
				{
					ads_ssfree(SelSet);//Free Selset
					ads_printf("\nERROR: AutoCAD System error !\n");
					return FALSE;
				}
				ErrCode = acdbOpenObject(pObj, objId, AcDb::kForRead);
				if(ErrCode != Acad::eOk)
				{
					ads_ssfree(SelSet);//Free Selset
					ads_printf("\nERROR: AutoCAD System error !\n");
					return FALSE;
				}
				if(pObj->isA() != AcDbPoint::desc())
				{
					ads_ssfree(SelSet);//Free Selset
					ads_printf("\nERROR: Entity selected is not a point\n");
					pObj->close();
					return FALSE;
				}
				ptTmp = AcDbPoint::cast(pObj)->position();
				if(!bConsiderZ) ptTmp.z = 0.0;
				if(strlen(strLyr) <= 0)
				{
					CString strCurLyr;
					
					CLAYER(strCurLyr);
					BLOCK(strBlk, ptTmp, strCurLyr, DTOR(dRotAngle), dX, dY, dZ);
				}
				else
				{
					BLOCK(strBlk, ptTmp, strLyr, DTOR(dRotAngle), dX, dY, dZ);
				}
				pObj->close();
				if(ads_usrbrk() == 1)
				{
					ads_printf("\n\nUser break encountered\nTotal %d# of blocks inserted successfully\n", (i+1));
					ads_ssfree(SelSet);//Free Selset
					return FALSE;
				}
				ads_printf("\rTotal %d# of blocks inserted successfully\n", (i+1));
			}
		}
		ads_printf("\nTotal %d# of blocks inserted successfully\n", iNoOfEnt);
	}
	ads_ssfree(SelSet);//Free Selset
	return TRUE;
}
BOOL CGeneralUtilFuncs::JoinPoints(const CString& strLyr, const BOOL& bLineOrPLine /* = TRUE*/, const BOOL& bConsiderZ/* = TRUE*/)
{
	AcGePoint3d PtBase;
	AcGePoint3dArray PtArray;
	
	{//Get all the point coords....
		ads_name SelSet;
		ads_name Entity;
		long i, iNoOfEnt;
		struct  resbuf  Rb01;
		const char szEntName[] = "POINT";
		char* szprompts[] = {"Select points (Survey Utility Generated) to join :", "Remove Points :"};
		// Entity type
		Rb01.restype = 0;
		Rb01.resval.rstring = (char*)szEntName;
		Rb01.rbnext = 0L;
		
		if(ads_ssget(":$", szprompts, 0L, &Rb01, SelSet) != RTNORM)
		{
			ads_printf("\nERROR: No points selected\n");
			return FALSE;
		}
		if(ads_sslength(SelSet, &iNoOfEnt) != RTNORM)
		{
			ads_ssfree(SelSet);//Free Selset
			return FALSE;
		}
		for(i = 0; i < iNoOfEnt; i++)
		{
			if(ads_ssname(SelSet, i, Entity) != RTNORM)
			{
				ads_printf("\nERROR: AutoCAD System error !\n");
				ads_ssfree(SelSet);//Free Selset
				return FALSE;
			}
			{//
				AcDbObject* pObj;
				AcDbObjectId objId;
				Acad::ErrorStatus ErrCode;
				AcGePoint3d ptTmp;

				ErrCode = acdbGetObjectId(objId, Entity);
				if(ErrCode != Acad::eOk)
				{
					ads_ssfree(SelSet);//Free Selset
					ads_printf("\nERROR: AutoCAD System error !\n");
					return FALSE;
				}
				ErrCode = acdbOpenObject(pObj, objId, AcDb::kForRead);
				if(ErrCode != Acad::eOk)
				{
					ads_ssfree(SelSet);//Free Selset
					ads_printf("\nERROR: AutoCAD System error !\n");
					return FALSE;
				}
				if(pObj->isA() != AcDbPoint::desc())
				{
					ads_ssfree(SelSet);//Free Selset
					ads_printf("\nERROR: Entity selected is not a point\n");
					pObj->close();
					return FALSE;
				}
				pObj->close();
				PtArray.append(AcDbPoint::cast(pObj)->position());
			}
		}
		ads_ssfree(SelSet);//Free Selset
	}
	{//Find the lower left corner (to be treated as Base point)
		int i;
		PtBase = PtArray[0];
		for(i = 1; i < PtArray.length(); i++)
		{
			if((PtArray[i]).x < PtBase.x)
			{
				PtBase = PtArray[i];
			}
			else
			{
				if((PtArray[i]).x == PtBase.x)
				{
					if((PtArray[i]).y < PtBase.y)
						PtBase = PtArray[i];
				}
			}
		}
	}
	if(!SortPointsOnLine(PtBase, PtArray))
		return FALSE;
	if(bLineOrPLine)
	{
		LINES(PtArray, strLyr);
	}
	else
	{
		POLYLINE(PtArray, strLyr, bConsiderZ);
	}
	ads_printf("\n%d# of points joined successfully\n", PtArray.length());
	return TRUE;
}

BOOL CGeneralUtilFuncs::QProfilePts(const RDPFLNEZDATA& Rec)
{
	AcGePoint3d PtBase;
	AcGePoint3dArray PtArray;
	
	{//Get all the point coords....
		const char* szprompts[] = {"Select points (Survey Utility Generated) to draw profile :", "Remove Points :"};
		ads_name SelSet;

		if(!SelectPoints(SelSet, szprompts[0], szprompts[1]))
		{
			ads_printf("\nERROR: No points selected\n");
			return FALSE;
		}
		if(!ExtractPtsFromSelSet(SelSet, PtArray))
		{
			ads_ssfree(SelSet);//Free Selset
			return FALSE;
		}
		ads_ssfree(SelSet);//Free Selset
	}
	if(PtArray.length() > 0)
	{//Find the lower left corner (to be treated as Base point)
		int i;
		PtBase = PtArray[0];
		for(i = 1; i < PtArray.length(); i++)
		{
			if((PtArray[i]).x < PtBase.x)
			{
				PtBase = PtArray[i];
			}
			else
			{
				if((PtArray[i]).x == PtBase.x)
				{
					if((PtArray[i]).y < PtBase.y)
						PtBase = PtArray[i];
				}
			}
		}
	}

	if(!SortPointsOnLine(PtBase, PtArray))
		return FALSE;
	
	//Proccess points...
	for(; IsCurSortDirOK(PtArray) == 0;)
	{
		PtArray.reverse();
	}

	///
	if(ads_usrbrk() == 1)
	{
		ads_printf("\nUser break!\n");
		return FALSE;
	}
	DrawQProfile(PtArray, Rec);
	ads_printf("\nProfile drawn successfully using %d# of points\n", PtArray.length());
	return TRUE;
}
BOOL CGeneralUtilFuncs::QProfileLines(const RDPFLNEZDATA& Rec)
{
	AcGePoint3dArray PtArray, arrayForOrdAnn;
	CStringArray arrayLyrNames;
	
	if(!GetPtsFrLinesForQPfl(PtArray))
		return FALSE;
	
	{//Get all the point coords....for Ordinate Annotation
		const char* szprompts[] = {"Select points for Ordinate Annotation :", "Remove Points :"};
		ads_name SelSet;

		if(SelectPoints(SelSet, szprompts[0], szprompts[1]))
		{
			if(!ExtractPtsNLyrsFromSelSet(SelSet, arrayForOrdAnn, arrayLyrNames))
			{
				ads_ssfree(SelSet);//Free Selset
				return FALSE;
			}
		}
		ads_ssfree(SelSet);//Free Selset
	}

	//Proccess points...
	for(; IsCurSortDirOK(PtArray) == 0;)
	{
		PtArray.reverse();
	}
	///
	if(ads_usrbrk() == 1)
	{
		ads_printf("\nUser break!\n");
		return FALSE;
	}
	DrawQProfile(PtArray, Rec, &arrayForOrdAnn, &arrayLyrNames);
	ads_printf("\nProfile drawn successfully using %d# of lines\n", PtArray.length());
	return TRUE;
}
int CGeneralUtilFuncs::IsCurSortDirOK(const AcGePoint3dArray& Array)
{
	ShowCurDirection(Array);
	{//Ask Confirmation 
		char szReply[255];
		ads_initget(1, "Yes No");
		ads_getkword("\nThe highlighted line indicates the start points of the selected segment\nIs it acceptable <Y/N> ?", szReply);
		if((szReply[0] == 'y') || (szReply[0] == 'Y'))
		{
			ads_redraw(0L, 1);
			return 1;
		}
	}
	ads_redraw(0L, 1);
	return 0;
}
int CGeneralUtilFuncs::ShowCurDirection(const AcGePoint3dArray& Array)
{
	CString strMsg;
	int i;

	if(Array.length() <= 1)
	{
		ads_printf("\nFATAL ERROR: Not sufficient point(s) present\n");
		return 0;
	}

	strMsg.Format("Start line [(%.3f, %.3f, %.3f) to (%.3f, %.3f, %.3f)], for current sequence is flashing...\nPress Escape to continue...", Array[0].x, Array[0].y, Array[0].z, Array[1].x, Array[1].y, Array[1].z);
	AfxMessageBox(strMsg, MB_ICONINFORMATION);
	for(i = 1; i < Array.length();)
	{
		char szDummy[255];
		DrawArrowHeadOnDisp(Array[i - 1], Array[i]);
		if(RTNORM != ads_getstring(0, "\nPress 'Enter' key or 'Space Bar' to view next segment", szDummy))
		{
			return 1;
		}
		ads_redraw(0L, 1);
		if(ads_usrbrk() == 1)
		{
			return 1;
		}
		if(i == (Array.length() - 1))
			i = 1;
		else
			i++;
	}
	return 1;
}
void CGeneralUtilFuncs::DrawArrowHeadOnDisp(const AcGePoint3d& ptStart, const AcGePoint3d& ptEnd, const BOOL bImmediateErase /*=FALSE*/)
{
	double fLen, fAngle;
	AcGePoint3d ptMid, ptLeft, ptRight;
	
	fLen = Dist2D(ptStart, ptEnd);
	fAngle = Angle2D(ptStart, ptEnd);
	ads_polar(asDblArray(ptStart), fAngle, fLen / 2.0, asDblArray(ptMid));
	ads_polar(asDblArray(ptMid), (fAngle + PI / 2.0), fLen / 2.0, asDblArray(ptLeft));
	ads_polar(asDblArray(ptMid), (fAngle - PI / 2.0), fLen / 2.0, asDblArray(ptRight));
	 
	ads_grdraw (asDblArray(ptRight), asDblArray(ptLeft), -1, 1);
	if(bImmediateErase)
		ads_grdraw (asDblArray(ptRight), asDblArray(ptLeft), -1, 1);

	ads_grdraw (asDblArray(ptRight), asDblArray(ptEnd), -1, 1);
	if(bImmediateErase)
		ads_grdraw (asDblArray(ptRight), asDblArray(ptEnd), -1, 1);
	
	ads_grdraw (asDblArray(ptLeft), asDblArray(ptEnd), -1, 1);
	if(bImmediateErase)
		ads_grdraw (asDblArray(ptLeft), asDblArray(ptEnd), -1, 1);
	
	ads_grdraw (asDblArray(ptStart), asDblArray(ptEnd), -1, 1);
	if(bImmediateErase)
		ads_grdraw (asDblArray(ptStart), asDblArray(ptEnd), -1, 1);
}
BOOL CGeneralUtilFuncs::ExtractPtsNLyrsFromSelSet(ads_name& objSelSet, AcGePoint3dArray& arrayParam, CStringArray& arrayLyrsParam)
{
	long i, iNoOfEnt;

	if((arrayParam.length() > 0) || (arrayLyrsParam.GetSize() > 0))
	{
		ads_printf("\nERROR: Invalid parameter passed TRACE: From >> CGeneralUtilFuncs::ExtractPtsNLyrsFromSelSet()!\n");
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
			AcDbObject* pObj;
			AcDbObjectId objId;
			Acad::ErrorStatus ErrCode;
			AcGePoint3d ptTmp;

			ErrCode = acdbGetObjectId(objId, Entity);
			if(ErrCode != Acad::eOk)
			{
				ads_printf("\nERROR: AutoCAD System error !\n");
				return FALSE;
			}
			ErrCode = acdbOpenObject(pObj, objId, AcDb::kForRead);
			if(ErrCode != Acad::eOk)
			{
				ads_printf("\nERROR: AutoCAD System error !\n");
				return FALSE;
			}
			if(pObj->isA() != AcDbPoint::desc())
			{
				ads_printf("\nERROR: Entity selected is not a point\n");
				pObj->close();
				return FALSE;
			}
			arrayParam.append(AcDbPoint::cast(pObj)->position());
			{//Extract Layer
				char* pszLyrThisEnt;

				pszLyrThisEnt = AcDbPoint::cast(pObj)->layer();
				arrayLyrsParam.Add(pszLyrThisEnt);
				delete []pszLyrThisEnt;
			}
			pObj->close();
		}
	}
	return TRUE;
}
BOOL CGeneralUtilFuncs::QProfilePickNAnnotate(const RDPFLNEZDATA& Rec)
{
	AcGePoint3dArray arrayPickedPoints, arrayPoints;
	AcDbObjectId objFirstSelObjID;
	BOOL bFirstEntFound;
	BOOL bContinueLoopFlag;
	double fDatumLineY;

	if(!SelectDatumLine(fDatumLineY))
	{
		ads_printf("\nERROR: Invalid Datum line selected!");
		return FALSE;
	}
	//
	bFirstEntFound = FALSE;
	bContinueLoopFlag = TRUE;
	while(bContinueLoopFlag)
	{
		AcGePoint3d ptPicked;
		int iUserResponse;

		if(Select2DPLine(ptPicked, &objFirstSelObjID, (bFirstEntFound) ? 0L : &arrayPoints, iUserResponse))
		{
			arrayPickedPoints.append(ptPicked);
			ads_printf("\nTotal %d# of points picked for annotation", arrayPickedPoints.length());
		}
		bFirstEntFound = TRUE;
		switch(iUserResponse)
		{
		case -1:
			{//User termination
				if(arrayPickedPoints.length() > 0)
				{
					if(IDYES == AfxMessageBox("Are you sure you want to cancel \"Quick Annotation\"", MB_ICONQUESTION|MB_YESNO))
						return FALSE;
				}
				else
				{
					return FALSE;
				}
			}
			break;
		case 0://Not a valid entity Selected
		case 1://Valid entity Selected
			//Nothing special to do!!
			break;
		case 2://No entity Selected at All..So Break out of Infinite loop
			bContinueLoopFlag = FALSE;
			break;
		default:
			bContinueLoopFlag = FALSE;
			break;
		}
	}
	//
	AnnotateQProfile(fDatumLineY, arrayPickedPoints, arrayPoints, Rec);
	return TRUE;
}
//NOTE:
//iUserResponse = -1 >> User termination
//iUserResponse = 0  >> Not a valid entity Selected
//iUserResponse = 1  >> Valid entity Selected
//iUserResponse = 2  >> No entity Selected at All
BOOL CGeneralUtilFuncs::Select2DPLine(AcGePoint3d& ptPicked, AcDbObjectId* pPrevSelObjID, AcGePoint3dArray* pVertices, int& iUserResponse)
{
	ads_name Entity;
	int iRetVal;

	iUserResponse = TRUE;
	iRetVal = ads_entsel(0L, Entity, asDblArray(ptPicked));
	if(RTNORM != iRetVal)
	{
		iUserResponse = (RTCAN == iRetVal) ? -1:2;
		return FALSE;
	}
	{//Examine selected Entity......
		AcDbObject* pObj;
		AcDbObjectId objId;
		Acad::ErrorStatus ErrCode;
		AcGePoint3d ptTmp;

		ErrCode = acdbGetObjectId(objId, Entity);
		if(ErrCode != Acad::eOk)
		{
			ads_printf("\nERROR: AutoCAD System error !");
			iUserResponse = -1;
			return FALSE;
		}
		ErrCode = acdbOpenObject(pObj, objId, AcDb::kForRead);
		if(ErrCode != Acad::eOk)
		{
			ads_printf("\nERROR: AutoCAD System error !");
			iUserResponse = -1;
			return FALSE;
		}
		if(pObj->isA() != AcDbPolyline::desc())
		{
			ads_printf("\nERROR: Selected Entity is not a 2d polyline\n");
			pObj->close();
			iUserResponse = 1;
			return FALSE;
		}
		if(!((AcDbPolyline*)pObj)->isOnlyLines())
		{
			ads_printf("\nERROR: Selected 2D Polyline contains curved segments !");
			pObj->close();
			iUserResponse = 1;
			return FALSE;
		}
		if(0L != pVertices)
		{//Extract vertices
			AcDbPolyline* pEnt;
			int i;

			pEnt = (AcDbPolyline*)pObj;
			//Extract points...
			for(i = 0; i < (int)pEnt->numVerts(); i++)
			{
				AcGePoint2d pt2D;

				ErrCode = pEnt->getPointAt(i, pt2D);
				if(ErrCode != Acad::eOk)
				{
					iUserResponse = -1;
					ads_printf("\nERROR: Error occured while iterating vertices!");
					pObj->close();
					return FALSE;
				}
				pVertices->append(AcGePoint3d(pt2D.x, pt2D.y, 0.0));
			}
			if(0L != pPrevSelObjID)
			{
				*pPrevSelObjID = objId;
			}
		}
		if(0L != pPrevSelObjID)
		{
			if(*pPrevSelObjID != objId)
			{
				iUserResponse = -1;
				ads_printf("\nERROR: Invalid entity picked!");
				pObj->close();
				return FALSE;
			}
		}
		pObj->close();
	}
	return TRUE;
}

BOOL CGeneralUtilFuncs::SelectDatumLine(double& fY)
{
	ads_name Entity;
	int iRetVal;
	AcGePoint3d ptTmp;

	fY = 0.0;
	ads_printf("\nSelect Datum Line :");
	iRetVal = ads_entsel(0L, Entity, asDblArray(ptTmp));
	if(RTNORM != iRetVal)
	{
		return FALSE;
	}
	{//Examine selected Entity......
		AcDbObject* pObj;
		AcDbObjectId objId;
		Acad::ErrorStatus ErrCode;

		ErrCode = acdbGetObjectId(objId, Entity);
		if(ErrCode != Acad::eOk)
		{
			ads_printf("\nERROR: AutoCAD System error !");
			return FALSE;
		}
		ErrCode = acdbOpenObject(pObj, objId, AcDb::kForRead);
		if(ErrCode != Acad::eOk)
		{
			ads_printf("\nERROR: AutoCAD System error !");
			return FALSE;
		}
		if(pObj->isA() != AcDbLine::desc())
		{
			ads_printf("\nERROR: Selected Entity is not a line\n");
			pObj->close();
			return FALSE;
		}
		fY = ((AcDbLine*)pObj)->startPoint().y;
		pObj->close();
	}
	return TRUE;
}
