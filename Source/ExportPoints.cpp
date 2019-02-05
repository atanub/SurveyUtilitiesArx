// File Name: ExportPoints.cpp
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"
#include "ResourceHelper.h"
#include "DrawUtil.h"
#include "Util.h"
#include "ExportPoints.h"
#include "QueryEntityProperty.h"

extern "C" HWND adsw_acadMainWnd();

//
// CExportPointsFactory
//////////////////////////////////////////////////////////////////////////////////////////////
//
bool CExportPointsFactory::Export(ExportType eType)
{
	CString strFile;
	CExportPointsRegular* pObjExportPoints;
	bool bResult;

	pObjExportPoints = 0L;

	{//Select Output File
		const CString strConstFileType = CString("Text Files (*.TXT)|*.TXT|All Files (*.*)|*.*||");
		DWORD dwFlags;

		dwFlags = OFN_HIDEREADONLY|OFN_NOREADONLYRETURN|OFN_EXPLORER|OFN_LONGNAMES|OFN_PATHMUSTEXIST;
		CFileDialog Dlg(false, "TXT", 0L, dwFlags, strConstFileType);
		if(Dlg.DoModal() != IDOK)
			return false;

		strFile = Dlg.GetPathName();
	}

	switch(eType)
	{
	case General:
		pObjExportPoints = new CExportPointsRegular(strFile);
		break;
	case MXRoads:
		//To Modify !!!
		pObjExportPoints = new CExportPointsMXRoads(strFile);
		break;
	default:
		return false;
	}

	bResult = pObjExportPoints->Export();
	delete pObjExportPoints;
	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// CExportPointsRegular
CExportPointsRegular::CExportPointsRegular(const char* pszFile) : m_pszFile(pszFile), m_pArrayObjIDs(0L)
{
	m_pSelector = 0L;
}
CExportPointsRegular::~CExportPointsRegular()
{
	if(0L == m_pSelector)
		delete m_pSelector;

	{//
		POSITION pos = m_mapLayerNVertices.GetStartPosition();
		while(pos != 0L)
		{
			CObject* pData;
			CTypedPtrArray<CPtrArray, CExportPointData*>* pDataOrg;
			CString string;

			m_mapLayerNVertices.GetNextAssoc(pos, string, pData);
			pDataOrg = (CTypedPtrArray<CPtrArray, CExportPointData*>*)pData;
			delete pDataOrg;
		}
		m_mapLayerNVertices.RemoveAll();
	}
}
bool CExportPointsRegular::Export()
{
	if(!selectEntities())
		return false;
	
	if(!processData())
		return false;
	
	if(!writeToFile())
		return false;
	
	return false;
}

bool CExportPointsRegular::selectEntities()
{
	if(m_pSelector != 0L)
		return false; //Control should never reach here !!

	m_pSelector = getEntitySelector();
	m_pArrayObjIDs = m_pSelector->Select();
	if((m_pArrayObjIDs == 0L) || (m_pArrayObjIDs->length() <= 0))
	{
		ads_printf("\nFailed to export points. No entities selected");
		delete m_pSelector;
		m_pSelector = 0L;
		m_pArrayObjIDs = 0L;
		return false;
	}
	return true;
}
bool CExportPointsRegular::processData()
{
	long i;
	Acad::ErrorStatus errCode;

	if(m_pArrayObjIDs == 0L)
		return false;

	for(i = 0; i < m_pArrayObjIDs->length(); i++)
	{
		AcDbObject* pObj;
		AcGePoint3dArray ptArray;
		CString strLayer;

		ads_printf("\rPlease wait...Extracting data from enity #%d\n", i+1);
		errCode = acdbOpenObject(pObj, m_pArrayObjIDs->at(i), AcDb::kForRead);
		if(errCode != Acad::eOk)
		{
			ads_printf("\nERROR: AutoCAD System error !\n");
			return false;
		}
		if(!processEntity(pObj, ptArray, strLayer))
		{
			pObj->close();
			return false;
		}
		pObj->close();
		//Store entities with single vertex...To be written after writing all other entities.
		if(ptArray.length() == 1)
		{
			addToMap(m_mapLayerNVerticesForSinglePtEntities, ptArray[0], strLayer);
			continue;
		}
		//Check & add to map If needed
		addToMap(m_mapLayerNVertices, ptArray, strLayer);
	}
	synchronizeMaps();
	return (m_mapLayerNVertices.GetCount() > 0);
}
bool CExportPointsRegular::processEntity(AcDbObject* pObj, AcGePoint3dArray& ptArrayParam, CString& strLayer)
{
	char* pszLyrThisEnt;

	pszLyrThisEnt = 0L;
	strLayer = "";
	if(ptArrayParam.length() > 0)
		return false; //Control should never reach here !!
	
	if(pObj->isA() == AcDbPoint::desc())
	{
		pszLyrThisEnt = AcDbPoint::cast(pObj)->layer();
		strLayer = pszLyrThisEnt;
		delete []pszLyrThisEnt;
		ptArrayParam.append(AcDbPoint::cast(pObj)->position());
		return true;
	}
	if(pObj->isA() == AcDbLine::desc())
	{
		pszLyrThisEnt = AcDbLine::cast(pObj)->layer();
		strLayer = pszLyrThisEnt;
		delete []pszLyrThisEnt;
		ptArrayParam.append(AcDbLine::cast(pObj)->startPoint());
		ptArrayParam.append(AcDbLine::cast(pObj)->endPoint());
		return true;
	}
	if(pObj->isA() == AcDbCircle::desc())
	{
		pszLyrThisEnt = AcDbCircle::cast(pObj)->layer();
		strLayer = pszLyrThisEnt;
		delete []pszLyrThisEnt;
		ptArrayParam.append(AcDbCircle::cast(pObj)->center());
		return true;
	}
	if(pObj->isA() == AcDbBlockReference::desc())
	{
		pszLyrThisEnt = AcDbBlockReference::cast(pObj)->layer();
		strLayer = pszLyrThisEnt;
		delete []pszLyrThisEnt;
		ptArrayParam.append(AcDbBlockReference::cast(pObj)->position());
		return true;
	}
	return false;
}
//Returns True if  Write is needed ...otherwise returns false
bool CExportPointsRegular::addToMap(CMapStringToOb& mapDestination, const AcGePoint3d& ptParam, const CString& strLayer)
{
	CObject* pData;
	CTypedPtrArray<CPtrArray, CExportPointData*>* pArray;

	if(mapDestination.Lookup(strLayer, pData))
	{
		pArray = (CTypedPtrArray<CPtrArray, CExportPointData*>*)(pData);
	}
	else
	{
		pArray = new CTypedPtrArray<CPtrArray, CExportPointData*>();
		mapDestination.SetAt(strLayer, pArray);
	}
	if(find(ptParam, pArray) < 0)
	{
		const CExportPointData* pLastRec = (pArray->GetSize() > 0) ? pArray->GetAt(pArray->GetSize() - 1) : 0L;
		const int iID = (pLastRec == 0L) ? 1 : (pLastRec->m_iVertexSequenceID + 2);
		pArray->Add(new CExportPointData(ptParam, iID));
		
		return true;//Added for Write
	}
	return false;//Ignored...Because the point already exists
}
bool CExportPointsRegular::addToMap(CMapStringToOb& mapDestination, const AcGePoint3dArray& pt3DArrayParam, const CString& strLayer)
{
	if(pt3DArrayParam.length() <= 0)
		return false; //Control should never reach here !!
	
	CObject* pData;
	CTypedPtrArray<CPtrArray, CExportPointData*>* pArray;
	int j;

	if(mapDestination.Lookup(strLayer, pData))
	{
		pArray = (CTypedPtrArray<CPtrArray, CExportPointData*>*)(pData);
	}
	else
	{
		pArray = new CTypedPtrArray<CPtrArray, CExportPointData*>();
		mapDestination.SetAt(strLayer, pArray);
	}
	//Check each vertex in 'pArray'....if all the vertices are present in 'pArray' 
	//in sequential order then it's a duplicate entity.... Ignore addition
	int iCountMatches, iFirstMatchedSeqID, iLastMatchedSeqID;

	iCountMatches = 0;
	for(j = 0, iCountMatches = 0, iFirstMatchedSeqID = iLastMatchedSeqID = -1; j < pt3DArrayParam.length(); j++)
	{
		int iIndex;
		
		iIndex = find(pt3DArrayParam[j], pArray);
		if(iIndex >= 0)
		{
			if(0 == iCountMatches)
			{
				iFirstMatchedSeqID = pArray->GetAt(iIndex)->m_iVertexSequenceID;
			}
			else
			{
				iLastMatchedSeqID = pArray->GetAt(iIndex)->m_iVertexSequenceID;
			}

			iCountMatches++;
		}

	}
	if(pt3DArrayParam.length() == iCountMatches)
	{
		if((iFirstMatchedSeqID >= 0) && (iLastMatchedSeqID >= 0) && (pt3DArrayParam.length() == (1 + iLastMatchedSeqID - iFirstMatchedSeqID)))
			return false;//Points found....Write NOT needed
	}
	{//Add to Map
		int iID;
	
		for(j = 0, iID = 0; j < pt3DArrayParam.length(); j++)
		{
			if(j == 0)
			{
				const CExportPointData* pLastRec = (pArray->GetSize() > 0) ? pArray->GetAt(pArray->GetSize() - 1) : 0L;
				iID = ((pLastRec == 0L) ? 1 : (pLastRec->m_iVertexSequenceID + 2));
			}
			pArray->Add(new CExportPointData(pt3DArrayParam[j], iID++));
		}
	}
	return true;//Point NOT found....Write needed
}
void CExportPointsRegular::synchronizeMaps()
{
	CString strLayerOfPointEnt;
	int j;

	POSITION pos = m_mapLayerNVerticesForSinglePtEntities.GetStartPosition();
	while(pos != 0L)
	{
		CObject* pData;
		CTypedPtrArray<CPtrArray, CExportPointData*>* pDataPts;

		m_mapLayerNVerticesForSinglePtEntities.GetNextAssoc(pos, strLayerOfPointEnt, pData);
		pDataPts = (CTypedPtrArray<CPtrArray, CExportPointData*>*)pData;
		
		{//Find in main array....If not found then add....
			CTypedPtrArray<CPtrArray, CExportPointData*>* pDataOrg;
			
			if(m_mapLayerNVertices.Lookup(strLayerOfPointEnt, pData))
			{
				pDataOrg = (CTypedPtrArray<CPtrArray, CExportPointData*>*)(pData);
				for(j = 0; j < pDataPts->GetSize(); j++)
				{
					CExportPointData* pRecForPoint = pDataPts->GetAt(j);
					
					if(find(pDataPts->GetAt(j)->m_Point, pDataOrg) < 0)
					{
						const CExportPointData* pLastRec = (pDataOrg->GetSize() > 0) ? pDataOrg->GetAt(pDataOrg->GetSize() - 1) : 0L;
						const int iID = ((pLastRec == 0L) ? 1 : (pLastRec->m_iVertexSequenceID + 2));

						pRecForPoint->m_iVertexSequenceID = iID;
						pDataOrg->Add(pRecForPoint);
						continue;
					}
					delete pRecForPoint;
				}
			}
			else
			{
				m_mapLayerNVertices.SetAt(strLayerOfPointEnt, pDataPts);
			}
		}
	}
	m_mapLayerNVerticesForSinglePtEntities.RemoveAll();
}

int CExportPointsRegular::find(const AcGePoint3d& pt3DParam, const CTypedPtrArray<CPtrArray, CExportPointData*>* pArray)
{
	int j;

	for(j = 0; j < pArray->GetSize(); j++)
	{
		const CExportPointData* pRec = pArray->GetAt(j);

		if(pRec->m_Point.isEqualTo(pt3DParam) == Adesk::kTrue)
			return j;//Point found....

	}
	return -1;//Point NOT found....
}

bool CExportPointsRegular::writeToFile() const
{
	CStdioFile FileToWrite;
	CString strLine;
	BOOL bFlag;
	int iCount, iPointCount;

	if(m_mapLayerNVertices.GetCount() <= 0)
	{
		ads_printf("\nERROR: No valid entity selected for export!\n");
		return false;
	}
	bFlag = FileToWrite.Open(m_pszFile, CFile::modeCreate|CFile::modeWrite|CFile::typeText);
	if(!bFlag)
	{
		ads_printf("\nERROR: Unable to open file %s", m_pszFile);
		return false;
	}
	
	writeHeader(&FileToWrite);
	{//
		POSITION pos;

		iCount = iPointCount = 0;
		pos = m_mapLayerNVertices.GetStartPosition();
		while(pos != 0L)
		{
			CObject* pData;
			CTypedPtrArray<CPtrArray, CExportPointData*>* pDataOrg;
			CString strLayer;

			m_mapLayerNVertices.GetNextAssoc(pos, strLayer, pData);
			pDataOrg = (CTypedPtrArray<CPtrArray, CExportPointData*>*)pData;
			writeData(&FileToWrite, pDataOrg, strLayer);
		
			iPointCount += pDataOrg->GetSize();
			iCount++;
			ads_printf("\rPlease wait...Writing data for Layer: %s [Layer #%d of %d]", strLayer, iCount, m_mapLayerNVertices.GetCount());
		}
	}
	ads_printf("\n\nFile \"%s\" written successfully\nTotal %d# of points exported", m_pszFile, iPointCount);


	return true;
}
bool CExportPointsRegular::writeHeader(CStdioFile* fileToWrite) const
{
	CString strLine;
	COleDateTime dateCurrent = COleDateTime::GetCurrentTime();
	
	strLine.Format("Generated on %s\n\n", dateCurrent.Format());
	fileToWrite->WriteString(strLine);

	strLine.Format("%-18s\t%-18s\t%-18s\t%-18s\n", "Northing", "Easting", "Elevation", "Layer");
	fileToWrite->WriteString(strLine);
	
	return true;
}
bool CExportPointsRegular::writeData(CStdioFile* fileToWrite, const CTypedPtrArray<CPtrArray, CExportPointData*>* pArray, const char* pszLyr) const
{
	CString strLine;

	for(int i = 0; i < pArray->GetSize(); i++)
	{
		const CExportPointData* pRec = pArray->GetAt(i);

		strLine.Format("%12.3f\t%12.3f\t%12.3f\t%-s\n", pRec->m_Point.y, pRec->m_Point.x, pRec->m_Point.z, pszLyr);
		fileToWrite->WriteString(strLine);
	}

	return true;
}
CEntitySelector* CExportPointsRegular::getEntitySelector() const
{
	return CEntitySelectorFactory::GetEntitySelector(CEntitySelectorFactory::SelectorType::ExportPoints);
}

//
//
// CExportPointsMXRoads
//////////////////////////////////////////////////////////////////////////////////////////////
//
CEntitySelector* CExportPointsMXRoads::getEntitySelector() const
{
	return CEntitySelectorFactory::GetEntitySelector(CEntitySelectorFactory::SelectorType::ExportPointsMXRoads);
}

bool CExportPointsMXRoads::writeHeader(CStdioFile* fileToWrite) const
{
	CString strLine;
	COleDateTime dateCurrent = COleDateTime::GetCurrentTime();
	
	strLine.Format("Input for MX Roads\r\nGenerated on %s\n\n", dateCurrent.Format());
	fileToWrite->WriteString(strLine);
	strLine.Format("%-18s\t%-18s\t%-18s\t%-18s\t%-18s\n", "Northing", "Easting", "Elevation", "Layer", "Sequence ID");
	fileToWrite->WriteString(strLine);
	
	return true;
}
bool CExportPointsMXRoads::writeData(CStdioFile* fileToWrite, const CTypedPtrArray<CPtrArray, CExportPointData*>* pArray, const char* pszLyr) const
{
	CString strLine;

	for(int i = 0; i < pArray->GetSize(); i++)
	{
		const CExportPointData* pRec = pArray->GetAt(i);

		strLine.Format("%12.3f\t%12.3f\t%12.3f\t%-s\t%d\n", pRec->m_Point.y, pRec->m_Point.x, pRec->m_Point.z, pszLyr, pRec->m_iVertexSequenceID);
		fileToWrite->WriteString(strLine);
	}

	return true;
}
bool CExportPointsMXRoads::processEntity(AcDbObject* pObj, AcGePoint3dArray& ptArrayParam, CString& strLayer)
{
	char* pszLyrThisEnt;

	if(CExportPointsRegular::processEntity(pObj, ptArrayParam, strLayer))
		return true;

	pszLyrThisEnt = 0L;
	strLayer = "";
	if(ptArrayParam.length() > 0)
		return false; //Control should never reach here !!

	
	if(pObj->isA() == AcDb3dPolyline::desc())
	{
		if(AcDb::Poly3dType::k3dSimplePoly != ((AcDb3dPolyline*)pObj)->polyType())
		{
			return false;
		}
		pszLyrThisEnt = AcDb3dPolyline::cast(pObj)->layer();
		strLayer = pszLyrThisEnt;
		delete []pszLyrThisEnt;

		return CQueryEntityProperty::Extract3dPolyVertices(((AcDb3dPolyline*)pObj), ptArrayParam);
	}
	return false;
}
