#ifndef _EXPORT_POINTS_H
#define _EXPORT_POINTS_H

#include "EntitySelectors\EntitySelector.h"

//
//
// CExportPointsFactory
//////////////////////////////////////////////////////////////////////////////////////////////
//
class CExportPointsFactory
{
public:
	typedef enum ExportType
	{
		General	= 0,
		MXRoads	= 1
	};
// Construction
protected:
	CExportPointsFactory() {;}
public:
	static bool	Export(ExportType eType);
};

//
//
// CExportPointData
//////////////////////////////////////////////////////////////////////////////////////////////
//
class CExportPointData
{
public:
	CExportPointData()
	{
		m_iVertexSequenceID = 0;
	}
	CExportPointData(const AcGePoint3d& ptParam, int iSequenceID = -1)
	{
		this->m_Point = ptParam;
		this->m_iVertexSequenceID = iSequenceID;
	}
	CExportPointData(const CExportPointData& obj)
	{
		this->m_Point				= obj.m_Point;
		this->m_iVertexSequenceID	= obj.m_iVertexSequenceID;
	}

public:
	AcGePoint3d			m_Point;
	int					m_iVertexSequenceID;
};

//
//
// CExportPointsRegular
//////////////////////////////////////////////////////////////////////////////////////////////
//
class CExportPointsRegular
{
// Construction
public:
	CExportPointsRegular(const char* pszFile);
public:
	virtual ~CExportPointsRegular();
public:
	bool	Export();
protected:
	bool						selectEntities();
	virtual CEntitySelector*	getEntitySelector() const;
	bool			processData();
	virtual bool	processEntity(AcDbObject* pObj, AcGePoint3dArray& pt3DArrayParam, CString& strLayer);
	bool			writeToFile() const;
	virtual bool	writeHeader(CStdioFile* fileToWrite) const;
	virtual bool	writeData(CStdioFile* fileToWrite, const CTypedPtrArray<CPtrArray, CExportPointData*>* pArray, const char* pszLyr) const;

	void			synchronizeMaps();
protected:
	static	int		find(const AcGePoint3d& pt3DParam, const CTypedPtrArray<CPtrArray, CExportPointData*>* pArray);
	static	bool	addToMap(CMapStringToOb& mapDestination, const AcGePoint3d& ptParam, const CString& strLayer);
	static	bool	addToMap(CMapStringToOb& mapDestination, const AcGePoint3dArray& pt3DArrayParam, const CString& strLayer);
protected:
	CMapStringToOb									m_mapLayerNVertices;
	CMapStringToOb									m_mapLayerNVerticesForSinglePtEntities;

	//
	CEntitySelector*			m_pSelector;
	const AcDbObjectIdArray*	m_pArrayObjIDs;
	const char*					m_pszFile;
};

//
//
// CExportPointsMXRoads
//////////////////////////////////////////////////////////////////////////////////////////////
//
class CExportPointsMXRoads : public CExportPointsRegular
{
// Construction
public:
	CExportPointsMXRoads(const char* pszFile) : CExportPointsRegular(pszFile)
	{
	}
protected:
	virtual bool				processEntity(AcDbObject* pObj, AcGePoint3dArray& pt3DParam, CString& strLayer);
	virtual CEntitySelector*	getEntitySelector() const;
	virtual bool				writeHeader(CStdioFile* fileToWrite) const;
	virtual bool				writeData(CStdioFile* fileToWrite, const CTypedPtrArray<CPtrArray, CExportPointData*>* pArray, const char* pszLyr) const;
};

#endif //_EXPORT_POINTS_H
