// XSecFrXLS.h : header file
//

#ifndef _XSECFRXLS_H
#define _XSECFRXLS_H

#include "XSecFrCSDEx.h"
#include "DrawCSFrXLSDlg.h"

//Field data.......
class CXSecFrXLS
{
public:
	CXSecFrXLS();
	~CXSecFrXLS();

	BOOL AddOrdinate(const NEZXSECORDINATE&);
	void SetChainage(const double& fVal) { m_fChg = fVal;}
	void SetDatum(const double& fVal) { m_fDatum = fVal;}
	double GetDatum() const { return m_fDatum;}

	const CNEZXSecOrdinateArray& GetOrdinates() const {return m_RecArray;}
	double GetMaxOffset();
	double GetMinOffset();
	double GetChainage() const { return m_fChg;}
	double GetMinElev();

	void Reset();
	BOOL Draw(const AcGePoint3d&, const XLSXSECDWGPARAM&);

	BOOL FindMinMaxOffElev(double& fMinOff, double& fMaxOff, double& fMaxElev);
	BOOL GetExtents(const XLSXSECDWGPARAM& recDwgAttribs, double& fWid, double& fHt, AXISDETAIL* pRec);
	void DumpVars();
private:
	void _DrawAxis(const XLSXSECDWGPARAM&, const AcGePoint3d&, AcGePoint2d&, const AXISDETAIL&);
	void _DrawOrdinates(const XLSXSECDWGPARAM&, const AcGePoint2d&, const AXISDETAIL&);
	void _DrawLabel(const XLSXSECDWGPARAM&, const AcGePoint3d&, const AXISDETAIL&);

	void _SortByOffset(CNEZXSecOrdinateArray&);
	int	 _GetMinOffsetDistIndex(const CNEZXSecOrdinateArray& RecArray);
public:
	const static double m_fTxtWidMF;
private:
	CNEZXSecOrdinateArray	m_RecArray;
	double				m_fChg;
	double				m_fDatum;
};
typedef CTypedPtrArray<CPtrArray, CXSecFrXLS*> CXSecFrXLSArray;


class CXSecFrXLSDwgGenerator
{
public:
	CXSecFrXLSDwgGenerator(const XLSXSECDWGPARAM&);
	~CXSecFrXLSDwgGenerator();
public:
	BOOL Add(const CXSecFrXLS&);
	void Reset();
	BOOL Draw(const AcGePoint3d&, CStringArray* = 0L);
	
private:
	BOOL _GetNextSectionRange(const double& fCurRowDepth, const int& iStartIndex, int& iEndIndex, double& fMaxHeight, BOOL&, CStringArray* = 0L);
	BOOL _GetMaxCSBoundRectHt(const int& iStartIndex, int& iEndIndex, double& fMaxHeight);

private:
	const XLSXSECDWGPARAM&	m_DwgAttribs;
	CXSecFrXLSArray			m_ArrayOrgData;
};

#endif //_XSECFRXLS_H

