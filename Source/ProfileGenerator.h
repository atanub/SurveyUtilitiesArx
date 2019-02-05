//ProfileGenerator.h

#ifndef _PROFILEGENERATOR_H_
#define _PROFILEGENERATOR_H_

//------------------------ UNDER CONSTRUCTION ------------------------ 
//------------------------ UNDER CONSTRUCTION ------------------------ 
//------------------------ UNDER CONSTRUCTION ------------------------ 
//------------------------ UNDER CONSTRUCTION ------------------------ 
//------------------------ UNDER CONSTRUCTION ------------------------ 
//
//CProfileDrawingParams
////////////////////////////////////////////////////////////////////////////////////
//
class CProfileDrawingParams
{
public:
	CProfileDrawingParams();
	CProfileDrawingParams(const CProfileDrawingParams&);
	~CProfileDrawingParams();

protected:
	CStringArray	m_arrayTags;
	CStringArray	m_arrayLayers;
	
	double			m_dDatum;
	double			m_dXScale,
					m_dYScale,
					m_dLabTxtSize,
					m_dNumAnnTxtSize,
					m_dRowHt;

	double			m_fTxtAnnotationAngleInDeg,
					m_fTxtAnnotationMargin;

	//Top Surface Related.......
	CString			m_szXistTopSurfAnnBlk;
	double			m_dXistTopSurfAnnBlkSpaceMin,
					m_dXistTopSurfAnnBlkScale;
};

//
//CProfileOrdinate
////////////////////////////////////////////////////////////////////////////////////
//
class CProfileOrdinate
{
public:
	CProfileOrdinate();
	CProfileOrdinate(const CProfileOrdinate&);
	~CProfileOrdinate();
public:
	//facility to Draw Itself....

protected:
	long								m_lBitCode;
	double								m_dChainage;
	CTypedPtrArray<CPtrArray, double*>	m_arrayOrdinateAttributes;

	char*								m_pszDesc;
	char*								m_pszBlock;
};

//
//CProfileOrdinateGroup
////////////////////////////////////////////////////////////////////////////////////
//
class CProfileOrdinateGroup
{
public:
	CProfileOrdinateGroup();
	CProfileOrdinateGroup(const CProfileOrdinateGroup&);
	~CProfileOrdinateGroup();

public:
	//facility to Draw something on the group of Ordinates...


protected:
	CTypedPtrArray<CPtrArray, CProfileOrdinate*>	m_arrayOfOrdinates;
};

//
//CProfileTag
////////////////////////////////////////////////////////////////////////////////////
//
class CProfileTag
{
public:
	CProfileTag(const char* pszDescription);
	CProfileTag(const CProfileTag&);
	~CProfileTag();
public:
	//facility to Draw Itself....
	bool	Draw(const double& fTxtHt, const char* pszLayer) const;

public:
	void		SetYLocation(const double& fLocation) { m_dOrdinateY = fLocation;}
	double		GetYLocation() const { return m_dOrdinateY;}
	
	const char*	GetDescription() const { return m_strDescription;}

protected:
	//Calculated during Run time....
	double								m_dOrdinateY;
protected:
	CString								m_strDescription;
};

//
//CProfileGenerator
////////////////////////////////////////////////////////////////////////////////////
//
class CProfileGenerator
{
public:
	CProfileGenerator();
	CProfileGenerator(const CProfileGenerator&);
	~CProfileGenerator();
public:
	bool Draw();

public:
	bool AddOrdinate(const char* pszTag, const double& fChainage, const double& fValueToPrint, const double* pfValueToDraw);
protected:
	CTypedPtrArray<CPtrArray, CProfileTag*>			m_arrayOfTags;
	CProfileOrdinateGroup							m_ordinateGroup;
	CProfileDrawingParams*							m_profileDrawingParams;
};

#endif //_PROFILEGENERATOR_H_
