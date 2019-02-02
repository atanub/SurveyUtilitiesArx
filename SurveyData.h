//SurveyData.h
//
#ifndef _SURVEYDATA_H
#define _SURVEYDATA_H

#include "stdafx.h"
#include <afxtempl.h>
#include <acgi.h>
#include "XSecFrXLS.h"
#include "ProfileFrXLS.h"

#include "Util.h"

typedef struct tag_FHeader
{
	short	iVerNoPart01;
	short	iVerNoPart02;
	short	iVerNoPart03;
} OUTFILE_HEADER;

typedef struct tag_SurveyPt
{
	double fNorthing;
	double fEasting;
	double fElev;
} SURVEYPOINT;
typedef struct tag_POINT3D
{
	double x;
	double y;
	double z;
} POINT3D;

typedef struct tag_Mark
{
	long lFBookID;
	long lPtSlNo;
	long lPtSlNoSub;
} MARK;

typedef struct tag_MasterData
{
	MARK		Mark;
	SURVEYPOINT SPoint;
	char		szDesc[_MAX_PATH];
} MASTERDATA;

typedef struct tag_FBookData
{
	long		lPtNo;
	SURVEYPOINT SPoint;
	char		szDesc[_MAX_PATH];
	char		szXtraDesc[_MAX_PATH];
} FIELDBOOKDATA;

typedef struct tag_PRData
{
	long		lBit;
	MARK		Mark;
	char		szText[_MAX_PATH];
	char		szBlock[_MAX_PATH];
} PROFILEDATA;

//typedef struct tag_InsPoint
//{
//	SURVEYPOINT Point;
//	char szRemark[_MAX_REM_SIZE];
//} INSPOINTREC;
//typedef CTypedPtrArray<CPtrArray, INSPOINTREC*> CInsPointRecs;

typedef CTypedPtrArray<CPtrArray, FIELDBOOKDATA*> CFBookDataRecs;
typedef CTypedPtrArray<CPtrArray, MASTERDATA*> CMasterDataRecs;
typedef CTypedPtrArray<CPtrArray, MARK*> CMarkRecs;
typedef CTypedPtrArray<CPtrArray, SURVEYPOINT*> CSurveyPoints;
typedef CTypedPtrArray<CPtrArray, PROFILEDATA*> CPRDataRecs;

typedef struct tag_RoadXSecData
{
	long			lID;
	CMarkRecs		MarkArray;
} ROADXSECDATA;

typedef struct tag_XSecData
{
	double			dChainage;
	CPRDataRecs		RecArray;
} XSECDATA;
typedef struct tag_XSNOutStruct
{
	long	lBit;
	double	dOffset;
	double	dElev;
	CString strTxt;
	CString strBlk;
	double	dNorth;
	double	dEast;
} XSNOUT_STRUCT;

typedef CTypedPtrArray<CPtrArray, XSECDATA*> CXSNDataRecs;
typedef CTypedPtrArray<CPtrArray, ROADXSECDATA*> CRXSecDataRecs;

class CSurveyData : public CObject
{
public:
	//;******************************************************
	//;*Bit Code Specification for each Line of Station data
	//;*	0 = No Text/Block Annotation & Ordinate Line
	//;*	1 = Only Draws Ordinate Line
	//;*	2 = Only Draws Text Annotation
	//;*	4 = Only Inserts the specified Blockname
	//;******************************************************
	//typedef enum {Nothing = 0, OrdLine = 1, OnlyTxt = 2, OnlyBlk = 4, MidPt = 8, OrdLineNTxt = 3, OrdLineNBlk = 5, TxtNBlk = 6, OrdLineTxtNBlk = 7} BitCode;
	typedef enum {Nothing = 0, OrdLine = 1, OnlyTxt = 2, OnlyBlk = 4, MidPt = 8/*, SuppressBaseAnn = 16*/} BitCode;
public:
	CSurveyData();
	~CSurveyData();
	
	void Reset();

	const CMasterDataRecs*	GetMasterDataRecPtr()	const {return &(m_MasterDataRecs);}
	const CMarkRecs*		GetPIDataRecPtr()		const {return &(m_PIDataRecs);}
	const CMarkRecs*		GetPINDataRecPtr()		const {return &(m_PINDataRecs);}
	const CPRDataRecs*		GetPRDataRecPtr()		const {return &(m_PRDataRecs);}
	const CRXSecDataRecs*	GetRXSecDataRecPtr()	const {return &(m_RXSecDataRecs);}
	const CXSNDataRecs*		GetXSNDataRecPtr()		const {return &(m_XSNDataRecs);}

	BOOL GeneratePIDOutput(const CString&, const CString&, const BOOL&, const double& = 1.0) const;
	BOOL GenerateCSDOutput(const CString&, const BOOL&, const double&, const CString&, const double&, const CString&, const double& = 1.0) const;
	BOOL GeneratePINOutput(const CString&, const BOOL&, const CString&, const double&, const CString&, const double&, const double&, const BOOL&, const BOOL&, const BOOL&, const double& = 1.0) const;
	
	BOOL GetXSNDataForDwg(CXSecFrXLSDwgGenerator&, const double& = 1.0) const;
	BOOL GetPFLDataForDwg(CProfileFrXLS&, const double& = 1.0) const;

	//Join Selected Points
	BOOL GenerateJSPOutput(const CString&, const CString&, const BOOL&, const CMasterDataRecs&, const double& = 1.0) const;
	BOOL CreateSPtArrayFromMarkArray(const CMarkRecs&, CSurveyPoints&) const;

	BOOL WriteData(const CString&) const;
	BOOL ReadData(const CString&);

	static BOOL ParsePointStr(const CString&, long&, long&, const char = '-');
	static BOOL IsNumeric(const CString&);
	static CString MarkToString(const MARK&);
	static CString MarkToShortString(const MARK&);

	static void DeallocateRXDPtrArray(CRXSecDataRecs&);
	static void DeallocateXSNPtrArray(CXSNDataRecs&);

	static long	SetBitFlag(const long&, const short&, const BOOL&);
	static BOOL	GetBitFlag(const long&, const short&);
	
	static int FindMark(const MARK&, const CMasterDataRecs&);
	static int FindMark(const MARK&, const CMarkRecs&);
	static int FindMark(const MARK&, const CPRDataRecs&);
	static int Find(const UINT&, const CUIntArray&);
	
// Operations
public:
	BOOL CopyMasterDataRecs(const CMasterDataRecs&, CMasterDataRecs&) const;
	BOOL CopyPIDataRecs(const CMarkRecs&, CMarkRecs&) const;
	BOOL CopyPRDataRecs(const CPRDataRecs&, CPRDataRecs&) const;
	BOOL CopyRXSecDataRecs(const CRXSecDataRecs&, CRXSecDataRecs&) const;

	BOOL ImportMasterDataRec(const MASTERDATA*);
	BOOL ImportPIDataRec(const MARK*);
	BOOL ImportRXDataRec(const ROADXSECDATA*);
	BOOL ImportPINDataRec(const MARK*);
	BOOL ImportPRDataRec(const PROFILEDATA*);
	BOOL ImportXSNDataRec(const XSECDATA*);

	void SetAutoChainageCalnInXSN(const BOOL& bFlag) { m_bAutoChainageCalnInXSN = bFlag;}

	static void SortXSNOutRecASC(XSNOUT_STRUCT*, const int&);
	static int CmpXSNOutRecASC(const void*, const void*);
private:
	BOOL _GetPoint3DFromMaster(const MARK&, POINT3D&, const double& = 1.0) const;

	BOOL _CanCopyMarkArray(const CMarkRecs&) const;
	void _ConvertSurveyPtToPoint3D(const SURVEYPOINT*, POINT3D&, const double& = 1.0) const;

	BOOL _GetPRDLimit(double&, double&, const double& = 1.0) const;

	//Geometoric Utility
	double _Angle(const POINT3D&, const POINT3D&) const;
	double _Distance2D(const POINT3D&, const POINT3D&) const;
	double _Distance2D(const SURVEYPOINT*, const SURVEYPOINT*) const;
	BOOL _IsPtOnLine2D(const POINT3D&, const POINT3D&, const POINT3D&) const;
	BOOL _IsParallel2D(const POINT3D&, const POINT3D&, const POINT3D&, const POINT3D&) const;
	BOOL _Inters2D(const POINT3D&, const POINT3D&, const POINT3D&, const POINT3D&, POINT3D&, const BOOL& = FALSE) const;

	BOOL _FindRotAngle(const CSurveyPoints&, double&) const;
	void _FindEdgePoints(const CSurveyPoints&, SURVEYPOINT&, SURVEYPOINT&) const;
	void _FindFurthestPtIndex(const CSurveyPoints&, const SURVEYPOINT&, int&) const;

	BOOL _FindChainage(const MARK&, double&, double&, const double& = 1.0) const;


private://Read/Write ..
	void _WriteMasterData(CFile&) const;
	void _WriteMarkArray(CFile&, const CMarkRecs&) const;
	void _WritePRData(CFile&, const CPRDataRecs&) const;
	void _WriteRXSecData(CFile&) const;
	void _WriteXSecData(CFile&) const;

	void _ReadMasterData(CFile&);
	void _ReadMarkArray(CFile&, CMarkRecs&) const;
	void _ReadPRData(CFile&, CPRDataRecs&) const;
	void _ReadRXSecData(CFile&);
	void _ReadXSecData(CFile&);

private:
	CMasterDataRecs		m_MasterDataRecs;
	CMarkRecs			m_PIDataRecs;
	CMarkRecs			m_PINDataRecs;//Insert Points
	CPRDataRecs			m_PRDataRecs;
	CRXSecDataRecs		m_RXSecDataRecs;
	CXSNDataRecs		m_XSNDataRecs;//X Sectional view

	BOOL				m_bAutoChainageCalnInXSN;//X Sectional Calculate Chainage automatically

private:
	static const OUTFILE_HEADER	m_OutFileHeader;
	static const char			m_chPointNoSeparator;
	static const double			m_dAllowebleErr;
};

#endif /*  _SURVEYDATA_H */
