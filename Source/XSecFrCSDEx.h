// XSecFrCSDEx.h : header file
//

#ifndef _XSECFRCSD_EX_H
#define _XSECFRCSD_EX_H

#include "XSecFrCSD.h"
#include "Structures.h"


void XSecFrCSDEx();

//Design data.......
class CXSecDsgnStn
{
public:
	CXSecDsgnStn();
	~CXSecDsgnStn();
	
	//Read from input file (ASCII Text)
	BOOL Read(CStdioFile& objFile, BOOL& bIsEOF, long& iLineNo, CStringArray* pStrArrayErrs = 0L);
	void DumpVars();

	double GetMinElev();
	double GetMaxOffset();
	double GetMinOffset();
	BOOL FindMinMaxOffElev(const double&, double& fMinOff, double& fMaxOff, double& fMaxElev);
	BOOL IsThisForChg(const double& fChg) const;
	
	double GetVolPerUnitLen(const double&);
	CCSDataRecArrayEx* GetOrdinates() { return &m_RecArray;}
	void CalcElevations(const double& fChg);
public:
	static double CalcArea(const double&, const double&, const double&, const double&, const double&);

private:
	BOOL _CalcElevAtChg(const double& fChg, double& fElev) const;
	BOOL _ParseCSDDsgnStartLine(const CStringArray& strArray, double& fStartChg, double& fEndChg, double& fDatum, double& fLongSlope);
	BOOL _ParseCSDLineInput(const CStringArray& strArray, CSDRECEX* pRecParam);
	void _SortRecsByOffset();
	int	 _GetMinOffsetDistIndex(CCSDataRecArrayEx&);
private:
	double	m_fChgStart;
	double	m_fChgEnd;
	double	m_fRLOfCenPt;
	double	m_fLongSlope;
	//NOTE:
	//CSDREC.dOffset == Distance(Offset)
	//CSDREC.dElev == Slope (1 in X)
	CCSDataRecArrayEx m_RecArray;
};
typedef CTypedPtrArray<CPtrArray, CXSecDsgnStn*> CXSecDsgnStnArray;

//Field data.......
class CXSecStn
{
public:
	CXSecStn();
	~CXSecStn();
	//Read from input file (ASCII Text)
	BOOL Read(CStdioFile& objFile, BOOL& bIsEOF, long& iLineNo);
	void DumpVars();

	BOOL CalcCutOrFillValue(CXSecDsgnStnArray&, double&, double&);
	BOOL Draw(const AcGePoint3d&, const CSDDWGPARAMEX&, CXSecDsgnStnArray&, CStdioFile* = 0L);

	BOOL FindMinMaxOffElev(double& fMinOff, double& fMaxOff, double& fMaxElev);
	BOOL GetExtents(const CSDDWGPARAMEX& recDwgAttribs, CXSecDsgnStnArray& arrayDsgnDataParan, double& fWid, double& fHt, AXISDETAIL* pRec);
	double GetMaxOffset();
	double GetMinOffset();
	double GetChainage() const { return m_fChg;}
	double GetMinElev();
	BOOL GetElevAt(const double& fOffset, double&);


private:
	double GetVolPerUnitLen(CXSecDsgnStn*, const double&, CStdioFile* = 0L);

public:
	static BOOL ParseCSDLineInput(const CStringArray& strArray, CSDREC* pRecParam);
	static BOOL ParseCSDStartLine(const CStringArray&, double& fChg, double& fDatum);

	static void RectangleLL(const AcGePoint3d&, const double&, const double&, const char*);
	static void RectangleLR(const AcGePoint3d&, const double&, const double&, const char*);
	static void RectangleUL(const AcGePoint3d&, const double&, const double&, const char*);
	static void AnnotateTopSurface(const AcGePoint3dArray& arraySurfacePts, const double& fBlkScale, const double& fMinSpa, const char* pszBlock, const char* pszLyr);

private:
	static BOOL _FindPointOnAlgn(const AcGePoint3dArray&, const double&, AcGePoint3d&, double&);
	void _DrawAxis(const CSDDWGPARAMEX&, const AcGePoint3d&, AcGePoint2d&, const AXISDETAIL&);
	void _DrawOrdinates(const CSDDWGPARAMEX&, const AcGePoint2d&, const AXISDETAIL&);
	void _DrawDsgnOrdinates(CXSecDsgnStn* pXSecDsgnStn, const CSDDWGPARAMEX& RecParam, const AcGePoint2d& ptGrphOrg, const AXISDETAIL& recAxisDet);
	void _DrawLabel(const double&, const double&, const CSDDWGPARAMEX&, const AcGePoint3d&, const AXISDETAIL&);

	CXSecDsgnStn* _GetAppropriateDsgnData(const CXSecDsgnStnArray&) const;
	void _SortByOffset(CCSDataRecArray&);
	int	 _GetMinOffsetDistIndex(const CCSDataRecArray& RecArray);
public:
	const static double m_fTxtWidMF;
private:
	CCSDataRecArray m_RecArray;
	double	m_fChg;
	double	m_fDatum;
};
typedef CTypedPtrArray<CPtrArray, CXSecStn*> CXSecStnArray;

class CXSecDwgGenerator
{
public:
	CXSecDwgGenerator(const CSDDWGPARAMEX&);
	~CXSecDwgGenerator();
public:
	BOOL Draw(const AcGePoint3d&, CStdioFile* = 0L, CStringArray* = 0L);
	BOOL CalcVolume(CStdioFile* = 0L, CStringArray* = 0L);
	
private:
	BOOL _ReadDesignData(CStringArray* = 0L);//Read from input file (ASCII Text)
	BOOL _ReadFieldData(CStringArray* = 0L);//Read from input file (ASCII Text)
	BOOL _GetNextSectionRange(const double& fCurRowDepth, const int& iStartIndex, int& iEndIndex, double& fMaxHeight, BOOL&, CStringArray* = 0L);
	BOOL _GetMaxCSBoundRectHt(const int& iStartIndex, int& iEndIndex, double& fMaxHeight);
private:
	static BOOL _SortFieldData(CXSecStnArray&);
	static BOOL _IsFieldDataXists(const CXSecStn*, const CXSecStnArray&);

private:
	const CSDDWGPARAMEX&	m_DwgAttribs;
	CXSecStnArray			m_ArrayOrgData;
	CXSecDsgnStnArray		m_ArrayDsgnData;
};

#endif //_XSECFRCSD_EX_H

