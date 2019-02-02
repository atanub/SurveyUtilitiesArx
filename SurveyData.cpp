//SurveyData.cpp
//
#include "stdafx.h"
#include "excel8.h"
#include "SurveyData.h"
#include "SurvUtilApp.h"

const char CSurveyData::m_chPointNoSeparator = '.';
const double CSurveyData::m_dAllowebleErr = 1e-6;
const OUTFILE_HEADER CSurveyData::m_OutFileHeader = {1, 0, 1};

CSurveyData::CSurveyData()
{
	m_bAutoChainageCalnInXSN = FALSE;
}
CSurveyData::~CSurveyData()
{
	Reset();
}
void CSurveyData::Reset()
{
	DeallocatePtrArray(m_MasterDataRecs);
	DeallocatePtrArray(m_PIDataRecs);
	DeallocatePtrArray(m_PRDataRecs);
	DeallocatePtrArray(m_PINDataRecs);
	DeallocateRXDPtrArray(m_RXSecDataRecs);
	DeallocateXSNPtrArray(m_XSNDataRecs);
}
BOOL CSurveyData::ImportMasterDataRec(const MASTERDATA* pRec)
{
	if(FindMark(pRec->Mark, m_MasterDataRecs) >= 0)
		return FALSE;//Mark already present

	MASTERDATA* pNewRec;
	pNewRec = new MASTERDATA;
	memset(pNewRec, 0, sizeof(MASTERDATA));
	memcpy(pNewRec, pRec, sizeof(MASTERDATA));
	m_MasterDataRecs.Add(pNewRec);
	return TRUE;
}
BOOL CSurveyData::ImportPIDataRec(const MARK* pRec)
{
	if(FindMark(*pRec, m_MasterDataRecs) < 0)
		return FALSE;//Mark not present
	MARK* pNewRec;
	pNewRec = new MARK;
	memset(pNewRec, 0, sizeof(MARK));
	memcpy(pNewRec, pRec, sizeof(MARK));
	m_PIDataRecs.Add(pNewRec);
	return TRUE;
}
BOOL CSurveyData::ImportPINDataRec(const MARK* pRec)
{
	if(FindMark(*pRec, m_MasterDataRecs) < 0)
		return FALSE;//Mark not present
	MARK* pNewRec;
	pNewRec = new MARK;
	memset(pNewRec, 0, sizeof(MARK));
	memcpy(pNewRec, pRec, sizeof(MARK));
	m_PINDataRecs.Add(pNewRec);
	return TRUE;
}
BOOL CSurveyData::ImportPRDataRec(const PROFILEDATA* pRec)
{
	if(FindMark(pRec->Mark, m_MasterDataRecs) < 0)
		return FALSE;//Mark not present
	PROFILEDATA* pNewRec;
	pNewRec = new PROFILEDATA;
	memset(pNewRec, 0, sizeof(PROFILEDATA));
	memcpy(pNewRec, pRec, sizeof(PROFILEDATA));
	m_PRDataRecs.Add(pNewRec);
	return TRUE;
}
BOOL CSurveyData::ImportRXDataRec(const ROADXSECDATA* pRec)
{
	{//Check each Mark's existance
		int i, iLim;

		iLim = pRec->MarkArray.GetSize();
		for(i = 0; i < iLim; i++)
		{
			if(FindMark(*(pRec->MarkArray[i]), m_MasterDataRecs) < 0)
				return FALSE;//Mark not present
		}
	}
	ROADXSECDATA* pNewRec;
	pNewRec = new ROADXSECDATA;
	pNewRec->lID = pRec->lID;
	if(!CopyPIDataRecs(pRec->MarkArray, pNewRec->MarkArray))
	{
		delete pNewRec;
		return FALSE;
	}
	m_RXSecDataRecs.Add(pNewRec);
	return TRUE;
}
BOOL CSurveyData::ImportXSNDataRec(const XSECDATA* pRec)
{
	int i;

	for(i = 0; i < pRec->RecArray.GetSize(); i++)
	{
		PROFILEDATA* pProfRec;
		
		pProfRec = pRec->RecArray[i];
		if(FindMark(pProfRec->Mark, m_MasterDataRecs) < 0)
			return FALSE;//Mark not present
	}
	{//Data is valid...so add..
		XSECDATA* pNewRec;
		
		pNewRec = new XSECDATA;
		pNewRec->dChainage = pRec->dChainage;

		for(i = 0; i < pRec->RecArray.GetSize(); i++)
		{
			PROFILEDATA* pProfRec;
			PROFILEDATA* pNewProfRec;
			
			pProfRec = pRec->RecArray[i];
			pNewProfRec = new PROFILEDATA;
			memcpy(pNewProfRec, pProfRec, sizeof(PROFILEDATA));
			pNewRec->RecArray.Add(pNewProfRec);
		}
		m_XSNDataRecs.Add(pNewRec);
	}
	return TRUE;
}

BOOL CSurveyData::IsNumeric(const CString& strParam)
{
	int i, iLim;
	
	iLim = strlen((LPCSTR)strParam);
	for(i = 0; i < iLim; i++)
	{
		if(((strParam[i] > '9') || (strParam[i] < '0')))
			return FALSE;
	}
	return TRUE;
}

BOOL CSurveyData::ParsePointStr(const CString& strMark, long& lPtSlNo, long& lPtSlNoSub, const char chSeparator /*= '-'*/)
{
	int iIndex;
	CString strTemp;

	lPtSlNo = lPtSlNoSub = 0L;
	iIndex = strMark.Find(chSeparator);
	if(iIndex <= 0)
	{
		int i, iLim;
		iLim = strlen((LPCSTR)strMark);
		for(i = 0; i < iLim; i++)
		{
			if(((strMark[i] > '9') || (strMark[i] < '0')) && (strMark[i] != chSeparator))
				return FALSE;
		}
		lPtSlNo = atol((LPCSTR)strMark);
		return TRUE;
	}
	strTemp = strMark.Mid(0, iIndex);
	if(!IsNumeric(strTemp)) return FALSE;
	lPtSlNo = atol((LPCSTR)strTemp);

	strTemp = strMark.Mid(iIndex + 1);
	if(!IsNumeric(strTemp)) return FALSE;
	lPtSlNoSub = atol((LPCSTR)strTemp);
	
	return TRUE;
}
BOOL CSurveyData::CopyMasterDataRecs(const CMasterDataRecs& FrRecArray, CMasterDataRecs& ToRecArray) const
{
	int i, iLim;
	
	//Check data first
	iLim = FrRecArray.GetSize();
	for(i = 0; i < iLim; i++)
	{
		if(FindMark(FrRecArray[i]->Mark, ToRecArray) >= 0)
		{
			CString strMsg, strMark;
			
			strMark = MarkToString(FrRecArray[i]->Mark);
			strMsg.Format("#%s already exists..Failed to import data", (LPCSTR)strMark);
			AfxMessageBox(strMsg, MB_ICONSTOP);
			return FALSE;
		}
	}
	iLim = FrRecArray.GetSize();
	for(i = 0; i < iLim; i++)
	{
		MASTERDATA* pRec;

		pRec = new MASTERDATA;
		memcpy(pRec, FrRecArray[i], sizeof(MASTERDATA));
		ToRecArray.Add(pRec);
	}
	return TRUE;
}
BOOL CSurveyData::_CanCopyMarkArray(const CMarkRecs& FrRecArray) const
{
	int i, iLim;

	iLim = FrRecArray.GetSize();
	for(i = 0; i < iLim; i++)
	{
		if(FindMark(*(FrRecArray[i]), m_MasterDataRecs) < 0)
		{//Mark doesn't exists
			CString strMsg, strMark;
			
			strMark = MarkToString(*(FrRecArray[i]));
			strMsg.Format("#%s doesn't exists in data base..Failed to import data", (LPCSTR)strMark);
			AfxMessageBox(strMsg, MB_ICONSTOP);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CSurveyData::CopyPIDataRecs(const CMarkRecs& FrRecArray, CMarkRecs& ToRecArray) const
{
	int i, iLim;

	if(!_CanCopyMarkArray(FrRecArray))
		return FALSE;

	iLim = FrRecArray.GetSize();
	for(i = 0; i < iLim; i++)
	{
		MARK* pRec;

		pRec = new MARK;
		memcpy(pRec, FrRecArray[i], sizeof(MARK));
		ToRecArray.Add(pRec);
	}
	return TRUE;
}
BOOL CSurveyData::CopyPRDataRecs(const CPRDataRecs& FrRecArray, CPRDataRecs& ToRecArray) const
{
	int i, iLim;

	iLim = FrRecArray.GetSize();
	for(i = 0; i < iLim; i++)
	{
		if(FindMark(FrRecArray[i]->Mark, m_MasterDataRecs) < 0)
		{//Mark doesn't exists
			CString strMsg, strMark;
			
			strMark = MarkToString(FrRecArray[i]->Mark);
			strMsg.Format("#%s doesn't exists in data base..Failed to import data", (LPCSTR)strMark);
			AfxMessageBox(strMsg, MB_ICONSTOP);
			return FALSE;
		}
	}

	iLim = FrRecArray.GetSize();
	for(i = 0; i < iLim; i++)
	{
		PROFILEDATA* pRec;

		pRec = new PROFILEDATA;
		memcpy(pRec, FrRecArray[i], sizeof(PROFILEDATA));
		ToRecArray.Add(pRec);
	}
	return TRUE;
}
BOOL CSurveyData::CopyRXSecDataRecs(const CRXSecDataRecs& FrRecArray, CRXSecDataRecs& ToRecArray) const
{
	int i, iLim;

	iLim = FrRecArray.GetSize();
	for(i = 0; i < iLim; i++)
	{
		if(!_CanCopyMarkArray(FrRecArray[i]->MarkArray))
			return FALSE;
	}

	iLim = FrRecArray.GetSize();
	for(i = 0; i < iLim; i++)
	{
		ROADXSECDATA* pRecToAdd;
		
		pRecToAdd = new ROADXSECDATA;
		CopyPIDataRecs(FrRecArray[i]->MarkArray, pRecToAdd->MarkArray);
		ToRecArray.Add(pRecToAdd);
	}
	return TRUE;
}

void CSurveyData::DeallocateRXDPtrArray(CRXSecDataRecs& Array)
{
	int i;
	ROADXSECDATA* pRec;
	for(i = 0; i < Array.GetSize(); i++)
	{
		pRec = Array[i];
		DeallocatePtrArray(pRec->MarkArray);
		delete pRec;
	}
	Array.RemoveAll();
}
void CSurveyData::DeallocateXSNPtrArray(CXSNDataRecs& Array)
{
	int i;
	XSECDATA* pRec;
	for(i = 0; i < Array.GetSize(); i++)
	{
		pRec = Array[i];
		DeallocatePtrArray(pRec->RecArray);
		delete pRec;
	}
	Array.RemoveAll();
}

BOOL CSurveyData::_GetPoint3DFromMaster(const MARK& Rec, POINT3D& PtParam, const double& dMF /*= 1.0*/) const
{
	int iIndex;
	
	iIndex = FindMark(Rec, m_MasterDataRecs);
	if(iIndex < 0) return FALSE;
	{//Extract & Convert
		const MASTERDATA* pMDRec = m_MasterDataRecs[iIndex];
		_ConvertSurveyPtToPoint3D(&(pMDRec->SPoint), PtParam, dMF);
	}
	return TRUE;
}

int CSurveyData::FindMark(const MARK& MarkToFind, const CMasterDataRecs& RecArray)
{
	int i, iLim;
	
	iLim = RecArray.GetSize();
	for(i = 0; i < iLim; i++)
	{
		MASTERDATA* pRec;
		pRec = RecArray[i];
		if((pRec->Mark.lFBookID == MarkToFind.lFBookID) &&  (pRec->Mark.lPtSlNo == MarkToFind.lPtSlNo) && (pRec->Mark.lPtSlNoSub == MarkToFind.lPtSlNoSub))
			return i;
	}
	return -1;//not found ..failure
}
int CSurveyData::FindMark(const MARK& MarkToFind, const CMarkRecs& RecArray)
{
	int i, iLim;
	
	iLim = RecArray.GetSize();
	for(i = 0; i < iLim; i++)
	{
		if((RecArray[i]->lFBookID == MarkToFind.lFBookID) &&  (RecArray[i]->lPtSlNo == MarkToFind.lPtSlNo) && (RecArray[i]->lPtSlNoSub == MarkToFind.lPtSlNoSub))
			return i;
	}
	return -1;//not found ..failure
}
int CSurveyData::FindMark(const MARK& MarkToFind, const CPRDataRecs& RecArray)
{
	int i, iLim;
	
	iLim = RecArray.GetSize();
	for(i = 0; i < iLim; i++)
	{
		if((RecArray[i]->Mark.lFBookID == MarkToFind.lFBookID) &&  (RecArray[i]->Mark.lPtSlNo == MarkToFind.lPtSlNo) && (RecArray[i]->Mark.lPtSlNoSub == MarkToFind.lPtSlNoSub))
			return i;
	}
	return -1;//not found ..failure
}
CString CSurveyData::MarkToString(const MARK& Mark)
{
	CString strToRet;
	strToRet.Format("Field Book# = %d, Main Point# = %d, Sub Point# = %d", Mark.lFBookID, Mark.lPtSlNo, Mark.lPtSlNoSub);
	return strToRet;
}
CString CSurveyData::MarkToShortString(const MARK& Mark)
{
	CString strToRet;
	strToRet.Format("%4d.%5d.%2d", Mark.lFBookID, Mark.lPtSlNo, Mark.lPtSlNoSub);
	return strToRet;
}
int	CSurveyData::Find(const UINT& iVal, const CUIntArray& Array)
{
	int i, iLim;
	
	iLim = Array.GetSize();
	for(i = 0; i < iLim; i++)
	{
		if(Array[i] == iVal)
			return i;
	}
	return -1;
}
void CSurveyData::_ConvertSurveyPtToPoint3D(const SURVEYPOINT* pSPoint, POINT3D& Point, const double& dMf /*= 1.0*/) const
{
	Point.x = pSPoint->fEasting * dMf;
	Point.y = pSPoint->fNorthing * dMf;
	Point.z = pSPoint->fElev * dMf;
}
double CSurveyData::_Distance2D(const POINT3D& Point01, const POINT3D& Point02) const
{
	double dResult;

	dResult = pow((Point01.x - Point02.x), 2.0) + pow((Point01.y - Point02.y), 2.0);
	dResult = pow(dResult, 0.5);
	
	return dResult;
}
double CSurveyData::_Distance2D(const SURVEYPOINT* pPoint01, const SURVEYPOINT* pPoint02) const
{
	POINT3D Pt01, Pt02;

	_ConvertSurveyPtToPoint3D(pPoint01, Pt01);
	_ConvertSurveyPtToPoint3D(pPoint02, Pt02);

	return _Distance2D(Pt01, Pt02);
}
BOOL CSurveyData::_IsPtOnLine2D(const POINT3D& PtToTest, const POINT3D& PtStart, const POINT3D& PtEnd) const
{
	double dA, dB, dAB;
	
	dA = _Distance2D(PtStart, PtToTest);
	dB = _Distance2D(PtEnd, PtToTest);
	dAB = _Distance2D(PtStart, PtEnd);

	if(fabs(dAB - (dA + dB)) <= m_dAllowebleErr)
		return TRUE;
		
	return FALSE;
}
BOOL CSurveyData::_IsParallel2D(const POINT3D& Point01, const POINT3D& Point02, const POINT3D& Point03, const POINT3D& Point04) const
{
	double dDelX01, dDelX02, dDelY01, dDelY02;
	
	dDelX01 = Point02.x - Point01.x;
	dDelX02 = Point04.x - Point03.x;

	dDelY01 = Point02.y - Point01.y;
	dDelY02 = Point04.y - Point03.y;
	
	if((dDelX01 == 0.0) && (dDelX02 == 0.0))
		return TRUE;//Both the lines are parallel to Y-Axis

	if((dDelY01 == 0.0) && (dDelY02 == 0.0))
		return TRUE;//Both the lines are parallel to X-Axis
	
	if((dDelX01 != 0.0) && (dDelX02 != 0.0))
	{
		if(fabs(dDelY01 / dDelX01) == fabs(dDelY02 / dDelX02))
			return TRUE;//Slopes are equal
	}
	return FALSE;
}
BOOL CSurveyData::_Inters2D(const POINT3D& PtStart01, const POINT3D& PtEnd01, const POINT3D& PtStart02, const POINT3D& PtEnd02, POINT3D& PointResult, const BOOL& bChkForOnline /*= FALSE*/) const
{
	double dResX, dResY, dDelX01, dDelX02, dDelY01, dDelY02;
	BOOL bCalcDone;

	if(_IsParallel2D(PtStart01, PtEnd01, PtStart02, PtEnd02))
		return FALSE;
		
	dDelX01 = PtEnd01.x - PtStart01.x;
	dDelX02 = PtEnd02.x - PtStart02.x;
	dDelY01 = PtEnd01.y - PtStart01.y;
	dDelY02 = PtEnd02.y - PtStart02.y;

	bCalcDone = FALSE;
	if((dDelX01 != 0.0) && (dDelX02 != 0.0))
	{
		double dC1, dC2;
		
		dC1 = dDelX01 / dDelY01;
		dC2 = dDelX02 / dDelY02;
		
		dResY = (PtStart01.x - PtStart02.x) - (dC1 * PtStart01.y) + (dC2 * PtStart02.y);
		dResY = dResY / (dC2 - dC1);
		dResX = PtStart01.x - (dC1 * PtStart01.y) + (dC1 * dResY);
		bCalcDone = TRUE;
	}
	else
	{
		if(dDelX01 == 0.0)
		{
			dResX = PtStart01.x;
			dResY = ((dResX - PtStart02.x) * (PtEnd02.y - PtStart02.y) / (PtEnd02.x - PtStart02.x)) + PtStart02.y;
			bCalcDone = TRUE;
		}
		else
		{
			if(dDelX02 == 0.0)
			{
				dResX = PtStart02.x;
				dResY = ((dResX - PtStart01.x) * (PtEnd01.y - PtStart01.y) / (PtEnd01.x - PtStart01.x)) + PtStart01.y;
				bCalcDone = TRUE;
			}
		}
	}
	VERIFY(bCalcDone);
	memset(&PointResult, 0, sizeof(PointResult));
	PointResult.x = dResX;
	PointResult.y = dResY;
	PointResult.z = 0.0;
	
	if(bChkForOnline)
	{
		if(_IsPtOnLine2D(PointResult, PtEnd02, PtStart02) && _IsPtOnLine2D(PointResult, PtEnd01, PtStart01))
			return TRUE;

		return FALSE;
	}
	return TRUE;
}
BOOL CSurveyData::_FindRotAngle(const CSurveyPoints& PtArray, double& dAngle) const
{
	int i;
	POINT3D pt3DEdge1, pt3DEdge2, pt3DTemp;
	SURVEYPOINT ptEdge1, ptEdge2;
	CSurveyPoints PIPoints;

	_FindEdgePoints(PtArray, ptEdge1, ptEdge2);
	_ConvertSurveyPtToPoint3D(&ptEdge1, pt3DEdge1);
	_ConvertSurveyPtToPoint3D(&ptEdge2, pt3DEdge2);

	if(!CreateSPtArrayFromMarkArray(m_PIDataRecs, PIPoints))
		return FALSE;
	

	for(i = 1; i < PIPoints.GetSize(); i++)
	{
		POINT3D pt3DStart, pt3DEnd;

		_ConvertSurveyPtToPoint3D(PIPoints[i - 1], pt3DStart);
		_ConvertSurveyPtToPoint3D(PIPoints[i - 0], pt3DEnd);

		if(_Inters2D(pt3DStart, pt3DEnd, pt3DEdge1, pt3DEdge2, pt3DTemp, TRUE))
		{
			dAngle = _Angle(pt3DStart, pt3DEnd);
			DeallocatePtrArray(PIPoints);
			return TRUE;
		}
	}
	DeallocatePtrArray(PIPoints);
	return FALSE;
}
double CSurveyData::_Angle(const POINT3D& pt3DStart, const POINT3D& pt3DEnd) const
{
	double dVal;
	if((pt3DEnd.y - pt3DStart.y) == 0.0)
	{
		if(pt3DEnd.x > pt3DStart.x)
			return 0.0;//|| to X-Axis
		return PI;//|| to X-Axis
	}

	if((pt3DEnd.x - pt3DStart.x) == 0.0)
	{
		if(pt3DEnd.y > pt3DStart.y)
			return (PI / 2.0);//|| to Y-Axis
		return (PI / 2.0) + PI;//|| to Y-Axis
	}

	dVal = (atan((pt3DEnd.y - pt3DStart.y) / (pt3DEnd.x - pt3DStart.x)));

	{//Test
		if(dVal < 0.0)
			dVal = PI * 2.0 + dVal;
		
		dVal = atan2((pt3DEnd.y - pt3DStart.y), (pt3DEnd.x - pt3DStart.x));
		if(dVal < 0.0)
			dVal = PI * 2.0 + dVal;
	}
	
	return dVal;
}

void CSurveyData::_FindEdgePoints(const CSurveyPoints& PtArray, SURVEYPOINT& ptEdge01, SURVEYPOINT& ptEdge02) const
{
	CUIntArray UIArray;
	int i, iIndex, iStart, iEnd;
	double dMax, dDist;
	

	for(i = 0; i < PtArray.GetSize(); i++)
	{
		_FindFurthestPtIndex(PtArray, *(PtArray[i]), iIndex);
		UIArray.Add(iIndex);
	}
	
	dMax = 0.0;
	iStart = iEnd = -1;
	for(i = 0; i < PtArray.GetSize(); i++)
	{
		dDist = _Distance2D(PtArray[i], PtArray[UIArray[i]]);
		if(dDist > dMax)
		{
			dMax = dDist;
			iStart = i;
			iEnd = UIArray[i];
		}
	}
	ptEdge01 = *(PtArray[iStart]);
	ptEdge02 = *(PtArray[iEnd]);
}
void CSurveyData::_FindFurthestPtIndex(const CSurveyPoints& PtArray, const SURVEYPOINT& PtBase, int& iIndex) const
{
	int i;
	double dMax, dDist;
	POINT3D	ptTemp, ptBase;

	_ConvertSurveyPtToPoint3D(&PtBase, ptBase);
	iIndex = -1;
	for(i = 0, dMax = 0.0; i < PtArray.GetSize(); i++)
	{
		_ConvertSurveyPtToPoint3D(PtArray[i], ptTemp);
		dDist = _Distance2D(ptTemp, ptBase);
		
		if(dDist > dMax)
		{
			dMax = dDist;
			iIndex = i;
		}
	}
}
BOOL CSurveyData::CreateSPtArrayFromMarkArray(const CMarkRecs& MarkRecArray, CSurveyPoints& PtArray) const
{
	int i, iIndex;
	for(i = 0; i < MarkRecArray.GetSize(); i++)
	{
		iIndex = FindMark(*(MarkRecArray[i]), m_MasterDataRecs);
		if(iIndex >= 0)
		{
			SURVEYPOINT* pNewRec;
			const MASTERDATA* pMData = m_MasterDataRecs[iIndex];
			
			pNewRec = new SURVEYPOINT;
			memcpy(pNewRec, &(pMData->SPoint), sizeof(SURVEYPOINT));
			PtArray.Add(pNewRec);
		}
		else
		{
			DeallocatePtrArray(PtArray);
			return FALSE;
		}
	}
	return TRUE;
}
BOOL CSurveyData::GeneratePIDOutput(const CString& strPath, const CString& strLyr, const BOOL& bIs3D, const double& dMF /*= 1.0*/) const
{
	//File Format...............
	//[X1] [Y1] [Z1]	[X2] [Y2] [Z2] LyrName 3DFlag
	CStdioFile FileToWrite;
	CString strLine;
	BOOL bFlag;

	bFlag = FileToWrite.Open(strPath, CFile::modeCreate|CFile::modeWrite|CFile::typeText);
	if(!bFlag)
	{
		ads_printf("\nERROR: Unable to open file %s", (LPCSTR)strPath);
		return FALSE;
	}
	int i, iIndexStart, iIndexEnd;
	POINT3D ptStart,  ptEnd;
	for(i = 1; i < m_PIDataRecs.GetSize(); i++)
	{
		iIndexStart = FindMark(*(m_PIDataRecs[i - 1]), m_MasterDataRecs);
		iIndexEnd	= FindMark(*(m_PIDataRecs[i - 0]), m_MasterDataRecs);
		if((iIndexStart < 0) || (iIndexEnd < 0))
			return FALSE;

		_ConvertSurveyPtToPoint3D(&(m_MasterDataRecs[iIndexStart]->SPoint), ptStart, dMF);
		_ConvertSurveyPtToPoint3D(&(m_MasterDataRecs[iIndexEnd]->SPoint), ptEnd, dMF);

		//strLine.Format("%e\t%e\t%e\t%e\t%e\t%e\t%s\t%d\n", ptStart.x, ptStart.y, ptStart.z, ptEnd.x, ptEnd.y, ptEnd.z, (LPCSTR)strLyr, bIs3D);
		strLine.Format("%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%s\t%d\n", ptStart.x, ptStart.y, ptStart.z, ptEnd.x, ptEnd.y, ptEnd.z, (LPCSTR)strLyr, bIs3D);
		FileToWrite.WriteString(strLine);
	}
	FileToWrite.Close();
	return TRUE;
}
BOOL CSurveyData::GetPFLDataForDwg(CProfileFrXLS& dwgGenerator, const double& dMF /*= 1.0*/) const
{
	CString strMsg;
	int i;
	double dCumDist;
	POINT3D PtPrev, PtCur;

	for(i = 0, dCumDist = 0.0; i < m_PRDataRecs.GetSize(); i++)
	{
		const PROFILEDATA* pPRDRec = m_PRDataRecs[i];
		XLSPROFILEORDINATE recOrdinate;

		if(!_GetPoint3DFromMaster(pPRDRec->Mark, PtCur, dMF))
		{
			strMsg.Format("ERROR: Point \"%s\" not found in Master Data..", m_PRDataRecs[i]->Mark);
			AfxMessageBox(strMsg, MB_ICONSTOP);
			return FALSE;
		}
		if(i > 0)
		{
			dCumDist += _Distance2D(PtPrev, PtCur);
		}
		recOrdinate.lBit		= pPRDRec->lBit;
		recOrdinate.dCumDist	= dCumDist;
		recOrdinate.dNorth		= PtCur.y;
		recOrdinate.dEast		= PtCur.x;
		recOrdinate.dElev		= PtCur.z;
		recOrdinate.strTxt		= pPRDRec->szText;
		recOrdinate.strBlk		= pPRDRec->szBlock;
		
		dwgGenerator.AddOrdinate(recOrdinate);

		PtPrev = PtCur;
	}
	return TRUE;
}
BOOL CSurveyData::_GetPRDLimit(double& dMaxElev, double& dTotHorzDist, const double& dMF /*= 1.0*/) const
{
	int i;
	double dCumDist;
	CString strMsg;
	POINT3D PtPrev, PtCur;

	for(i = 0, dCumDist = 0.0; i < m_PRDataRecs.GetSize(); i++)
	{
		const PROFILEDATA* pPRDRec = m_PRDataRecs[i];
		if(!_GetPoint3DFromMaster(pPRDRec->Mark, PtCur, dMF))
		{
			strMsg.Format("ERROR: %s not found in Master Data..Can't continue", m_PRDataRecs[i]->Mark);
			AfxMessageBox(strMsg, MB_ICONSTOP);
			return FALSE;
		}
		if(i > 0)
		{
			dCumDist += _Distance2D(PtPrev, PtCur);
			if(dMaxElev < PtCur.z)
				dMaxElev = PtCur.z;
		}
		else
		{
			dCumDist = 0.0;
			dMaxElev = PtCur.z;
		}
		PtPrev = PtCur;
	}
	dTotHorzDist = dCumDist;
	return TRUE;
}

BOOL CSurveyData::GeneratePINOutput(const CString& strPath, const BOOL& b3DFlag, const CString& strLyrSpLevel, const double& fTxtSize, const CString& strBlkName, const double& fBlkScale, const double& dRotAngleInRad, const BOOL& bSpLevFlag, const BOOL& bDescFlag, const BOOL& bShowPtNoFlag, const double& dMF /*= 1.0*/) const
{
	//File format 
	//[SL#] [X] [Y] [Z] [RotAngleRad] [3DFlag] [TxtSize] [BlkName] [BlkScale] [LyrName for POINT] [LyrName for SpLevel] [SpLevelFlag] [DescFlag] [Show Pt# Flag] [MarkStr]
	CStdioFile FileToWrite;
	CString strLine;
	BOOL bFlag;
	int i, iMasterRecIndex;
	CSurveyPoints SPointArray;

	bFlag = FileToWrite.Open(strPath, CFile::modeCreate|CFile::modeWrite|CFile::typeText);
	if(!bFlag)
	{
		ads_printf("\nERROR: Unable to open file %s", (LPCSTR)strPath);
		return FALSE;
	}
	CreateSPtArrayFromMarkArray(m_PINDataRecs, SPointArray);
	for(i = 0; i < SPointArray.GetSize(); i++)
	{
		MARK* pMarkRec;
		POINT3D pt3d;
		CString strLyrName, strMark;
	
		//Extract Layer from MasterData
		pMarkRec = m_PINDataRecs[i];
		iMasterRecIndex = FindMark(*pMarkRec, m_MasterDataRecs);
		if(iMasterRecIndex < 0)
			ASSERT(FALSE);//Data not properly imported
		const MASTERDATA* pMasterRec = m_MasterDataRecs[iMasterRecIndex];
		strLyrName = pMasterRec->szDesc;
		strMark.Format("(%d.%d.%d)", pMarkRec->lFBookID, pMarkRec->lPtSlNo, pMarkRec->lPtSlNoSub);

		const SURVEYPOINT* pSPoint = SPointArray[i];
		_ConvertSurveyPtToPoint3D(pSPoint, pt3d, dMF);
		//[SL#] [X] [Y] [Z] [RotAngleRad] [3DFlag] [TxtSize] [BlkName] [BlkScale] [LyrName for POINT] [LyrName for SpLevel] [SpLevelFlag] [DescFlag] [Show Pt# Flag] [MarkStr]
		strLine.Format("%d\t%.6f\t%.6f\t%.6f\t%.6f\t%d\t%.6f\t%s\t%.6f\t%s\t%s\t%d\t%d\t%d\t%s\n", (i + 1),
						pt3d.x, pt3d.y, pt3d.z, dRotAngleInRad,
						b3DFlag, fTxtSize, (LPCSTR)strBlkName, fBlkScale, (LPCSTR)strLyrName, (LPCSTR)strLyrSpLevel, bSpLevFlag, bDescFlag, bShowPtNoFlag, (LPCSTR)strMark
					  );
		FileToWrite.WriteString(strLine);
	}
	DeallocatePtrArray(SPointArray);
	return TRUE;
}
BOOL CSurveyData::GenerateCSDOutput(const CString& strPath, const BOOL& b3DFlag, const double& fTxtSize, const CString& strBlkName, const double& fBlkScale, const CString& strLyrName, const double& dMF /*= 1.0*/) const
{
	//File format 
	//[SL#] [X] [Y] [Z] [RotAngle] [3DFlag] [TxtSize] [BlkName] [BlkScale] [LyrName]
	CStdioFile FileToWrite;
	CString strLine;
	BOOL bFlag;
	int i, j;
	CSurveyPoints SPointArray;
	double dRotAngleInRad;

	bFlag = FileToWrite.Open(strPath, CFile::modeCreate|CFile::modeWrite|CFile::typeText);
	if(!bFlag)
	{
		ads_printf("\nERROR: Unable to open file %s", (LPCSTR)strPath);
		return FALSE;
	}
	for(i = 0; i < m_RXSecDataRecs.GetSize(); i++)
	{
		CreateSPtArrayFromMarkArray(m_RXSecDataRecs[i]->MarkArray, SPointArray);

		if(!_FindRotAngle(SPointArray, dRotAngleInRad))
		{
			CString strMsg;
			
			strMsg.Format("Intersection point not found for Station data #%d\nWould you like to continue ?", (i + 1));
			if(AfxMessageBox(strMsg, MB_ICONQUESTION|MB_YESNO) != IDYES)
			{
				DeallocatePtrArray(SPointArray);
				return FALSE;
			}
		}
		else
		{//Dump output to file
			//File format 
			//[SL#] [X] [Y] [Z] [RotAngle] [3DFlag] [TxtSize] [BlkName] [BlkScale] [LyrName]
			strLine.Format("*Start Station [%d]\n", i);
			FileToWrite.WriteString(strLine);
			for(j = 0; j < SPointArray.GetSize(); j++)
			{
				const SURVEYPOINT* pSPoint;
				POINT3D pt3d;
				
				pSPoint = SPointArray[j];
				_ConvertSurveyPtToPoint3D(pSPoint, pt3d, dMF);
//				strLine.Format("%d\t%d\t%e\t%e\t%e\t%e\t%d\t%e\t%s\t%e\t%s\n", (i + 1), (j + 1),
				strLine.Format("%d\t%d\t%.6f\t%.6f\t%.6f\t%.6f\t%d\t%.6f\t%s\t%.6f\t%s\n", (i + 1), (j + 1),
								pt3d.x, pt3d.y, pt3d.z, dRotAngleInRad,
								b3DFlag, fTxtSize, (LPCSTR)strBlkName, fBlkScale, (LPCSTR)strLyrName
							  );
				FileToWrite.WriteString(strLine);
			}
			strLine.Format("*End Station [%d]\n", i);
			FileToWrite.WriteString(strLine);
#ifdef _DEBUG
			ads_printf("\ndRotAngleInRad = %f", dRotAngleInRad);
#endif //_DEBUG
		}
		DeallocatePtrArray(SPointArray);
	}
	return TRUE;
}

//Join Selected Points
BOOL CSurveyData::GenerateJSPOutput(const CString& strPath, const CString& strLyr, const BOOL& b3DFlag, const CMasterDataRecs& RecArray, const double& dMF /*= 1.0*/) const
{
	//File format 
	//[SL#] [X] [Y] [Z] [X0] [Y0] [Z0] [3DFlag] [LyrName]
	CStdioFile FileToWrite;
	CString strLine;
	BOOL bFlag;
	int i;

	bFlag = FileToWrite.Open(strPath, CFile::modeCreate|CFile::modeWrite|CFile::typeText);
	if(!bFlag)
	{
		ads_printf("\nERROR: Unable to open file %s", (LPCSTR)strPath);
		return FALSE;
	}
	for(i = 1; i < RecArray.GetSize(); i++)
	{
		//File format 
		//[SL#] [X] [Y] [Z] [X0] [Y0] [Z0] [3DFlag] [LyrName]
		MASTERDATA* pStartRec;
		MASTERDATA* pEndRec;
		POINT3D ptStart, ptEnd;

		pStartRec = RecArray[i - 1];
		pEndRec = RecArray[i];

		_ConvertSurveyPtToPoint3D(&(pStartRec->SPoint), ptStart, dMF);
		_ConvertSurveyPtToPoint3D(&(pEndRec->SPoint), ptEnd, dMF);

		strLine.Format("%d\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%d\t%s\n",
						i, ptStart.x, ptStart.y, ptStart.z, ptEnd.x, ptEnd.y, ptEnd.z,
						b3DFlag, (LPCSTR)strLyr);
		FileToWrite.WriteString(strLine);
	}
	return TRUE;
}
long CSurveyData::SetBitFlag(const long& lValParam, const short& iBitPosn, const BOOL& bFlag)
{
	int iVal;
	long lVal;
	BOOL bCurFlag;

	iVal = (int)pow(2.0, iBitPosn);
	bCurFlag = (lValParam & iVal);
	if(bCurFlag == bFlag)
		return lValParam;//No need to change...Flag is already 'On'
	if(bCurFlag)
	{//Bit flag is 'On'..Turn it 'Off'
		lVal = lValParam ^ iVal;
		return lVal;
	}
	ASSERT(bCurFlag == FALSE);
	//Bit flag is 'Off'..Turn it 'On'
	lVal = lValParam|iVal;
	return lVal;
}
BOOL CSurveyData::GetBitFlag(const long& lValParam, const short& iBitPosn)
{
	int iVal;

	iVal = (int)pow(2.0, iBitPosn);
	return (lValParam & iVal) ? TRUE : FALSE;
}
BOOL CSurveyData::GetXSNDataForDwg(CXSecFrXLSDwgGenerator& dwgGenerator, const double& dMF /*= 1.0*/) const
{
	CWaitCursor WaitCursor;
	int i, j;
	XSECDATA* pRec;
	CString strMsg;
	CArray<XSNOUT_STRUCT, XSNOUT_STRUCT> TmpArray;

	dwgGenerator.Reset();
	ads_printf("\rCalculating Cross Section values..");
	for(i = 0; i < m_XSNDataRecs.GetSize(); i++)
	{
		PROFILEDATA* pPrfRecMid;

		pRec = m_XSNDataRecs[i];
		ads_printf("\rProccessing Cross Section #%d of %d", (i+1), m_XSNDataRecs.GetSize());

		//Find the Centre point
		pPrfRecMid = 0L;
		for(j = 0; j < pRec->RecArray.GetSize(); j++)
		{
			if(GetBitFlag(pRec->RecArray[j]->lBit, 3)) 
			{
				pPrfRecMid = pRec->RecArray[j];
				break;
			}
		}
		if(pPrfRecMid == 0L)
		{//Centre point not found
			strMsg.Format("Centre point not found for cross section data set #%d\nWould you like to continue ?", (i + 1));
			if(AfxMessageBox(strMsg, MB_ICONQUESTION|MB_YESNO) != IDYES)
			{
				return FALSE;
			}
		}
		else
		{//Write data....
			PROFILEDATA* pPrfRec;
			POINT3D ptCentre, ptTmp;
			double dDist, dAngle;
			BOOL bWriteThisDataSet;

			bWriteThisDataSet = TRUE;
			//Find Chainage
			if(!_FindChainage(pPrfRecMid->Mark, dDist, dAngle, dMF))
			{//failed to calculate chainage value
				strMsg.Format("Failed to calculate chainage value in cross section data set #%d\nWould you like to continue ?", (i + 1));
				if(AfxMessageBox(strMsg, MB_ICONQUESTION|MB_YESNO) != IDYES)
				{
					CSurvUtilApp::HideProgressDlg();
					return FALSE;
				}
				bWriteThisDataSet = FALSE;
			}
			if(m_bAutoChainageCalnInXSN)
			{
				pRec->dChainage = dDist;
			}
			if(bWriteThisDataSet)
			{
				CXSecFrXLS objXSecFrXLS;
				XSNOUT_STRUCT RecXSNout;
				//Write start of station...
				//[Bit Flag Start] [Chainage]

				objXSecFrXLS.SetChainage(pRec->dChainage);
				//ads_printf("\n\n\nDEBUG: PID Angle = %f\n", RTOD(dAngle));

				if(!_GetPoint3DFromMaster(pPrfRecMid->Mark, ptCentre, dMF))
				{
					ads_printf("\n\n\nERROR: Internal error occured while proccessing C/S data #%d", (i + 1));
					return FALSE;
				}
				for(j = 0; j < pRec->RecArray.GetSize(); j++)
				{
					pPrfRec = pRec->RecArray[j];
					if(pPrfRec != pPrfRecMid)
					{
						double dAngleInRad;
						BOOL bIsPtOnLeft;
						VERIFY(_GetPoint3DFromMaster(pPrfRec->Mark, ptTmp, dMF));
						dDist = _Distance2D(ptCentre, ptTmp);
				
						dAngleInRad = _Angle(ptCentre, ptTmp);
						//ads_printf("\n\n\nCS = %f\n", RTOD(dAngleInRad));

						//Determine in which side the point residing
						bIsPtOnLeft = ((PI * 2.0 - fabs(dAngle - dAngleInRad)) < PI);
						if(bIsPtOnLeft)
							dDist = 0.0 - dDist;
						
						RecXSNout.lBit = pPrfRec->lBit;
						RecXSNout.dOffset = dDist;
						RecXSNout.dElev = ptTmp.z;
						RecXSNout.strTxt = pPrfRec->szText;
						RecXSNout.strBlk = pPrfRec->szBlock;
						RecXSNout.dNorth = ptTmp.y;
						RecXSNout.dEast = ptTmp.x;

						TmpArray.Add(RecXSNout);
					}
					else
					{
						RecXSNout.lBit = pPrfRec->lBit;
						RecXSNout.dOffset = 0.0;
						RecXSNout.dElev = ptCentre.z;
						RecXSNout.strTxt = pPrfRec->szText;
						RecXSNout.strBlk = pPrfRec->szBlock;
						RecXSNout.dNorth = ptCentre.y;
						RecXSNout.dEast = ptCentre.x;

						TmpArray.Add(RecXSNout);
					}
				}
				CSurveyData::SortXSNOutRecASC(TmpArray.GetData(), TmpArray.GetSize());
				for(j = 0; j < TmpArray.GetSize(); j++)
				{
					NEZXSECORDINATE recNew;
					RecXSNout = TmpArray[j];

					recNew.lBit		= RecXSNout.lBit;
					recNew.dOffset	= RecXSNout.dOffset;
					recNew.dElev	= RecXSNout.dElev;
					recNew.dNorth	= RecXSNout.dNorth;
					recNew.dEast	= RecXSNout.dEast;
					recNew.strTxt	= RecXSNout.strTxt;
					recNew.strBlk	= RecXSNout.strBlk;

					objXSecFrXLS.AddOrdinate(recNew);
				}
				TmpArray.RemoveAll();
				//
				dwgGenerator.Add(objXSecFrXLS);
			}
		}
	}
	ads_printf("\n\nAll Cross Section Proccessed Successfully!");
	return TRUE;
}
void CSurveyData::SortXSNOutRecASC(XSNOUT_STRUCT* pArray, const int& iSize)
{
	qsort((void*)pArray, iSize, sizeof(XSNOUT_STRUCT), CSurveyData::CmpXSNOutRecASC);
}
int CSurveyData::CmpXSNOutRecASC(const void* pV1, const void* pV2)
{
	const XSNOUT_STRUCT* pRec01 = (XSNOUT_STRUCT*)pV1;
	const XSNOUT_STRUCT* pRec02 = (XSNOUT_STRUCT*)pV2;

	if(pRec01->dOffset > pRec02->dOffset)
		return 1;
	if(pRec01->dOffset == pRec02->dOffset)
		return 0;
	if(pRec01->dOffset < pRec02->dOffset)
		return -1;
	return 0;
}

BOOL CSurveyData::_FindChainage(const MARK& Mark, double& dChainage, double& dAngleInRad, const double& dMF /*= 1.0*/) const
{
	int i;
	POINT3D ptBase;
	CSurveyPoints PIPoints;

	dChainage = dAngleInRad = 0.0;
	if(!_GetPoint3DFromMaster(Mark, ptBase, dMF))
		return FALSE;
	if(!CreateSPtArrayFromMarkArray(m_PIDataRecs, PIPoints))
		return FALSE;
	
	for(i = 1; i < PIPoints.GetSize(); i++)
	{
		POINT3D pt3DStart, pt3DEnd;

		_ConvertSurveyPtToPoint3D(PIPoints[i - 1], pt3DStart);
		_ConvertSurveyPtToPoint3D(PIPoints[i - 0], pt3DEnd);

		if(_IsPtOnLine2D(ptBase, pt3DStart, pt3DEnd))
		{
			dChainage += _Distance2D(pt3DStart, ptBase);
			dAngleInRad = _Angle(pt3DStart, pt3DEnd);
			DeallocatePtrArray(PIPoints);
			return TRUE;
		}
		else
		{
			dChainage += _Distance2D(pt3DStart, pt3DEnd);
		}
	}
	DeallocatePtrArray(PIPoints);
	return FALSE;
}
//Read/Write...from a BIN file
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSurveyData::WriteData(const CString& strFile) const
{
	CFile FileOut;
	CFileException e;
	
	if(!FileOut.Open(strFile, CFile::modeCreate|CFile::modeWrite, &e))
	{
		ads_printf("\nFile could not be opened (ERROR = %d) ", e.m_cause);
		return FALSE;
	}
	
	//File Header...
	FileOut.Write(&(CSurveyData::m_OutFileHeader), sizeof(CSurveyData::m_OutFileHeader));
	_WriteMasterData(FileOut);
	_WriteMarkArray(FileOut, m_PIDataRecs);
	_WriteMarkArray(FileOut, m_PINDataRecs);
	_WritePRData(FileOut, m_PRDataRecs);
	_WriteRXSecData(FileOut);
	_WriteXSecData(FileOut);
	return TRUE;
}
void CSurveyData::_WriteMasterData(CFile& FileOut) const
{
	int i, iLim;
	MASTERDATA* pRec;

	iLim = m_MasterDataRecs.GetSize();
	FileOut.Write(&iLim, sizeof(iLim));//Size...
	for(i = 0; i < m_MasterDataRecs.GetSize(); i++)
	{
		pRec = m_MasterDataRecs[i];
		FileOut.Write(pRec, sizeof(MASTERDATA));
	}
}
void CSurveyData::_WriteMarkArray(CFile& FileOut, const CMarkRecs& MarkArray) const
{
	int i, iLim;
	MARK* pRec;

	iLim = MarkArray.GetSize();
	FileOut.Write(&iLim, sizeof(iLim));//Size...
	for(i = 0; i < MarkArray.GetSize(); i++)
	{
		pRec = MarkArray[i];
		FileOut.Write(pRec, sizeof(MARK));
	}
}
void CSurveyData::_WritePRData(CFile& FileOut, const CPRDataRecs& PRDataRecs) const
{
	int i, iLim;
	PROFILEDATA* pRec;

	iLim = PRDataRecs.GetSize();
	FileOut.Write(&iLim, sizeof(iLim));//Size...
	for(i = 0; i < PRDataRecs.GetSize(); i++)
	{
		pRec = PRDataRecs[i];
		FileOut.Write(pRec, sizeof(PROFILEDATA));
	}
}
void CSurveyData::_WriteRXSecData(CFile& FileOut) const
{
	int i, iLim;
	ROADXSECDATA* pRec;

	iLim = m_RXSecDataRecs.GetSize();
	FileOut.Write(&iLim, sizeof(iLim));//Size...
	for(i = 0; i < m_RXSecDataRecs.GetSize(); i++)
	{
		pRec = m_RXSecDataRecs[i];
		FileOut.Write(&(pRec->lID), sizeof(pRec->lID));
		_WriteMarkArray(FileOut, pRec->MarkArray);
	}
}
void CSurveyData::_WriteXSecData(CFile& FileOut) const
{
	int i, iLim;
	XSECDATA* pRec;

	FileOut.Write(&m_bAutoChainageCalnInXSN, sizeof(m_bAutoChainageCalnInXSN));//Calculation Flag
	iLim = m_XSNDataRecs.GetSize();
	FileOut.Write(&iLim, sizeof(iLim));//Size...

	for(i = 0; i < m_XSNDataRecs.GetSize(); i++)
	{
		pRec = m_XSNDataRecs[i];
		FileOut.Write(&(pRec->dChainage), sizeof(pRec->dChainage));
		_WritePRData(FileOut, pRec->RecArray);
	}
}
BOOL CSurveyData::ReadData(const CString& strFile)
{
	CFile FileOut;
	CFileException e;
	OUTFILE_HEADER HeaderRec;
	if(!FileOut.Open(strFile, CFile::modeRead, &e))
	{
		TRACE("\nFile could not be opened (ERROR = %d) ", e.m_cause);
		return FALSE;
	}
	Reset();//Reset Member Vars..
	
	//File Header...
	FileOut.Read(&HeaderRec, sizeof(HeaderRec));

	_ReadMasterData(FileOut);
	_ReadMarkArray(FileOut, m_PIDataRecs);
	_ReadMarkArray(FileOut, m_PINDataRecs);
	_ReadPRData(FileOut, m_PRDataRecs);
	
	_ReadRXSecData(FileOut);
	_ReadXSecData(FileOut);

	return TRUE;
}
void CSurveyData::_ReadMasterData(CFile& File)
{
	int i, iLim;

	File.Read(&iLim, sizeof(iLim));//Size.... 
	VERIFY(0 == m_MasterDataRecs.GetSize());
	for(i = 0; i < iLim; i++)
	{
		MASTERDATA* pRec;
		
		pRec = new MASTERDATA;
		File.Read(pRec, sizeof(MASTERDATA));
		m_MasterDataRecs.Add(pRec);
	}
}
void CSurveyData::_ReadMarkArray(CFile& File, CMarkRecs& RecArray) const
{
	int i, iLim;

	File.Read(&iLim, sizeof(iLim));//Size.... 
	VERIFY(0 == RecArray.GetSize());
	for(i = 0; i < iLim; i++)
	{
		MARK* pRec;
		
		pRec = new MARK;
		File.Read(pRec, sizeof(MARK));
		RecArray.Add(pRec);
	}
}
void CSurveyData::_ReadRXSecData(CFile& File)
{
	int i, iLim;

	File.Read(&iLim, sizeof(iLim));//Size.... 
	VERIFY(0 == m_RXSecDataRecs.GetSize());
	for(i = 0; i < iLim; i++)
	{
		ROADXSECDATA* pRec;
		
		pRec = new ROADXSECDATA;
		
		File.Read(&(pRec->lID), sizeof(pRec->lID));
		_ReadMarkArray(File, pRec->MarkArray);

		m_RXSecDataRecs.Add(pRec);
	}
}
void CSurveyData::_ReadXSecData(CFile& File)
{
	int i, iLim;

	File.Read(&m_bAutoChainageCalnInXSN, sizeof(m_bAutoChainageCalnInXSN));//Calculation Flag
	File.Read(&iLim, sizeof(iLim));//Size.... 
	VERIFY(0 == m_XSNDataRecs.GetSize());
	for(i = 0; i < iLim; i++)
	{
		XSECDATA* pRec;
		
		pRec = new XSECDATA;
		
		File.Read(&(pRec->dChainage), sizeof(pRec->dChainage));
		_ReadPRData(File, pRec->RecArray);

		m_XSNDataRecs.Add(pRec);
	}
}
void CSurveyData::_ReadPRData(CFile& File, CPRDataRecs& RecArray) const
{
	int i, iLim;

	File.Read(&iLim, sizeof(iLim));//Size.... 
	VERIFY(0 == RecArray.GetSize());
	for(i = 0; i < iLim; i++)
	{
		PROFILEDATA* pRec;
		
		pRec = new PROFILEDATA;
		
		File.Read(pRec, sizeof(PROFILEDATA));
		RecArray.Add(pRec);
	}
}
