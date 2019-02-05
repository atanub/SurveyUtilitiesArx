// RdXSecFrDWG.h : header file
//

#ifndef _RDXSECFRDWG_H
#define _RDXSECFRDWG_H

#include "RdXSecFrDWGDlg.h"


class CRdXSecFrDWG
{
protected:
	CRdXSecFrDWG();
	~CRdXSecFrDWG();
protected:
	class CPointAndLyrInfoCache
	{
	public:
		CPointAndLyrInfoCache() {;}
		~CPointAndLyrInfoCache() {;}
	public:
		const char*		FindLayer(const AcGePoint3d*) const;
	public:
		AcGePoint3dArray	m_arrayForPoints;
		CStringArray		m_arrayLyrNames;
	};
public:
	static void RdXSecFrDWG(const RDXSECOFFELEVDATA&);
	static void RevertPLine();
protected:
	static BOOL XtractVertsFr3DPLine(ads_name, AcGePoint3dArray&, CString* = 0L);
	static BOOL GetPIData(AcGePoint3dArray&);
	static BOOL GetXSecPtsArray(AcDbVoidPtrArray&);
	static BOOL GetUserSpecXSecRefLines(CPointPair3DPtrArray&);

	static void DeAllocateXSecPtsArray(AcDbVoidPtrArray&);
	static BOOL SortXSecPtsArray(const AcGePoint3dArray&, AcDbVoidPtrArray&);
	static BOOL GetChainage(const AcGePoint3dArray*, const AcGePoint3dArray&, double&, AcGePoint3d* = 0L, BOOL* = 0L);
	static long GetNearestXSec(const AcGePoint3dArray&, const AcDbVoidPtrArray&);
	static BOOL GetDatumValue(const AcGePoint3dArray&, const double&, double&);
	static BOOL GetNextSectionRange(const int& iStartIndex, const double& fCurRowDepth, const AcDbVoidPtrArray& Array, const AcGePoint3dArray& PIDArray, const AcGePoint3dArray& DatumRangeArray, const RDXSECOFFELEVDATA& RecParam, int& iEndIndex, double& fLeftOfXAxisWid, double& fMaxHeight);

	static BOOL ReArrangeCSData(AcGePoint3dArray& arrayXSecPts, const AcGePoint3dArray& PIDArray, const AcGePoint3d& ptMidOfXSec);

	static BOOL DrawAllSecs(const AcGePoint3d&, const AcDbVoidPtrArray&, AcGePoint3dArray&, const AcGePoint3dArray&, const RDXSECOFFELEVDATA&, const double&, const double&, const double&);
	static void DrawAxis(const AcGePoint3d&, const RDXSECOFFELEVDATA&, const double&, const double&, const double&, const double&, AcGePoint3d&);
	static void DrawOrdinates(const AcGePoint3d&, const AcGePoint3dArray&, const AcGePoint3d&, const RDXSECOFFELEVDATA&, const double&, const double&, const double&, const double&, CStdioFile*, CPointAndLyrInfoCache* pObjPointAndLyrInfoCache = 0L);

	static BOOL DrawAllSections(const AcGePoint3d&, const AcDbVoidPtrArray&, const AcGePoint3dArray&, const AcGePoint3dArray&, const RDXSECOFFELEVDATA&, CPointAndLyrInfoCache* pObjPointAndLyrInfoCache = 0L);
	static BOOL DrawCSRow(const AcGePoint3d& ptLLCStart, const int& iStartIndex, const int& iEndIndex, const AcDbVoidPtrArray& Array, const AcGePoint3dArray& PIDArray, const AcGePoint3dArray& DatumRangeArray, const RDXSECOFFELEVDATA& RecParam, const double& fLeftOfXAxisWid, const double& fCSHeight, const CString& strCSGridLyr, const AcGePoint3d& ptULOfNPlate, int& iLastDrawnCS, CStdioFile* pOutFile, CPointAndLyrInfoCache* pObjPointAndLyrInfoCache = 0L);

	static BOOL CalculateOffset(const AcGePoint3d&, const AcGePoint3dArray&, const AcGePoint3d&, double&);

	static void RectangleUL(const AcGePoint3d&, const double&, const double&, const char*);
	static void RectangleLR(const AcGePoint3d&, const double&, const double&, const char*);
	static BOOL IsCSDwgPossible(const AcGePoint3d&, const double&, const double&, const AcGePoint3d&, const double&, const double&);
	static BOOL IsPointWithinRect(const AcGePoint3d&, const AcGePoint3d&, const double&, const double&);

	static BOOL FindPathContainingMinMaxLevels(const AcDbVoidPtrArray&, const BOOL, AcGePoint3dArray&);
	static BOOL CalcChainageOfPropCSD(const AcGePoint3dArray&, const POINTPAIR3D*, double&);
	static BOOL TransformAllXSecPoints(const AcGePoint3dArray&, const AcDbVoidPtrArray&, CPointPair3DPtrArray&);
	static BOOL TransformXSecData(AcGePoint3dArray&, const AcGePoint3d&, const AcGePoint3d&);
	//
	static BOOL FindMinMaxOffElevInStn(const AcGePoint3dArray&, const AcGePoint3d&, double&, double&, double&, double&);
	static int FindMaxLenOfDatumStr(const AcGePoint3dArray&);
	//
	static void ShowInvalidCSDataSeg(const AcGePoint3dArray&, const CString&);
	//
	static BOOL WritePIDFile(const AcGePoint3dArray& PIDArray, CStdioFile* pOutFile);
	static CString PrintDouble(const double);
	//
	static BOOL GetEqIntervalOffsetDist(const double&, const double&, const double&, const double&, const double&, const double&, AcGePoint2dArray&);
	static BOOL GenOrdsInEqInterval(AcGePoint3dArray&, const AcGePoint3d&, const double&, AcGePoint3dArray*);
	static BOOL GenOrdsInEqInterval(const AcDbVoidPtrArray&, const AcGePoint3dArray&, const double&, AcGePoint3dArray*);
	//
protected:
	static const char*	m_pszLyrDebug;
	static const char*	m_pszGenLyrName;
	static const double m_fMinBufferInDWGUnits;
	static const double m_fDatumIsMultipleOf;
};

#endif //_RDXSECFRDWG_H

