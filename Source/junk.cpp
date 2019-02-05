#include "stdafx.h"

#define ENTITY_RELATION_SERVICE  "SUARX_ENTITY_RELATION_SERVICE"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CSurvUtilEntRelation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSurvUtilEntRelation: public AcDbObject 
{ 
public:
	ACRX_DECLARE_MEMBERS(CSurvUtilEntRelation);

	// CSurvUtilEntRelation protocol

	CSurvUtilEntRelation();
	virtual ~CSurvUtilEntRelation();

	const AcDbObjectId& GetLinkID() const { return m_iDbObjID;}
	void				SetLinkID(const AcDbObjectId& iObjID) { m_iDbObjID = iObjID; }

	// Overridden methods from AcDbObject
	virtual Acad::ErrorStatus   dwgInFields(AcDbDwgFiler* filer);
	virtual Acad::ErrorStatus   dwgOutFields(AcDbDwgFiler* filer) const;

protected:
	AcDbObjectId m_iDbObjID;
};
MAKE_ACDBOPENOBJECT_FUNCTION(CSurvUtilEntRelation);
/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CRDPflNEZDataHolder
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRDPflNEZDataHolder : public AcDbObject 
{ 
public:
	ACRX_DECLARE_MEMBERS(CRDPflNEZDataHolder);

	// CRDPflNEZDataHolder protocol

	CRDPflNEZDataHolder();
	virtual ~CRDPflNEZDataHolder();

	//
	const RDPFLNEZDATA*	GetRdPflNEZDataRec() const {return m_pRdPflNEZDataRec;}
	void				SetRdPflNEZDataRec(const RDPFLNEZDATA*);
	
	// Overridden methods from AcDbObject
	virtual Acad::ErrorStatus   dwgInFields(AcDbDwgFiler* filer);
	virtual Acad::ErrorStatus   dwgOutFields(AcDbDwgFiler* filer) const;

protected:
	RDPFLNEZDATA*			m_pRdPflNEZDataRec;
};
MAKE_ACDBOPENOBJECT_FUNCTION(CRDPflNEZDataHolder);
*/

//SurvUtilEntRelation.cpp


#include "SurvUtilEntRelation.h"


CSurvUtilEntRelation::CSurvUtilEntRelation()
{
    m_iDbObjID = 0L;
}

CSurvUtilEntRelation::~CSurvUtilEntRelation()
{
}
Acad::ErrorStatus CSurvUtilEntRelation::dwgInFields(AcDbDwgFiler* filer)
{
    assertWriteEnabled();
 
    Acad::ErrorStatus es;
    es = AcDbObject::dwgInFields(filer);
    assert(es == Acad::eOk);
	{//
		AcDbHardPointerId objHardPtrID;

		es = filer->readItem(&objHardPtrID);
		m_iDbObjID = objHardPtrID;
		assert(es == Acad::eOk);
	}
	return es;
}

Acad::ErrorStatus CSurvUtilEntRelation::dwgOutFields(AcDbDwgFiler* filer) const
{
    assertReadEnabled();

    Acad::ErrorStatus es;
    es = AcDbObject::dwgOutFields(filer);
    assert(es == Acad::eOk);

	{//
		AcDbHardPointerId objHardPtrID(m_iDbObjID);

		es = filer->writeItem(objHardPtrID);
		assert(es == Acad::eOk);
	}
    return es;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
CRDPflNEZDataHolder::CRDPflNEZDataHolder()
{
    m_pRdPflNEZDataRec = 0L;
}

CRDPflNEZDataHolder::~CRDPflNEZDataHolder()
{
    if(m_pRdPflNEZDataRec != 0L)
		delete m_pRdPflNEZDataRec;
}
Acad::ErrorStatus CRDPflNEZDataHolder::dwgInFields(AcDbDwgFiler* filer)
{
    assertWriteEnabled();
 
    Acad::ErrorStatus es;
    es = AcDbObject::dwgInFields(filer);
    assert(es == Acad::eOk);
	
	if(0L != m_pRdPflNEZDataRec)
	{//
		delete m_pRdPflNEZDataRec;
		m_pRdPflNEZDataRec = 0L;
	}
	{//
		ads_binary objBinary;

		objBinary.clen = sizeof(RDPFLNEZDATA);
		objBinary.buf = 0L;

		es = filer->readItem(&objBinary);
		assert(es == Acad::eOk);
		m_pRdPflNEZDataRec = (RDPFLNEZDATA*)objBinary.buf;
	}
	return es;
}

Acad::ErrorStatus CRDPflNEZDataHolder::dwgOutFields(AcDbDwgFiler* filer) const
{
    assertReadEnabled();

    Acad::ErrorStatus es;
    es = AcDbObject::dwgOutFields(filer);
    assert(es == Acad::eOk);

	{//
		ads_binary objBinary;

		objBinary.clen = sizeof(RDPFLNEZDATA);
		objBinary.buf = m_pRdPflNEZDataRec;

		es = filer->writeItem(&objBinary);
		assert(es == Acad::eOk);
	}
    return es;
}
void CRDPflNEZDataHolder::SetRdPflNEZDataRec(const RDPFLNEZDATA* pRec)
{
	if(0L == m_pRdPflNEZDataRec)
		m_pRdPflNEZDataRec = new RDPFLNEZDATA;
	
	*m_pRdPflNEZDataRec = *pRec;
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void createDictionary()
{
    AcDbDictionary *pNamedobj;
    acdbCurDwg()->getNamedObjectsDictionary(pNamedobj,
        AcDb::kForWrite);

    // Check to see if the dictionary we want to create is

    // already present if it's not, then create it and add
    // it to the named object dictionary
    //
    AcDbDictionary *pDict;
    if (pNamedobj->getAt("ASDK_DICT", (AcDbObject*&) pDict,
        AcDb::kForWrite) == Acad::eKeyNotFound)
    {
        pDict = new AcDbDictionary;
        AcDbObjectId DictId;
        pNamedobj->setAt("ASDK_DICT", pDict, DictId);
    }
    pNamedobj->close();

    if (pDict) {
        // Create new objects to add to the new dictionary,

        // add them, then close them.
        //
        AsdkMyClass *pObj1 = new AsdkMyClass(1);
        AsdkMyClass *pObj2 = new AsdkMyClass(2);

        AcDbObjectId rId1, rId2;
        pDict->setAt("OBJ1", pObj1, rId1);
        pDict->setAt("OBJ2", pObj2, rId2);

        pObj1->close();
        pObj2->close();
        pDict->close();
    }

}
*/