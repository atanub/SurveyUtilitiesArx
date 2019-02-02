// XSecFrCSDEx.cpp : implementation file
//
#include "stdafx.h"
#include "SurvUtilApp.h"
#include "Util.h"
#include "DrawUtil.h"
#include "XSecFrCSDEx.h"
#include "XData.h"
#include "ResultDlg.h"
#include "DrawCSFrCSDExDlg.h"
#include "ParameterDataCache.h"

void XSecFrCSDEx()
{
	CStringArray strArrayErrs;
	CSDDWGPARAMEX Rec;
	AcGePoint3d ptSheetULC;
	CStdioFile objLogFile;
	CStdioFile* pObjLogFile;
	
	ptSheetULC.x = 10;
	ptSheetULC.y = 20;
	ptSheetULC.z = 0;

	Rec = *(CSurvUtilApp::GetDataCache()->GetCSDDwgParamEx());
	if(!::GetCSDExDwgParam(Rec))
	{
		return;
	}
	CSurvUtilApp::GetDataCache()->SetCSDDwgParamEx(&Rec);
	if(RTNORM != ads_getpoint(0L, "\nPick upper left corner of drawing sheet :", asDblArray(ptSheetULC)))
	{
		ads_printf("\nERROR: Invalid upper left corner of drawing sheet picked");
		return;
	}

	CXSecDwgGenerator objXSecDwgGenerator(Rec);

	pObjLogFile = 0L;
	if(strlen(Rec.szDataFileOut) > 0)
	{
		if(!objLogFile.Open(Rec.szDataFileOut, CFile::modeCreate|CFile::modeWrite|CFile::typeText))
		{
			CString strTmp;
			
			strTmp.Format("WARNING: Unable to open calculation log file \"%s\"", Rec.szDataFileOut);
			ads_printf("\n%s", (LPCSTR)strTmp);
			
			strArrayErrs.Add(strTmp);
			pObjLogFile = 0L;
		}
		else
		{
			pObjLogFile = &objLogFile;
	//#ifndef _DEBUG //DISABLED in Release Version
			//pObjLogFile->Close();
			//pObjLogFile = 0L;
	//#endif//_DEBUG
		}
	}
	if(!objXSecDwgGenerator.Draw(ptSheetULC, pObjLogFile, &strArrayErrs))
	{
		ads_printf("\n\nERROR: Not all the Cross Sections drawn successfully\n");
	}
	if(!objXSecDwgGenerator.CalcVolume(pObjLogFile, &strArrayErrs))
	{
		ads_printf("\n\nERROR: Failed to calculate volume\n");
	}
	//
	if(pObjLogFile != 0L)
	{
		pObjLogFile->Close();
	}
	if(strArrayErrs.GetSize() > 0)
	{
		::ShowResultDlg(strArrayErrs);
	}
}


// class -> CXSecDwgGenerator **************************************************************************************************
CXSecDwgGenerator::CXSecDwgGenerator(const CSDDWGPARAMEX& Rec) : m_DwgAttribs(Rec)
{
}
CXSecDwgGenerator::~CXSecDwgGenerator()
{
	DeallocatePtrArray(m_ArrayOrgData);
	DeallocatePtrArray(m_ArrayDsgnData);
}
BOOL CXSecDwgGenerator::CalcVolume(CStdioFile* pObjLogFile/* = 0L*/, CStringArray* pStrArrayErrs/* = 0L*/)
{
	int i;
	AcGeDoubleArray	arrayCutOrFillVals, arrayChgVals;
	CString strLine;

	for(i = 0; i < m_ArrayOrgData.GetSize(); i++)
	{
		CXSecStn* pRec;
		double fFieldDataVol, fDsgnVol;

		if(ads_usrbrk() == 1)
		{
			if(pStrArrayErrs != 0L)
			{
				strLine.Format("User break encountered\r\nTotal %d# of section(s) processed\n", i);
				pStrArrayErrs->Add(strLine);
			}
			else
			{
				ads_printf("\n\nUser break encountered\r\nTotal %d# of section(s) processed\n", i);
			}
			return TRUE;//User break 
		}
		pRec = m_ArrayOrgData[i];
		if(!pRec->CalcCutOrFillValue(m_ArrayDsgnData, fFieldDataVol, fDsgnVol))
		{
			ads_printf("\nERROR: Failed to Calculate Volume at Chainage : %f", pRec->GetChainage());
			return FALSE;//ERROR
		}
		if(pObjLogFile != 0L)
		{//
			if(i == 0)
			{//Heading....
				strLine.Format("****|****|****|****|****|****|->SurvUtilARX on %s|<-|****|****|****|****|****|****\n\n", COleDateTime::GetCurrentTime().Format());
				pObjLogFile->WriteString(strLine);
				strLine.Format("Detail of Cut/Fill order by chainage........\n");
				pObjLogFile->WriteString(strLine);
			}
			{//Data...
				strLine.Format("At Chainage %.3f Volume = %.3f[%s] [[Existing = %.3f;;Proposed = %.3f]]\n", pRec->GetChainage(), fabs(fFieldDataVol - fDsgnVol), ((fFieldDataVol - fDsgnVol) > 0.0) ? "Cut":"Fill", fFieldDataVol, fDsgnVol);
				pObjLogFile->WriteString(strLine);
			}
		}
		arrayCutOrFillVals.append(fFieldDataVol - fDsgnVol);
		arrayChgVals.append(pRec->GetChainage());
	}
	ads_printf("\r\nTotal %d # of Cross Sections processed", i);
	{//Calculate Total volume of Cut/Fill.............
		double fVolPrev, fVolCur, fDistPrev, fDistCur, fVolume, fDist;
		
		for(i = 1, fVolume = 0.0, fDist = 0.0; i < arrayCutOrFillVals.length(); i++)
		{
			fVolPrev = arrayCutOrFillVals[i - 1];
			fVolCur = arrayCutOrFillVals[i - 0];
			
			fDistPrev = arrayChgVals[i - 1];
			fDistCur = arrayChgVals[i - 0];
			
			fVolume += (0.5 * (fDistCur - fDistPrev) * (fVolPrev + fVolCur));
			fDist += (fDistCur - fDistPrev);
		}
		if(pObjLogFile != 0L)
		{//
			strLine.Format("****|****|****|****|****|****|->SurvUtilARX on %s|<-|****|****|****|****|****|****\n\n", COleDateTime::GetCurrentTime().Format());
			pObjLogFile->WriteString(strLine);
			strLine.Format("Total Volume			: %.3f[%s]\n", fabs(fVolume), (fVolume > 0.0) ? "Cut":"Fill");
			pObjLogFile->WriteString(strLine);
			strLine.Format("Total Distance Surveyed : %.3f\n", fDist);
			pObjLogFile->WriteString(strLine);
			strLine.Format("****|****|****|****|****|****|->SurvUtilARX on %s|<-|****|****|****|****|****|****\n\n", COleDateTime::GetCurrentTime().Format());
		}
	}
	return TRUE;//
}
BOOL CXSecDwgGenerator::Draw(const AcGePoint3d& ptSheetULCParam, CStdioFile* pObjLogFile/* = 0L*/, CStringArray* pStrArrayErrs/* = 0L*/)
{
	const double fSheetToSheetDist = m_DwgAttribs.dSheetWid * 10.0 / 100.0;
	BOOL bErrFlag, bIsUserBreak;
	int i, j;
	AcGePoint3d ptSheetULC;

	
	bErrFlag = _ReadDesignData(pStrArrayErrs);
	bErrFlag = bErrFlag && _ReadFieldData(pStrArrayErrs);
	
	if(!bErrFlag)
	{
		return bErrFlag;
	}
	//Prepare Drawing Settings
	LAYER(m_DwgAttribs.szLyrAnnotation, 1);
	LAYER(m_DwgAttribs.szLyrTopSurface, 2);
	LAYER(m_DwgAttribs.szLyrOrdinate, 3);
	LAYER(m_DwgAttribs.szLyrAxis, 4);
	
	LAYER(m_DwgAttribs.szLyrAnnotationDsgn, 5);
	LAYER(m_DwgAttribs.szLyrTopSurfaceDsgn, 6);
	LAYER(m_DwgAttribs.szLyrOrdinateDsgn, 7);

	LAYER(m_DwgAttribs.szDwgSheet, 8);

	ptSheetULC = ptSheetULCParam;
	CXSecStn::RectangleUL(ptSheetULC, m_DwgAttribs.dSheetWid, m_DwgAttribs.dSheetHeight, m_DwgAttribs.szDwgSheet);
	
	double fCumHt, fRowHeight, fCumWid;
	bIsUserBreak = FALSE;
	for(i = 0, fCumHt = 0.0; i < m_ArrayOrgData.GetSize();)
	{
		int iEndIndex;
		
		if(!_GetNextSectionRange(fCumHt, i, iEndIndex, fRowHeight, bIsUserBreak, pStrArrayErrs))
		{
			return TRUE;//End 
		}
		if(bIsUserBreak)
		{
			CString strTmp;

			if(pStrArrayErrs != 0L)
			{
				strTmp.Format("User break encountered\r\nTotal %d# of section(s) drawn\n", i);
				pStrArrayErrs->Add(strTmp);
			}
			else
			{
				ads_printf("\n\nUser break encountered\r\nTotal %d# of section(s) drawn\n", i);
			}
			return TRUE;//User break 
		}
		if(fRowHeight > m_DwgAttribs.dSheetHeight)
		{
			ads_printf("\nERROR: Failed to draw C/S\nPossible cause: Sheet height is too small\nPlease Check C/S data from Chainage %.3f to %.3f", (m_ArrayOrgData[i])->GetChainage(), (m_ArrayOrgData[iEndIndex])->GetChainage());
			return FALSE;//End 
		}
		//
		if((fCumHt + fRowHeight) > m_DwgAttribs.dSheetHeight)
		{
			ptSheetULC.x += m_DwgAttribs.dSheetWid + fSheetToSheetDist;
			ptSheetULC.y = ptSheetULCParam.y;
			fCumHt = 0.0;
			CXSecStn::RectangleUL(ptSheetULC, m_DwgAttribs.dSheetWid, m_DwgAttribs.dSheetHeight, m_DwgAttribs.szDwgSheet);
		}

		//Draw...
		for(j = i, fCumWid = 0.0; j <= iEndIndex; j++)
		{
			bIsUserBreak = (ads_usrbrk() == 1);
			if(bIsUserBreak)
			{
				CString strTmp;

				if(pStrArrayErrs != 0L)
				{
					strTmp.Format("User break encountered\r\nTotal %d# of section(s) drawn\n", i);
					pStrArrayErrs->Add(strTmp);
				}
				else
				{
					ads_printf("\n\nUser break encountered\r\nTotal %d# of section(s) drawn\n", i);
				}
				return TRUE;//User break 
			}

			CXSecStn* pRec;
			AcGePoint3d ptLLCOfCS;
			
			ptLLCOfCS.z = 0.0;
			ptLLCOfCS.x = ptSheetULC.x + fCumWid;
			ptLLCOfCS.y = ptSheetULC.y - (fCumHt + fRowHeight);

			pRec = m_ArrayOrgData[j];
			
			//{//DEBUG
			//	char szTest[132];
			//	CString strTemp;
			//
			//	strTemp.Format("\nDEBUG: about to draw C/S at Chainage : %f", pRec->GetChainage());
			//	ads_getstring(0, strTemp, szTest);
			//}

			if(!pRec->Draw(ptLLCOfCS, m_DwgAttribs, m_ArrayDsgnData, pObjLogFile))
			{
				ads_printf("\nERROR: Failed to draw C/S at Chainage : %f", pRec->GetChainage());
			}
			else
			{
				AXISDETAIL recAxisDet;
				double fWid, fHt;

				if(!pRec->GetExtents(m_DwgAttribs, m_ArrayDsgnData, fWid, fHt, &recAxisDet))
				{
					ads_printf("\nFATAL ERROR: Control should not reach here [CXSecDwgGenerator::Draw]!!");
					return FALSE;
				}
				else
				{
					fCumWid += fWid;
				}
				ads_printf("\rCross Section #%d at chainage %.3f drawn successfully!", (i+1), pRec->GetChainage());
			}
		}
		
		//
		fCumHt += fRowHeight;
		i = iEndIndex + 1;
	}
	ads_printf("\r\nTotal %d # of Cross Sections drawn", i);
	return bErrFlag;
}
BOOL CXSecDwgGenerator::_GetMaxCSBoundRectHt(const int& iStartIndex, int& iEndIndex, double& fMaxHeight)
{
	int i;
	CXSecStn *pRec;
	double fWid, fHt;
	BOOL bFlag;
	AXISDETAIL recAxisDet;
	
	for(i = iStartIndex, bFlag = FALSE; i <= iEndIndex; i++)
	{
		pRec = m_ArrayOrgData[i];
		if(pRec->GetExtents(m_DwgAttribs, m_ArrayDsgnData, fWid, fHt, &recAxisDet))
		{
			bFlag = TRUE;
			if(i == iStartIndex)
			{
				fMaxHeight = fHt;
			}
			else
			{
				if(fHt > fMaxHeight)
					fMaxHeight = fHt;
			}
		}
	}
	return bFlag;
}
BOOL CXSecDwgGenerator::_GetNextSectionRange(const double& /*fCurRowTopDepth*/, const int& iStartIndex, int& iEndIndex, double& fMaxHeight, BOOL& bIsUserBreak, CStringArray* pStrArrayErrs/* = 0L*/)
{
	int i;
	double fWid, fHt, fCumWid;
	CXSecStn *pRec;
	AXISDETAIL recAxisDet;

	iEndIndex = iStartIndex;
	fMaxHeight = 0.0;
	for(i = iStartIndex, fCumWid = 0.0; i < m_ArrayOrgData.GetSize(); i++)
	{
		bIsUserBreak = (ads_usrbrk() == 1);
		if(bIsUserBreak)
		{
			return TRUE;
		}

		pRec = m_ArrayOrgData[i];
		if(!pRec->GetExtents(m_DwgAttribs, m_ArrayDsgnData, fWid, fHt, &recAxisDet))
		{
			ads_printf("\nWarning: Failed to draw C/S at Chainage : %f", pRec->GetChainage());
		}
		else
		{
			if((fWid > m_DwgAttribs.dSheetWid) || (fHt > m_DwgAttribs.dSheetHeight))
			{//C/S Extent too big to fit in Sheet
				CString strTmp;

				if(pStrArrayErrs != 0L)
				{
					strTmp.Format("ERROR: Cross Section at Section %.3f can't be fit into the drawing sheet", pRec->GetChainage());
					pStrArrayErrs->Add(strTmp);
				}
				else
				{
					ads_printf("\n\nERROR: Cross Section at Section %.3f can't be fit into the drawing sheet\n", pRec->GetChainage());
				}
				return FALSE;//ERROR 
			}

			if((fCumWid + fWid) >= m_DwgAttribs.dSheetWid)
			{
				if(!_GetMaxCSBoundRectHt(iStartIndex, iEndIndex, fMaxHeight))
				{
					return FALSE;
				}
				return TRUE;
			}
			fCumWid += fWid;
			iEndIndex = i;
		}
	}
	if(!_GetMaxCSBoundRectHt(iStartIndex, iEndIndex, fMaxHeight))
	{
		return FALSE;
	}
	return TRUE;
}
//Read from input file (ASCII Text)
BOOL CXSecDwgGenerator::_ReadFieldData(CStringArray* pStrArrayErrs /* = 0L*/)
{
	DeallocatePtrArray(m_ArrayOrgData);
	{//Read Original Field Data
		CString strTmp;
		CStdioFile	FileInp;
		BOOL bFlag, bIsEOFReached;
		long iLineNo;

		if(!FileInp.Open(m_DwgAttribs.szDataFile, CFile::modeRead|CFile::typeText))
		{
			if(pStrArrayErrs != 0L)
			{
				strTmp.Format("ERROR: Unable to open input file %s", m_DwgAttribs.szDataFile);
				pStrArrayErrs->Add(strTmp);
			}
			else
			{
				ads_printf("\nERROR: Unable to open input file %s", m_DwgAttribs.szDataFile);
			}
			return FALSE;
		}
		ads_printf("\rPlease wait... reading data file");
		for(iLineNo = 0;;)
		{
			CXSecStn* pNewStnData;
			BOOL bCanAdd;
			
			bCanAdd = TRUE;
			pNewStnData = new CXSecStn();
			ads_printf("\rPlease wait... reading line no. %d of file %s", iLineNo, m_DwgAttribs.szDataFile);
			bFlag = pNewStnData->Read(FileInp, bIsEOFReached, iLineNo);
			if(!bFlag || bIsEOFReached)
			{//Some Errors occured..Or EOF reached 
				delete pNewStnData;
				if(bIsEOFReached)
					return TRUE;

				if(pStrArrayErrs != 0L)
				{
					strTmp.Format("ERROR: Invalid data found at line# %d in file %s", iLineNo, m_DwgAttribs.szDataFile);
					pStrArrayErrs->Add(strTmp);
				}
				return FALSE;
			}
			pNewStnData->DumpVars();
			//Check for duplicate station
			if(_IsFieldDataXists(pNewStnData, m_ArrayOrgData))
			{
				bCanAdd = FALSE;
				strTmp.Format("ERROR: Duplicate station data found at CHG: %f, line# %d in file %s", pNewStnData->GetChainage(), iLineNo, m_DwgAttribs.szDataFile);
				if(pStrArrayErrs != 0L)
				{
					pStrArrayErrs->Add(strTmp);
				}
				else
				{
					ads_printf("\n%s", strTmp);
				}
				delete pNewStnData;
			}
			{//Check whether sequence is correct or not
				if((m_ArrayOrgData.GetSize() > 0) && bCanAdd)
				{
					if((m_ArrayOrgData[m_ArrayOrgData.GetSize() - 1])->GetChainage() >= pNewStnData->GetChainage())
					{
						strTmp.Format("ERROR: Station data sequence is not proper for CHG: %f, at line# %d in file %s", pNewStnData->GetChainage(), iLineNo, m_DwgAttribs.szDataFile);
						if(pStrArrayErrs != 0L)
						{
							pStrArrayErrs->Add(strTmp);
						}
						else
						{
							ads_printf("\n%s", strTmp);
						}
						delete pNewStnData;
						bCanAdd = FALSE;
					}
				}
			}
			if(bCanAdd)
			{
				m_ArrayOrgData.Add(pNewStnData);
			}
			if(ads_usrbrk() == 1)
			{
				if(pStrArrayErrs != 0L)
				{
					strTmp.Format("User break encountered\r\nTotal %d# of line(s) read in file %s", iLineNo, m_DwgAttribs.szDataFile);
					pStrArrayErrs->Add(strTmp);
				}
				else
				{
					ads_printf("\nUser break encountered\nTotal %d# of line(s) read in file %s\n", iLineNo, m_DwgAttribs.szDataFile);
				}
				return FALSE;
			}
		}
	}
	_SortFieldData(m_ArrayOrgData);
}
BOOL CXSecDwgGenerator::_IsFieldDataXists(const CXSecStn* pStnData, const CXSecStnArray& arrayParam)
{
	int i;
	for(i = 0; i < arrayParam.GetSize(); i++)
	{
		if((arrayParam[i])->GetChainage() == pStnData->GetChainage())
			return TRUE;
	}
	return FALSE;
}
BOOL CXSecDwgGenerator::_SortFieldData(CXSecStnArray& /*arrayParam*/)
{//Yet to be implemented!!
	return TRUE;
}
BOOL CXSecDwgGenerator::_ReadDesignData(CStringArray* pStrArrayErrs /* = 0L*/)
{
	DeallocatePtrArray(m_ArrayDsgnData);
	
	{//Read Design Field Data
		CString strTmp;
		CStdioFile	FileInp;
		BOOL bFlag, bIsEOFReached;
		long iLineNo;

		if(!FileInp.Open(m_DwgAttribs.szDataFileDsgn, CFile::modeRead|CFile::typeText))
		{
			if(pStrArrayErrs != 0L)
			{
				strTmp.Format("ERROR: Unable to open input file %s", m_DwgAttribs.szDataFileDsgn);
				pStrArrayErrs->Add(strTmp);
			}
			else
			{
				ads_printf("\nERROR: Unable to open input file %s", m_DwgAttribs.szDataFileDsgn);
			}
			return FALSE;
		}
		ads_printf("\rPlease wait... reading design data [file %s]", m_DwgAttribs.szDataFileDsgn);
		for(iLineNo = 0;;)
		{
			CXSecDsgnStn* pNewDsgnData;
			
			pNewDsgnData = new CXSecDsgnStn();
			ads_printf("\rPlease wait... reading line no. %d in file %s", iLineNo, m_DwgAttribs.szDataFileDsgn);
			bFlag = pNewDsgnData->Read(FileInp, bIsEOFReached, iLineNo, pStrArrayErrs);
			if(!bFlag || bIsEOFReached)
			{//Some Errors occured..Or EOF reached 
				delete pNewDsgnData;
				if(bIsEOFReached)
					return TRUE;

				if(pStrArrayErrs != 0L)
				{
					strTmp.Format("ERROR: Invalid data found at line# %d in file %s", iLineNo, m_DwgAttribs.szDataFileDsgn);
					pStrArrayErrs->Add(strTmp);
				}
				return FALSE;
			}
			pNewDsgnData->DumpVars();
			m_ArrayDsgnData.Add(pNewDsgnData);
			if(ads_usrbrk() == 1)
			{
				if(pStrArrayErrs != 0L)
				{
					strTmp.Format("User break encountered\nTotal %d# of line(s) read in file %s", iLineNo, m_DwgAttribs.szDataFileDsgn);
					pStrArrayErrs->Add(strTmp);
				}
				else
				{
					ads_printf("\nUser break encountered\nTotal %d# of line(s) read in file %s\n", iLineNo, m_DwgAttribs.szDataFileDsgn);
				}
				return FALSE;
			}
		}
	}
}

// class -> CXSecDsgnStn **************************************************************************************************
CXSecDsgnStn::CXSecDsgnStn()
{
	m_fChgStart = 0.0;
	m_fChgEnd = 0.0;
	m_fRLOfCenPt = 0.0;
}
CXSecDsgnStn::~CXSecDsgnStn()
{
	if(m_RecArray.GetSize() > 0)
	{
		DeallocatePtrArray(m_RecArray);
	}
}
void CXSecDsgnStn::DumpVars()
{
#ifdef _DEBUG
	int i;

	ads_printf("\nFrom CXSecDsgnStn::DumpVars()->>");
	ads_printf("\nStart Chg = %f,  End Chg = %f,  RL of Cen Pt. = %f,  Long Slope. = %f [Total %d# of data]", m_fChgStart, m_fChgEnd, m_fRLOfCenPt, m_fLongSlope, m_RecArray.GetSize());
	for(i = 0; i < m_RecArray.GetSize(); i++)
	{
		CSDRECEX* pRec;

		pRec = m_RecArray.GetAt(i);

		ads_printf("\nBitCode = %d, dOffset = %f, Slope 1 In %f, dCalcElev = %f, strTxt = %s, strBlk = %s", pRec->lBit, pRec->dOffset, pRec->dSlope, pRec->dCalcElev, pRec->strTxt, pRec->strBlk);
	}
	ads_printf("\n\n\n");
#endif //_DEBUG
}
//NOTE: Make sure elevations calculated successfully prior to call this
double CXSecDsgnStn::GetMinElev()
{
	int i;
	double fMinElev;

	if(m_RecArray.GetSize() <= 0)
	{
		ads_printf("\nERROR: Control should not reach here [CXSecDsgnStn::CalcArea() Tag=A]\n");
		return -1.0;
	}

	fMinElev = (m_RecArray[0])->dCalcElev;
	for(i = 1; i < m_RecArray.GetSize(); i++)
	{
		if((m_RecArray[i])->dCalcElev < fMinElev)
			fMinElev = m_RecArray[i]->dCalcElev;
	}
	return fMinElev;
}
double CXSecDsgnStn::GetMinOffset()
{
	if(m_RecArray.GetSize() <= 0)
	{
		ads_printf("\nERROR: Control should not reach here! [CXSecDsgnStn::GetMinOffset()]\n\n");
		return 0.0;
	}
	return (m_RecArray[0])->dOffset;
}
double CXSecDsgnStn::GetMaxOffset()
{
	if(m_RecArray.GetSize() <= 0)
	{
		ads_printf("\nERROR: Control should not reach here! [CXSecDsgnStn::GetMaxOffset()]\n\n");
		return 0.0;
	}
	return (m_RecArray[m_RecArray.GetSize()-1])->dOffset;
}
BOOL CXSecDsgnStn::Read(CStdioFile& objFile, BOOL& bIsEOF, long& iLineNo, CStringArray* pStrArrayErrs /* = 0L*/)
{
	DeallocatePtrArray(m_RecArray);
	for(bIsEOF = FALSE; (bIsEOF == FALSE);)
	{
		CStringArray strWordArray;
		
		if(!ReadNextValidLine(objFile, strWordArray, iLineNo))
		{//EOF reached...
			if(m_RecArray.GetSize() != 0)
			{//Chk whether this data set is OK or not ....may be an error
				CSDRECEX* pRecTmp;
				
				pRecTmp = m_RecArray[m_RecArray.GetSize() - 1];
				if(pRecTmp->lBit != 10)
				{
					DeallocatePtrArray(m_RecArray);
					return FALSE;//ERROR
				}
				return TRUE;//ERROR
			}
			bIsEOF = TRUE;
			return TRUE;
		}
		{//
			int iBitCode;

			iBitCode = atoi(strWordArray[0]);
			if(iBitCode == 10)
			{//End of this Station Data
				goto FINAL;
			}
			if(iBitCode == 9)
			{//Start of this Station Data
				if(m_RecArray.GetSize() != 0)
				{
					DeallocatePtrArray(m_RecArray);//for Safety
					return FALSE;
				}
				if(!_ParseCSDDsgnStartLine(strWordArray, m_fChgStart, m_fChgEnd, m_fRLOfCenPt, m_fLongSlope))
				{
					DeallocatePtrArray(m_RecArray);
					return FALSE;
				}
			}
			else
			{//Collect lines Data 
				CSDRECEX* pNewRec;

				pNewRec = new CSDRECEX;
				if(!_ParseCSDLineInput(strWordArray, pNewRec))
				{
					delete pNewRec;
					DeallocatePtrArray(m_RecArray);
					return FALSE;//ERROR
				}
				if(pNewRec->dOffset == 0.0)
				{//Reserved for Internal Use....
					if(pStrArrayErrs != 0L)
					{
						CString strTmp;

						strTmp.Format("WARNING: Design Data at Line #%d (From CH:.3%f to CH:%.3f) against Offset 0.0 ignored, since it's reserved for internal use", iLineNo, m_fChgStart, m_fChgEnd);
						pStrArrayErrs->Add(strTmp);
					}
					else
					{
						ads_printf("\n\nWARNING: Design Data at Line #%d (From CH:.3%f to CH:%.3f) against Offset 0.0 ignored, since it's reserved for internal use", iLineNo, m_fChgStart, m_fChgEnd);
					}
					delete pNewRec;
				}
				else
				{
					m_RecArray.Add(pNewRec);
				}
			}
		}
	}
	goto FINAL;
	FINAL :
	{
		{//Add Internal data @ offset 0.0 
			CSDRECEX* pNewRec;

			pNewRec = new CSDRECEX;
			pNewRec->lBit		= 1L;
			pNewRec->dOffset	= 0.0;
			pNewRec->dSlope		= 0.0;
			pNewRec->dCalcElev  = 0.0;
			m_RecArray.Add(pNewRec);
		}
		_SortRecsByOffset();
	}
	return (m_RecArray.GetSize() > 0);
}

int CXSecDsgnStn::_GetMinOffsetDistIndex(CCSDataRecArrayEx& RecArray)
{
	int i, iIndex;
	
	if(RecArray.GetSize() < 1)
		return -1;//Error
	if(RecArray.GetSize() == 1)
		return 0;

	for(i = 0, iIndex = 0; i < RecArray.GetSize(); i++)
	{
		CSDRECEX* pRecMin;
		CSDRECEX* pRecCur;

		pRecCur = RecArray[i];
		pRecMin = RecArray[iIndex];
		
		if(pRecCur->dOffset < pRecMin->dOffset)
			iIndex = i;
	}
	return iIndex;
}
void CXSecDsgnStn::_SortRecsByOffset()
{
	int iIndex, iOrgLen;
	CCSDataRecArrayEx RecArray;
	
	iOrgLen = m_RecArray.GetSize();
	for(; m_RecArray.GetSize() > 0;)
	{
		iIndex = _GetMinOffsetDistIndex(m_RecArray);
		if(iIndex >= 0)
		{
			CSDRECEX* pRec;
			
			pRec = m_RecArray.GetAt(iIndex);
			m_RecArray.RemoveAt(iIndex);
			RecArray.Add(pRec);
		}
		else
		{
			ads_printf("\nERROR: Control should not reach here [CXSecDsgnStn::_SortRecsByOffset()Tag=A]\n");
		}
	}
	if(m_RecArray.GetSize() != 0)
	{
		ads_printf("\nERROR: Control should not reach here [CXSecDsgnStn::_SortRecsByOffset()Tag=B]\n");
		return;
	}
	m_RecArray.Append(RecArray);
	if(iOrgLen != m_RecArray.GetSize())
	{
		ads_printf("\nERROR: Control should not reach here [CXSecDsgnStn::_SortRecsByOffset()Tag=C]\n");
		return;
	}
}
//;******************************************************
//;*Bit Code Specification for each Line of Station data
//;*	0 = No Text/Block Annotation & Ordinate Line
//;*	1 = Only Draws Ordinate Line
//;*	2 = Only Draws Text Annotation
//;*	4 = Only Inserts the specified Blockname
//;******************************************************
BOOL CXSecDsgnStn::_ParseCSDLineInput(const CStringArray& strArray, CSDRECEX* pRecParam)
{
	if(strArray.GetSize() < 3)
		return FALSE;
	pRecParam->lBit = atoi(strArray[0]);
	//
	pRecParam->dOffset		= atof(strArray[1]);
	pRecParam->dSlope		= atof(strArray[2]);
	pRecParam->dCalcElev	= 0.0;

	//
	if((pRecParam->lBit & 4) && (pRecParam->lBit & 2))
	{
		if(strArray.GetSize() < 5)
		{
			return FALSE;//Text/Block or both not found
		}
		else
		{
			pRecParam->strTxt = strArray[3];
			pRecParam->strBlk = strArray[4];
		}
	}
	else
	{
		if((pRecParam->lBit & 2) || (pRecParam->lBit & 4))
		{
			if(strArray.GetSize() < 4)
				return FALSE;//Text/Block not found
			if(pRecParam->lBit & 2)
			{
				pRecParam->strTxt = strArray[3];
			}
			else
			{
				if(pRecParam->lBit & 4)
					pRecParam->strBlk = strArray[3];
			}

		}
	}
	return TRUE;
}
BOOL CXSecDsgnStn::_ParseCSDDsgnStartLine(const CStringArray& strArray, double& fStartChg, double& fEndChg, double& fRLOfCenPt, double& fLongSlope)
{
	long lBit;
	
	lBit = atoi(strArray[0]);
	if(lBit != 9)
		return FALSE;
	
	if(strArray.GetSize() < 5)
		return FALSE;//StartChg/EndChg or both not found
	
	fStartChg	= atof(strArray[1]);
	fEndChg		= atof(strArray[2]);
	fRLOfCenPt	= atof(strArray[3]);
	fLongSlope	= atof(strArray[4]);
	return TRUE;
}
BOOL CXSecDsgnStn::IsThisForChg(const double& fChg) const
{
	return ((fChg <= m_fChgEnd) && (fChg >= m_fChgStart));
}
BOOL CXSecDsgnStn::_CalcElevAtChg(const double& fChg, double& fElev) const
{
	fElev = 0.0;
	if(!IsThisForChg(fChg))
		return FALSE;

	fElev = m_fRLOfCenPt + (fChg - m_fChgStart) * 1.0 / m_fLongSlope;
	return TRUE;
}

double CXSecDsgnStn::CalcArea(const double& fElevMin, const double& fElevA, const double& fElevB, const double& fOffsetA, const double& fOffsetB)
{
	double fHtA, fHtB, fWid, fArea;

	if((fElevMin > fElevA) || (fElevMin > fElevB))
	{
		ads_printf("\nERROR: Control should not reach here [CXSecDsgnStn::CalcArea() Tag=A]\n");
		return -1.0;
	}
	fHtA = fElevA - fElevMin;
	fHtB = fElevB - fElevMin;
	fWid = fabs(fOffsetA - fOffsetB);

	fArea = fWid * (fHtA + fHtB) / 2.0;
	return fArea;
}

//
//This func assumes the Ordinate Elevations are already calculated
//
double CXSecDsgnStn::GetVolPerUnitLen(const double& fMinElev)
{
	int i;
	CSDRECEX *pRecCur;
	CSDRECEX *pRecPrev;
	double fValToRet;

	if(m_RecArray.GetSize() <= 1)
	{
		ads_printf("\nERROR: Volume Calculation failed, inadequate # of data found\n");
		return 0.0;
	}

	for(i = 1, fValToRet = 0.0; i < m_RecArray.GetSize(); i++)
	{
		pRecPrev	= m_RecArray[i - 1];
		pRecCur		= m_RecArray[i];

		fValToRet += CXSecDsgnStn::CalcArea(fMinElev, pRecPrev->dCalcElev, pRecCur->dCalcElev, pRecPrev->dOffset, pRecCur->dOffset);
	}
	return fValToRet;
}
void CXSecDsgnStn::CalcElevations(const double& fChg)
{
	int i;
	CCSDataRecArrayEx arrayLeftOrds, arrayRiteOrds;
	CSDRECEX* pRecCur;
	double fElevAtZeroOffset;

	if(!_CalcElevAtChg(fChg, fElevAtZeroOffset))
	{
		ads_printf("\nERROR: Control should not reach here [CXSecDsgnStn::CalcElevations() Tag=A]\n");
		return;
	}

	//Initialize Elevs with 0's
	for(i = 0; i < m_RecArray.GetSize(); i++)
	{
		pRecCur = m_RecArray[i];
		pRecCur->dCalcElev = 0.0;
	}
	for(i = 0; i < m_RecArray.GetSize(); i++)
	{
		pRecCur = m_RecArray[i];
		if(pRecCur->dOffset < 0.0)
		{//All left hand side ordinates 
			arrayLeftOrds.InsertAt(0, pRecCur);//Insert @ the begining
		}
		else if(pRecCur->dOffset > 0.0)
		{
			arrayRiteOrds.Add(pRecCur);//Insert @ the end
		}
		else if(pRecCur->dOffset == 0.0)
		{
			pRecCur->dCalcElev = fElevAtZeroOffset;
		}

	}
	//Calculate Elevations on Left Ordinates.....
	//ads_printf("\nLeft Ordinates are...");
	for(i = 0; i < arrayLeftOrds.GetSize(); i++)
	{
		pRecCur = arrayLeftOrds[i];
		if(i == 0)
		{
			if(pRecCur->dSlope == 0.0)
			{
				pRecCur->dCalcElev = fElevAtZeroOffset;
			}
			else
			{
				pRecCur->dCalcElev = fElevAtZeroOffset + (fabs(pRecCur->dOffset) * 1.0 / pRecCur->dSlope);
			}
		}
		else
		{
			CSDRECEX* pRecPrev;

			pRecPrev = arrayLeftOrds[i-1];
			if(pRecCur->dSlope == 0.0)
			{
				pRecCur->dCalcElev = pRecPrev->dCalcElev;
			}
			else
			{
				pRecCur->dCalcElev = pRecPrev->dCalcElev + ((fabs(pRecCur->dOffset) - fabs(pRecPrev->dOffset)) * 1.0 / pRecCur->dSlope);
			}
		}
		//ads_printf("\npRecCur->dOffset = %f, Calculated Elev = %f", pRecCur->dOffset, pRecCur->dCalcElev);
	}
	//Calculate Elevations on Rite Ordinates.....
	//ads_printf("\nRight Ordinates are...");
	for(i = 0; i < arrayRiteOrds.GetSize(); i++)
	{
		pRecCur = arrayRiteOrds[i];
		if(i == 0)
		{
			if(pRecCur->dSlope == 0.0)
			{
				pRecCur->dCalcElev = fElevAtZeroOffset;
			}
			else
			{
				pRecCur->dCalcElev = fElevAtZeroOffset + (pRecCur->dOffset * 1.0 / pRecCur->dSlope);
			}
		}
		else
		{
			CSDRECEX* pRecPrev;

			pRecPrev = arrayRiteOrds[i-1];
			if(pRecCur->dSlope == 0.0)
			{
				pRecCur->dCalcElev = pRecPrev->dCalcElev;
			}
			else
			{
				pRecCur->dCalcElev = pRecPrev->dCalcElev + ((pRecCur->dOffset - pRecPrev->dOffset) * 1.0 / pRecCur->dSlope);
			}
		}
		//ads_printf("\npRecCur->dOffset = %f, Calculated Elev = %f", pRecCur->dOffset, pRecCur->dCalcElev);
	}
}
BOOL CXSecDsgnStn::FindMinMaxOffElev(const double& fForChg, double& fMinOff, double& fMaxOff, double& fMaxElev)
{
	int i;

	if(m_RecArray.GetSize() <= 0)
		return FALSE;//Not Initialized Properly
	fMinOff = GetMinOffset();
	fMaxOff = GetMaxOffset();
	
	CalcElevations(fForChg);
	fMaxElev = (m_RecArray[0])->dCalcElev;
	for(i = 1; i < m_RecArray.GetSize(); i++)
	{
		if((m_RecArray[i])->dCalcElev > fMaxElev)
			fMaxElev = (m_RecArray[i])->dCalcElev;
	}
	return TRUE;
}


// class -> CXSecStn **************************************************************************************************
const double CXSecStn::m_fTxtWidMF = 1.0;

CXSecStn::CXSecStn()
{
	m_fChg = 0.0;
	m_fDatum = 0.0;
}
CXSecStn::~CXSecStn()
{
	if(m_RecArray.GetSize() > 0)
	{
		DeallocatePtrArray(m_RecArray);
	}
}
void CXSecStn::DumpVars()
{
	return;
#ifdef _DEBUG
	int i;

	ads_printf("\nFrom CXSecStn::DumpVars()->>");
	ads_printf("\nChg = %f,  Datum = %f [Total %d# of data]", m_fChg, m_fDatum, m_RecArray.GetSize());
	for(i = 0; i < m_RecArray.GetSize(); i++)
	{
		CSDREC* pRec;

		pRec = m_RecArray.GetAt(i);

		ads_printf("\nBitCode = %d, dOffset = %f, dElev = %f, strTxt = %s, strBlk = %s", pRec->lBit, pRec->dOffset, pRec->dElev, pRec->strTxt, pRec->strBlk);
	}
	ads_printf("\n\n\n");
#endif //_DEBUG
}
void CXSecStn::_SortByOffset(CCSDataRecArray& RecArrayParam)
{
	CCSDataRecArray RecArray;
	int iIndex, iOrgLen;
	
	iOrgLen = RecArrayParam.GetSize();
	for(; RecArrayParam.GetSize() > 0;)
	{
		iIndex = _GetMinOffsetDistIndex(RecArrayParam);
		if(iIndex >= 0)
		{
			CSDREC* pRec;
			
			pRec = RecArrayParam.GetAt(iIndex);
			RecArrayParam.RemoveAt(iIndex);
			RecArray.Add(pRec);
		}
		else
		{
			ads_printf("\nERROR: Control should not reach here [CXSecStn::_SortByOffset() Tag=A]\n");
		}
	}
	if(RecArrayParam.GetSize() != 0)
	{
		ads_printf("\nERROR: Control should not reach here [CXSecStn::_SortByOffset()]\n");
		return;
	}
	RecArrayParam.Append(RecArray);
	if(iOrgLen != RecArrayParam.GetSize())
	{
		ads_printf("\nERROR: Control should not reach here [CXSecStn::_SortByOffset()]\n");
		return;
	}
}
int CXSecStn::_GetMinOffsetDistIndex(const CCSDataRecArray& RecArray)
{
	int i, iIndex;
	
	if(RecArray.GetSize() < 1)
		return -1;//Error
	if(RecArray.GetSize() == 1)
		return 0;

	for(i = 0, iIndex = 0; i < RecArray.GetSize(); i++)
	{
		CSDREC* pRecMin;
		CSDREC* pRecCur;

		pRecCur = RecArray[i];
		pRecMin = RecArray[iIndex];
		
		if(pRecCur->dOffset < pRecMin->dOffset)
			iIndex = i;
	}
	return iIndex;
}
BOOL CXSecStn::Read(CStdioFile& objFile, BOOL& bIsEOF, long& iLineNo)
{
	DeallocatePtrArray(m_RecArray);
	for(bIsEOF = FALSE; (bIsEOF == FALSE);)
	{
		CStringArray strWordArray;
		
		if(!ReadNextValidLine(objFile, strWordArray, iLineNo))
		{//EOF reached...
			if(m_RecArray.GetSize() != 0)
			{//Chk whether this data set is OK or not ....may be an error
				CSDREC* pRecTmp;
				
				pRecTmp = m_RecArray[m_RecArray.GetSize() - 1];
				if(pRecTmp->lBit != 10)
				{
					DeallocatePtrArray(m_RecArray);
					return FALSE;//ERROR
				}
				return TRUE;//ERROR
			}
			bIsEOF = TRUE;
			return TRUE;
		}
		{//
			int iBitCode;

			iBitCode = atoi(strWordArray[0]);
			if(iBitCode == 10)
			{//End of this Station Data
				_SortByOffset(m_RecArray);
				return (m_RecArray.GetSize() > 0);
			}
			if(iBitCode == 9)
			{//Start of this Station Data
				if(m_RecArray.GetSize() != 0)
				{
					DeallocatePtrArray(m_RecArray);//for Safety
					return FALSE;
				}
				if(!ParseCSDStartLine(strWordArray, m_fChg, m_fDatum))
				{
					CString strTmp;

					strTmp.Format("Invalid no of parameters found at line #%d (the start of data segment)\nExpected format is <<Bit Code = 9>><<Chainage>><<Datum Level>>", iLineNo);
					AfxMessageBox(strTmp, MB_ICONSTOP);

					DeallocatePtrArray(m_RecArray);
					return FALSE;
				}
			}
			else
			{//Collect lines Data 
				CSDREC* pNewRec;

				pNewRec = new CSDREC;
				if(!ParseCSDLineInput(strWordArray, pNewRec))
				{
					delete pNewRec;
					DeallocatePtrArray(m_RecArray);
					return FALSE;//ERROR
				}
				m_RecArray.Add(pNewRec);
			}
		}
	}
	_SortByOffset(m_RecArray);
	return (m_RecArray.GetSize() > 0);
}
BOOL CXSecStn::ParseCSDStartLine(const CStringArray& strArray, double& fChg, double& fDatum)
{
	long lBit;
	
	lBit = atoi(strArray[0]);
	if(lBit != 9)
		return FALSE;
	
	if(strArray.GetSize() < 3)
		return FALSE;//Chg/Datum or both not found
	
	fChg	= atof(strArray[1]);
	fDatum	= atof(strArray[2]);
	return TRUE;
}
//;******************************************************  
//;*Bit Code Specification for each Line of Station data
//;*	0 = No Text/Block Annotation & Ordinate Line
//;*	1 = Only Draws Ordinate Line
//;*	2 = Only Draws Text Annotation
//;*	4 = Only Inserts the specified Blockname
//;******************************************************
BOOL CXSecStn::ParseCSDLineInput(const CStringArray& strArray, CSDREC* pRecParam)
{
	if(strArray.GetSize() < 3)
		return FALSE;
	pRecParam->lBit = atoi(strArray[0]);
	//
	pRecParam->dOffset	= atof(strArray[1]);
	pRecParam->dElev	= atof(strArray[2]);

	//
	if((pRecParam->lBit & 4) && (pRecParam->lBit & 2))
	{
		if(strArray.GetSize() < 5)
		{
			return FALSE;//Text/Block or both not found
		}
		else
		{
			pRecParam->strTxt = strArray[3];
			pRecParam->strBlk = strArray[4];
		}
	}
	else
	{
		if((pRecParam->lBit & 2) || (pRecParam->lBit & 4))
		{
			if(strArray.GetSize() < 4)
				return FALSE;//Text/Block not found
			if(pRecParam->lBit & 2)
			{
				pRecParam->strTxt = strArray[3];
			}
			else
			{
				if(pRecParam->lBit & 4)
					pRecParam->strBlk = strArray[3];
			}

		}
	}
	return TRUE;
}
double CXSecStn::GetMinOffset()
{
	if(m_RecArray.GetSize() <= 0)
	{
		ads_printf("\nERROR: Control should not reach here! [CXSecStn::GetMinOffset()]\n\n");
		return 0.0;
	}
	return (m_RecArray[0])->dOffset;
}
double CXSecStn::GetMaxOffset()
{
	if(m_RecArray.GetSize() <= 0)
	{
		ads_printf("\nERROR: Control should not reach here! [CXSecStn::GetMaxOffset()]\n\n");
		return 0.0;
	}
	return (m_RecArray[m_RecArray.GetSize()-1])->dOffset;
}
BOOL CXSecStn::FindMinMaxOffElev(double& fMinOff, double& fMaxOff, double& fMaxElev)
{
	int i;
	if(m_RecArray.GetSize() <= 0)
		return FALSE;//Not Initialized Properly
	fMinOff = GetMinOffset();
	fMaxOff = GetMaxOffset();
	fMaxElev = 0.0;

	for(i = 1, fMaxElev = (m_RecArray[0])->dElev; i < m_RecArray.GetSize(); i++)
	{
		if((m_RecArray[i])->dElev > fMaxElev)
			fMaxElev = (m_RecArray[i])->dElev;
	}

	return TRUE;
}
CXSecDsgnStn* CXSecStn::_GetAppropriateDsgnData(const CXSecDsgnStnArray& arrayDsgnDataParan) const
{
	int i;

	for(i = 0; i < arrayDsgnDataParan.GetSize(); i++)
	{
		if((arrayDsgnDataParan[i])->IsThisForChg(m_fChg))
		{
			return (arrayDsgnDataParan[i]);
		}
	}
	return 0L;
}
BOOL CXSecStn::GetExtents(const CSDDWGPARAMEX& objDwgAttribs, CXSecDsgnStnArray& arrayDsgnDataParan, double& fWid, double& fHt, AXISDETAIL* pAxisDetRec)
{
	CXSecDsgnStn *pDsgnRec;
	double fMaxHt;


	if(pAxisDetRec == 0L)
	{
		ads_printf("\nERROR: Control should not reach here [CXSecStn::GetExtents()]!!");
		return FALSE;
	}
	{//Initialize default values which are dependent to 'CSDDWGPARAMEX'
		pAxisDetRec->fTxtWidMF		= 1.0;
		pAxisDetRec->fTitleXtn		= objDwgAttribs.dCSLabelAnnSize * 2.25;
		pAxisDetRec->fMargin		= objDwgAttribs.dRowAnnSize * 2.0;
		pAxisDetRec->iMaxDatumDigits= 7;
		
		pAxisDetRec->fRiteOfYAxisA = 0.0;
		pAxisDetRec->fRiteOfYAxisB = 0.0;
	}

	pDsgnRec = _GetAppropriateDsgnData(arrayDsgnDataParan);
	if(pDsgnRec == 0L)
	{//Design data not found
		return FALSE;
	}
	{//Find overall Min/Max Offset, Elev
		double fMinOff, fMaxOff, fMaxElev;
		double fMinOffDsgn, fMaxOffDsgn, fMinOffOrg, fMaxOffOrg, fMaxElevDsgn, fMaxElevOrg;
		
		pDsgnRec->FindMinMaxOffElev(m_fChg, fMinOffDsgn, fMaxOffDsgn, fMaxElevDsgn);
		this->FindMinMaxOffElev(fMinOffOrg, fMaxOffOrg, fMaxElevOrg);
		
		fMaxElev = (fMaxElevDsgn > fMaxElevOrg) ? fMaxElevDsgn:fMaxElevOrg;
		fMaxHt = fMaxElev - m_fDatum;

		fMinOff = (fMinOffDsgn < fMinOffOrg) ? fMinOffDsgn:fMinOffOrg;
		fMaxOff = (fMaxOffDsgn > fMaxOffOrg) ? fMaxOffDsgn:fMaxOffOrg;

		if(fMinOff > 0.0)
			fMinOff = 0.0;


		pAxisDetRec->fRiteOfYAxisA = (fabs(fMinOff) * objDwgAttribs.dXScale);//Space for Ordinates (Left of Graph Org)
		pAxisDetRec->fRiteOfYAxisB = (fMaxOff * objDwgAttribs.dXScale);//Space for Ordinates (Rite of Graph Org)
	}
	{//Calculate Extents now(Horz)
		int iMaxLenOfTagStrs;

		iMaxLenOfTagStrs = __max (strlen(objDwgAttribs.szElevTag), strlen(objDwgAttribs.szOffsetTag));
		iMaxLenOfTagStrs = __max ((int)strlen(objDwgAttribs.szDatumTag) + (int)pAxisDetRec->iMaxDatumDigits, (int)iMaxLenOfTagStrs);

		fWid = 0.0;
		fWid += (pAxisDetRec->fRiteOfYAxisA + pAxisDetRec->fRiteOfYAxisB);//Space for Ordinates
		fWid += (iMaxLenOfTagStrs * objDwgAttribs.dRowAnnSize * m_fTxtWidMF);// Space for Annoation
	
		fWid += (2.0 * pAxisDetRec->fMargin);
		
		pAxisDetRec->fLeftOfYAxis = (iMaxLenOfTagStrs * objDwgAttribs.dRowAnnSize * pAxisDetRec->fTxtWidMF);// Space for Annoation
	}
	{//Calculate Extents now(Vert)

		fHt = 0.0;
		fHt += (fMaxHt * objDwgAttribs.dYScale);//Space for Ordinates
		fHt += (objDwgAttribs.dRowHt * 3.0 + pAxisDetRec->fTitleXtn);// Space for Annoation
		
		fHt += (2.0 * pAxisDetRec->fMargin);

		pAxisDetRec->fTopOfXAxis = (fMaxHt * objDwgAttribs.dYScale);//Space for Ordinates
		pAxisDetRec->fBotOfXAxis = (objDwgAttribs.dRowHt * 3.0 + pAxisDetRec->fTitleXtn);// Space for Annoation
	}
	return TRUE;
}
BOOL CXSecStn::CalcCutOrFillValue(CXSecDsgnStnArray& arrayDsgnDataParan, double& fFieldDataVol, double& fDsgnVol)
{
	CXSecDsgnStn* pXSecDsgnStn;
	
	fDsgnVol = -1.0;
	fFieldDataVol = -1.0;

	pXSecDsgnStn = _GetAppropriateDsgnData(arrayDsgnDataParan);
	if(pXSecDsgnStn == 0L)
		return FALSE;

	pXSecDsgnStn->CalcElevations(m_fChg);
	{//Calculate Area
		double fTmp, fMinElev;

		fTmp = GetMinElev();
		fMinElev = pXSecDsgnStn->GetMinElev();
		if(fTmp < fMinElev)
			fMinElev = fTmp;

		fDsgnVol = pXSecDsgnStn->GetVolPerUnitLen(fMinElev);
		fFieldDataVol = GetVolPerUnitLen(pXSecDsgnStn, fMinElev);
	}
	return TRUE;
}
BOOL CXSecStn::Draw(const AcGePoint3d& ptLLC, const CSDDWGPARAMEX& objDwgAttribs, CXSecDsgnStnArray& arrayDsgnDataParan, CStdioFile* pObjLogFile /*= 0L*/)
{
	double fWid, fHt;
	AXISDETAIL recAxisDet;
	AcGePoint2d ptGraphOrg;
	CXSecDsgnStn* pXSecDsgnStn;
	
	pXSecDsgnStn = _GetAppropriateDsgnData(arrayDsgnDataParan);
	if(pXSecDsgnStn == 0L)
		return FALSE;

	if(!GetExtents(objDwgAttribs, arrayDsgnDataParan, fWid, fHt, &recAxisDet))
		return FALSE;
	
	RectangleLL(ptLLC, fWid, fHt, objDwgAttribs.szDwgSheet);//Border

	_DrawAxis(objDwgAttribs, ptLLC, ptGraphOrg, recAxisDet);
	_DrawOrdinates(objDwgAttribs, ptGraphOrg, recAxisDet);
	_DrawDsgnOrdinates(pXSecDsgnStn, objDwgAttribs, ptGraphOrg, recAxisDet);
	
	if(pObjLogFile != 0L)
	{//Write Log file
		CString strLine;

		strLine.Format("Calculation log at Chainage : %.3f\n", m_fChg);
		pObjLogFile->WriteString(strLine);
	}
	{//Calculate Area
		double fTmp, fMinElev, fDsgnVol, fFieldDataVol;

		fTmp = GetMinElev();
		fMinElev = pXSecDsgnStn->GetMinElev();
		if(fTmp < fMinElev)
			fMinElev = fTmp;

		if(!CalcCutOrFillValue(arrayDsgnDataParan, fFieldDataVol, fDsgnVol))
			return FALSE;

		if(pObjLogFile != 0L)
		{//Write Log file
			CString strLine;

			strLine.Format("Volume per unit length for considering Datum : %.3f\n\tProposed Section : %.3f\n\tOriginal Section : %.3f\n", fMinElev, fDsgnVol, fFieldDataVol);
			pObjLogFile->WriteString(strLine);
			strLine.Format("Result : %.3f(%s)\n", fabs(fFieldDataVol - fDsgnVol), ((fFieldDataVol - fDsgnVol) > 0.0) ? "Cut":"Fill");
			pObjLogFile->WriteString(strLine);
			strLine.Format("****|****|****|****|****|****|->SurvUtilARX on %s|<-|****|****|****|****|****|****\n\n", COleDateTime::GetCurrentTime().Format());
			pObjLogFile->WriteString(strLine);
		}
		//
		_DrawLabel(fDsgnVol, fFieldDataVol, objDwgAttribs, ptLLC, recAxisDet);
	}
	return TRUE;
}
void CXSecStn::_DrawLabel(const double& fVolmDsgn, const double& fVolmOrg, const CSDDWGPARAMEX& RecParam, const AcGePoint3d& ptLLC, const AXISDETAIL& recAxisDet)
{
	CString strCaption;
	AcGePoint3d ptIns;

	strCaption.Format("%s %.3f", RecParam.szXSecLabelTag, m_fChg);
	ptIns = ptLLC;
	ptIns.x += recAxisDet.fMargin + (recAxisDet.fLeftOfYAxis + recAxisDet.fRiteOfYAxisA + recAxisDet.fRiteOfYAxisB) * 0.5;
	ptIns.y += recAxisDet.fMargin;
	ptIns.y += RecParam.dCSLabelAnnSize / 2.0;

	TEXTMID(ptIns, strCaption, 0.0, RecParam.dCSLabelAnnSize, RecParam.szLyrAxis);

	ptIns.y += RecParam.dCSLabelAnnSize;
	if((fVolmDsgn - fVolmOrg) < 0.0)
		strCaption.Format("%s %.3f", "Cut : ", fabs(fVolmDsgn - fVolmOrg));
	else
		strCaption.Format("%s %.3f", "Fill : ", fabs(fVolmDsgn - fVolmOrg));
	
	TEXTMID(ptIns, strCaption, 0.0, (RecParam.dCSLabelAnnSize / 2.0), RecParam.szLyrAxis);
}
BOOL CXSecStn::GetElevAt(const double& fOffset, double& fElev)
{
	int i;
	BOOL bFound;
	CSDREC *pRecCur;
	CSDREC *pRecPrev;

	if(m_RecArray.GetSize() <= 1)
	{
		ads_printf("\nERROR: Volume Calculation failed, inadequate # of data found\n");
		return FALSE;
	}
	fElev = 0.0;
	for(i = 1, bFound = FALSE; i < m_RecArray.GetSize(); i++)
	{
		pRecPrev	= m_RecArray[i - 1];
		pRecCur		= m_RecArray[i];
		if((fOffset >= pRecPrev->dOffset) && (fOffset <= pRecCur->dOffset))
		{//Found,,,,
			const double fDistDiff = pRecCur->dOffset - pRecPrev->dOffset;
			const double fElevDiff = pRecCur->dElev - pRecPrev->dElev;

			if(fDistDiff == 0.0)
				fElev = pRecPrev->dElev;
			else
				fElev = pRecPrev->dElev + (fElevDiff / fDistDiff) * (fOffset - pRecPrev->dOffset);
			return TRUE;
		}
	}
	return FALSE;//Not Found,,,,
}
double CXSecStn::GetVolPerUnitLen(CXSecDsgnStn* pXSecDsgnStn, const double& fMinElev, CStdioFile* pObjLogFile /*= 0L*/)
{
	CCSDataRecArray tmpRecArray;
	CCSDataRecArray tmpRecArrayForNewRecs;
	int i;
	CSDREC *pRecCur;
	CSDREC *pRecPrev;
	BOOL bIsErrOccured;
	double fValToRet;

	if(m_RecArray.GetSize() <= 1)
	{
		ads_printf("\nERROR: Volume Calculation failed, inadequate # of data found\n");
		return 0.0;
	}

	bIsErrOccured = FALSE;
	{//Form New Existing Section
		BOOL bGotLeftOffset, bGotRiteOffset;
		const double fMinDsgnOffset = pXSecDsgnStn->GetMinOffset();
		const double fMaxDsgnOffset = pXSecDsgnStn->GetMaxOffset();

		bGotLeftOffset = FALSE;
		bGotRiteOffset = FALSE;
		for(i = 0; i < m_RecArray.GetSize(); i++)
		{
			pRecCur		= m_RecArray[i];

			if(!bGotLeftOffset)
			{
				if(fMinDsgnOffset < pRecCur->dOffset)
				{
					CSDREC *pRecTmp;

					pRecTmp = new CSDREC;
					*pRecTmp = *pRecCur;
					pRecTmp->dOffset = fMinDsgnOffset;
					
					tmpRecArrayForNewRecs.Add(pRecTmp);
					tmpRecArray.Add(pRecTmp);
					bGotLeftOffset = TRUE;
				}
				else if(fMinDsgnOffset == pRecCur->dOffset)
				{
					tmpRecArray.Add(pRecCur);
					bGotLeftOffset = TRUE;
				}
			}
			if(!bGotLeftOffset && (i > 0))
			{
				CSDREC *pRecPrev = m_RecArray[i - 1];
				if((fMinDsgnOffset >= pRecPrev->dOffset) && (fMinDsgnOffset <= pRecCur->dOffset))
				{
					CSDREC *pRecTmp;

					pRecTmp = new CSDREC;
					*pRecTmp = *pRecCur;
					pRecTmp->dOffset = fMinDsgnOffset;
					bGotLeftOffset = GetElevAt(fMinDsgnOffset, pRecTmp->dElev);
					
					tmpRecArrayForNewRecs.Add(pRecTmp);
					tmpRecArray.Add(pRecTmp);
				}
			}
			else if(bGotLeftOffset)
			{
				if(pRecCur->dOffset <= fMaxDsgnOffset)
				{
					tmpRecArray.Add(pRecCur);
					if(i == (m_RecArray.GetSize() - 1))
					{//Last offset-> Designed Max Offset is Out of bound of Existing data
						CSDREC *pRecTmp;

						pRecTmp = new CSDREC;
						*pRecTmp = *pRecCur;
						pRecTmp->dOffset = fMaxDsgnOffset;
						
						tmpRecArrayForNewRecs.Add(pRecTmp);
						tmpRecArray.Add(pRecTmp);
						bGotRiteOffset = TRUE;
						break;
					}
				}
				else
				{
					CSDREC *pRecTmp;

					pRecTmp = new CSDREC;
					*pRecTmp = *pRecCur;
					pRecTmp->dOffset = fMaxDsgnOffset;
					bGotRiteOffset = GetElevAt(fMaxDsgnOffset, pRecTmp->dElev);
					
					tmpRecArrayForNewRecs.Add(pRecTmp);
					tmpRecArray.Add(pRecTmp);
					break;
				}
			}
		}
		bIsErrOccured = !(bGotLeftOffset && bGotRiteOffset);
	}
	
	if(bIsErrOccured)
	{//ERROR
		CString strTemp;

		strTemp.Format("ERROR: Failed to Calculate Volume at Chainage %.3f\nPossible cause : Proposed Section is not compatible with existing Section", m_fChg);
		AfxMessageBox(strTemp, MB_ICONSTOP|MB_ICONERROR);
		fValToRet = 0.0;
		
		if(pObjLogFile != 0L)
		{//Write Log file
			CString strLine;

			strLine.Format("ERROR: Failed to Calculate Volume at Chainage %.3f\nPossible cause : Proposed Section is not compatible with existing Section", m_fChg);
			pObjLogFile->WriteString(strLine);
		}
	}
	else
	{
		_SortByOffset(tmpRecArray);

		if(pObjLogFile != 0L)
		{//Write Log file
			CString strLine;

			strLine.Format("Following Ordinates (%d #s) considered for volume (per unit length) calculation\n", tmpRecArray.GetSize());
			pObjLogFile->WriteString(strLine);
		}
		for(i = 1, fValToRet = 0.0; i < tmpRecArray.GetSize(); i++)
		{
			pRecPrev	= tmpRecArray[i - 1];
			pRecCur		= tmpRecArray[i];

			fValToRet += CXSecDsgnStn::CalcArea(fMinElev, pRecPrev->dElev, pRecCur->dElev, pRecPrev->dOffset, pRecCur->dOffset);
		}
		if(pObjLogFile != 0L)
		{//Write Log file
			for(i = 0; i < tmpRecArray.GetSize(); i++)
			{
				CString strLine;

				pRecPrev	= tmpRecArray[i];
				const BOOL bIsNewOrd = (FindInPtrArray(pRecPrev, tmpRecArrayForNewRecs) >= 0);

				if(!bIsNewOrd)
				{
					strLine.Format("#%3d) at Offset %.3f Elevation = %.3f\n", i, pRecPrev->dOffset, pRecPrev->dElev);
				}
				else
				{
					strLine.Format("#%3d) at Offset %.3f Elevation = %.3f [Calculated WRT Proposed C/S Data]\n", i, pRecPrev->dOffset, pRecPrev->dElev);
				}
				pObjLogFile->WriteString(strLine);
			}
		}
	}

	DeallocatePtrArray(tmpRecArrayForNewRecs);
	return fValToRet;
}
double CXSecStn::GetMinElev()
{
	int i;
	double fMinElev;

	if(m_RecArray.GetSize() <= 0)
	{
		ads_printf("\nERROR: Control should not reach here [CXSecStn::CalcArea() Tag=A]\n");
		return -1.0;
	}

	fMinElev = (m_RecArray[0])->dElev;
	for(i = 1; i < m_RecArray.GetSize(); i++)
	{
		if((m_RecArray[i])->dElev < fMinElev)
			fMinElev = m_RecArray[i]->dElev;
	}
	return fMinElev;
}
void CXSecStn::_DrawDsgnOrdinates(CXSecDsgnStn* pXSecDsgnStn, const CSDDWGPARAMEX& RecParam, const AcGePoint2d& ptGrphOrg, const AXISDETAIL& /*recAxisDet*/)
{
	CCSDataRecArrayEx* pCSDataRecArrayEx;
	AcGePoint3dArray ProfTopPtsArray;
	AcGePoint3d PtStart, PtEnd, PtTxtOrg;
	double fOrdDist, fElev;
	int i, j;
	ads_name entLast;
	CSDRECEX* pRec;

	pCSDataRecArrayEx = pXSecDsgnStn->GetOrdinates();
	
	//Draw Ords
	for(i = 0; i < pCSDataRecArrayEx->GetSize(); i++)
	{
		pRec = pCSDataRecArrayEx->GetAt(i);
		fOrdDist = pRec->dOffset * RecParam.dXScale;
		fElev = (pRec->dCalcElev - m_fDatum) * RecParam.dYScale;

		//Elev 
		PtTxtOrg.x = ptGrphOrg.x; PtTxtOrg.y = ptGrphOrg.y;
		PtTxtOrg.x  += fOrdDist;
		PtTxtOrg.y  -= (1.5 * RecParam.dRowHt);
		TEXTMID(PtTxtOrg, RTOS(pRec->dCalcElev), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotationDsgn);
		if(RTNORM != ads_entlast (entLast))
		{
			ads_printf("\nERROR: Fatal error!");
			return;
		}
		if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszElevTag, pRec->dCalcElev))
		{
			ads_printf("\nERROR: Fatal error!");
			return;
		}
		//Offset
		PtTxtOrg.x = ptGrphOrg.x; PtTxtOrg.y = ptGrphOrg.y;
		PtTxtOrg.x  += fOrdDist;
		PtTxtOrg.y  -= (2.5 * RecParam.dRowHt);
		TEXTMID(PtTxtOrg, RTOS(pRec->dOffset), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotationDsgn);
		if(RTNORM != ads_entlast (entLast))
		{
			ads_printf("\nERROR: Fatal error!");
			return;
		}

		{//Ord
			PtStart.x = ptGrphOrg.x;
			PtStart.y = ptGrphOrg.y;
			PtStart.x  += fOrdDist;

			PtEnd = PtStart;
			PtEnd.y  += fElev;
			
			if(pRec->lBit & 1)
			{
				LINE(PtStart, PtEnd, RecParam.szLyrOrdinateDsgn);
			}
			//Add to Vertex Array...
			ProfTopPtsArray.append(PtEnd);
			//Text Block Annotation
			//******************************************************
			//*Bit Code Specification for each Line of Station data
			//*	0 = No Text/Block Annotation & Ordinate Line
			//*	1 = Only Draws Ordinate Line
			//*	2 = Only Draws Text Annotation
			//*	4 = Only Inserts the specified Blockname
			//******************************************************
			if(pRec->lBit & 2)
			{
				PtStart = PtEnd;
				PtStart.y += RecParam.dNumAnnSize;
				TEXTLEFT(PtStart, pRec->strTxt, DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotationDsgn);
			}
			if(pRec->lBit & 4)
			{
				PtStart = PtEnd;
				PtStart.y += RecParam.dNumAnnSize;
				if(!IsBlockExists(pRec->strBlk))
				{
					ads_printf("\nERROR: Block \"%s\" does not exists...Insertion failed\n", pRec->strBlk);
				}
				else
				{
					BLOCK(pRec->strBlk, PtStart, RecParam.szLyrAnnotationDsgn, 0.0, RecParam.dNumAnnSize, RecParam.dNumAnnSize);
				}
			}

			//Draw Ticks Alreay drawn
			for(j = 0; j <= 3; j++)
			{
				switch(j)
				{
				case 0:
					PtStart.x = ptGrphOrg.x;
					PtStart.y = ptGrphOrg.y;
					PtStart.x  += fOrdDist;
					PtStart.y  -= ((j * RecParam.dRowHt)  + (RecParam.dNumAnnSize * 0.0));
			
					PtEnd = PtStart;
					PtEnd.y  -= (RecParam.dNumAnnSize * 1.0);
					break;
				case 3:
					PtStart.x = ptGrphOrg.x;
					PtStart.y = ptGrphOrg.y;
					PtStart.x  += fOrdDist;
					PtStart.y  -= ((j * RecParam.dRowHt)  + (RecParam.dNumAnnSize * 0.0));
			
					PtEnd = PtStart;
					PtEnd.y  += (RecParam.dNumAnnSize * 1.0);
					break;
				default:
					PtStart.x = ptGrphOrg.x;
					PtStart.y = ptGrphOrg.y;
					PtStart.x  += fOrdDist;
					PtStart.y  -= ((j * RecParam.dRowHt) - (RecParam.dNumAnnSize * 0.5));
			
					PtEnd = PtStart;
					PtEnd.y  -= (RecParam.dNumAnnSize * 1.0);
			
					break;
				}
				LINE(PtStart, PtEnd, RecParam.szLyrAnnotationDsgn);
			}
		}
	}
	POLYLINE(ProfTopPtsArray, RecParam.szLyrTopSurfaceDsgn, FALSE);
	if(strlen(RecParam.szPropTopSurfAnnBlk) > 0)
	{
		AnnotateTopSurface(ProfTopPtsArray, RecParam.dPropTopSurfAnnBlkScale, (RecParam.dPropTopSurfAnnBlkScaleMin * RecParam.dXScale), RecParam.szPropTopSurfAnnBlk, RecParam.szLyrTopSurfaceDsgn);
	}
}
void CXSecStn::_DrawOrdinates(const CSDDWGPARAMEX& RecParam, const AcGePoint2d& ptGrphOrg, const AXISDETAIL& /*recAxisDet*/)
{
	AcGePoint3dArray ProfTopPtsArray;
	AcGePoint3d PtStart, PtEnd, PtTxtOrg;
	double fOrdDist, fElev;
	int i, j;
	ads_name entLast;
	CSDREC* pRec;

	//Draw Ords
	for(i = 0; i < m_RecArray.GetSize(); i++)
	{
		pRec = m_RecArray[i];
		fOrdDist = pRec->dOffset * RecParam.dXScale;
		fElev = (pRec->dElev - m_fDatum) * RecParam.dYScale;

		//Elev 
		PtTxtOrg.x = ptGrphOrg.x; PtTxtOrg.y = ptGrphOrg.y;
		PtTxtOrg.x  += fOrdDist;
		PtTxtOrg.y  -= (0.5 * RecParam.dRowHt);
		TEXTMID(PtTxtOrg, RTOS(pRec->dElev), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
		if(RTNORM != ads_entlast (entLast))
		{
			ads_printf("\nERROR: Fatal error!");
			return;
		}
		if(1 != CXDataPflAndCS::AttachXData(entLast, CXDataPflAndCS::m_pszElevTag, pRec->dElev))
		{
			ads_printf("\nERROR: Fatal error!");
			return;
		}
		//Offset
		PtTxtOrg.x = ptGrphOrg.x; PtTxtOrg.y = ptGrphOrg.y;
		PtTxtOrg.x  += fOrdDist;
		PtTxtOrg.y  -= (2.5 * RecParam.dRowHt);
		TEXTMID(PtTxtOrg, RTOS(pRec->dOffset), DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
		if(RTNORM != ads_entlast (entLast))
		{
			ads_printf("\nERROR: Fatal error!");
			return;
		}

		{//Ord
			PtStart.x = ptGrphOrg.x;
			PtStart.y = ptGrphOrg.y;
			PtStart.x  += fOrdDist;

			PtEnd = PtStart;
			PtEnd.y  += fElev;
			
			if(pRec->lBit & 1)
			{
				LINE(PtStart, PtEnd, RecParam.szLyrOrdinate);
			}
			//Add to Vertex Array...
			ProfTopPtsArray.append(PtEnd);
			//Text Block Annotation
			//******************************************************
			//*Bit Code Specification for each Line of Station data
			//*	0 = No Text/Block Annotation & Ordinate Line
			//*	1 = Only Draws Ordinate Line
			//*	2 = Only Draws Text Annotation
			//*	4 = Only Inserts the specified Blockname
			//******************************************************
			if(pRec->lBit & 2)
			{
				PtStart = PtEnd;
				PtStart.y += RecParam.dNumAnnSize;
				TEXTLEFT(PtStart, pRec->strTxt, DTOR(90.0), RecParam.dNumAnnSize, RecParam.szLyrAnnotation);
			}
			if(pRec->lBit & 4)
			{
				PtStart = PtEnd;
				PtStart.y += RecParam.dNumAnnSize;
				if(!IsBlockExists(pRec->strBlk))
				{
					ads_printf("\nERROR: Block \"%s\" does not exists...Insertion failed\n", pRec->strBlk);
				}
				else
				{
					BLOCK(pRec->strBlk, PtStart, RecParam.szLyrAnnotation, 0.0, RecParam.dNumAnnSize, RecParam.dNumAnnSize);
				}
			}

			//Draw Ticks
			for(j = 0; j <= 3; j++)
			{
				switch(j)
				{
				case 0:
					PtStart.x = ptGrphOrg.x;
					PtStart.y = ptGrphOrg.y;
					PtStart.x  += fOrdDist;
					PtStart.y  -= ((j * RecParam.dRowHt)  + (RecParam.dNumAnnSize * 0.0));

					PtEnd = PtStart;
					PtEnd.y  -= (RecParam.dNumAnnSize * 1.0);
					break;
				case 3:
					PtStart.x = ptGrphOrg.x;
					PtStart.y = ptGrphOrg.y;
					PtStart.x  += fOrdDist;
					PtStart.y  -= ((j * RecParam.dRowHt)  + (RecParam.dNumAnnSize * 0.0));

					PtEnd = PtStart;
					PtEnd.y  += (RecParam.dNumAnnSize * 1.0);
					break;
				default:
					PtStart.x = ptGrphOrg.x;
					PtStart.y = ptGrphOrg.y;
					PtStart.x  += fOrdDist;
					PtStart.y  -= ((j * RecParam.dRowHt) - (RecParam.dNumAnnSize * 0.5));

					PtEnd = PtStart;
					PtEnd.y  -= (RecParam.dNumAnnSize * 1.0);

					break;
				}
				LINE(PtStart, PtEnd, RecParam.szLyrAnnotation);
			}
		}
	}
	POLYLINE(ProfTopPtsArray, RecParam.szLyrTopSurface, FALSE);
	if(strlen(RecParam.szXistTopSurfAnnBlk) > 0)
	{
		AnnotateTopSurface(ProfTopPtsArray, RecParam.dXistTopSurfAnnBlkScale, RecParam.dXistTopSurfAnnBlkSpaceMin, RecParam.szXistTopSurfAnnBlk, RecParam.szLyrTopSurface);
	}
}
void CXSecStn::_DrawAxis(const CSDDWGPARAMEX& RecParam, const AcGePoint3d& ptLLCParam, AcGePoint2d& ptGraphOrg, const AXISDETAIL& recAxisDet)
{
	int i;
	double	dTotalXAxisLen;
	AcGePoint3d PtStart, PtEnd, ptLLCOfExtent;

	ptLLCOfExtent = ptLLCParam;
	ptLLCOfExtent.x += recAxisDet.fMargin;
	ptLLCOfExtent.y += (recAxisDet.fTitleXtn + recAxisDet.fMargin);

	dTotalXAxisLen = recAxisDet.fLeftOfYAxis + recAxisDet.fRiteOfYAxisA + recAxisDet.fRiteOfYAxisB;

	//Draw Axis..Horz Lines
	for(i = 0; i <= 3; i++)
	{
		if((i == 0) || (i == 3))
		{
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * RecParam.dRowHt);	PtStart.z	= 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * RecParam.dRowHt);	PtEnd.z		= 0.0;
			
			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
		}
		switch(i)
		{
		case 0:
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * RecParam.dRowHt) + RecParam.dNumAnnSize;	PtStart.z	= 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * RecParam.dRowHt) + RecParam.dNumAnnSize;	PtEnd.z		= 0.0;

			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		case 3:
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y  = ptLLCOfExtent.y	+ (i * RecParam.dRowHt) - RecParam.dNumAnnSize;	PtStart.z	= 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y	+ (i * RecParam.dRowHt) - RecParam.dNumAnnSize;	PtEnd.z		= 0.0;

			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		default:
			PtStart.x	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * RecParam.dRowHt) - (RecParam.dNumAnnSize / 2.0);	PtStart.z = 0.0;
			PtEnd.x		= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * RecParam.dRowHt) - (RecParam.dNumAnnSize / 2.0);	PtEnd.z = 0.0;

			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			
			PtStart[X]	= ptLLCOfExtent.x;					PtStart.y	= ptLLCOfExtent.y + (i * RecParam.dRowHt) + (RecParam.dNumAnnSize / 2.0);	PtStart.z = 0.0;
			PtEnd[X]	= ptLLCOfExtent.x + dTotalXAxisLen;	PtEnd.y		= ptLLCOfExtent.y + (i * RecParam.dRowHt) + (RecParam.dNumAnnSize / 2.0);	PtEnd.z = 0.0;

			LINE(PtStart, PtEnd, RecParam.szLyrAxis);
			break;
		}

	}
	//Draw Axis..Vert Lines
	PtStart.x	= ptLLCOfExtent.x;	PtStart.y	= ptLLCOfExtent.y + (0.0 * RecParam.dRowHt);	PtStart.z = 0.0;
	PtEnd.x		= ptLLCOfExtent.x;	PtEnd.y		= ptLLCOfExtent.y + (3.0 * RecParam.dRowHt);	PtEnd.z = 0.0;

	LINE(PtStart, PtEnd, RecParam.szLyrAxis);

	//Text Labels...
	//Elev (Field)
	PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (2.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, RecParam.szElevTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	//Elev (Design)
	PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (1.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, RecParam.szElevTagDsgn, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	//Offset 
	PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (0.5 * RecParam.dRowHt);	PtStart.z = 0.0;
	TEXTLEFT(PtStart, RecParam.szOffsetTag, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	
	{//Datum 
		CString strTmp;
		
		PtStart.x	= ptLLCOfExtent.x + (RecParam.dRowAnnSize / 2.0);	PtStart.y  = ptLLCOfExtent.y	+ (3.0 * RecParam.dRowHt) + (1.25 * RecParam.dRowAnnSize);	PtStart.z = 0.0;
		
		strTmp.Format("%s %s", RecParam.szDatumTag, RTOS(m_fDatum));

		TEXTLEFT(PtStart, strTmp, 0.0, RecParam.dRowAnnSize, RecParam.szLyrAxis);
	}
	ptGraphOrg.x = ptLLCOfExtent.x;
	ptGraphOrg.y = ptLLCOfExtent.y;

	ptGraphOrg.x += (recAxisDet.fLeftOfYAxis + recAxisDet.fRiteOfYAxisA);
	ptGraphOrg.y += (3.0 * RecParam.dRowHt);
}

void CXSecStn::RectangleLL(const AcGePoint3d& ptLowerLeft, const double& fWid, const double& fHt, const char* pszLyr)
{
	AcGePoint3d Pt01, Pt02, Pt03, Pt04;
	AcGePoint3dArray ptArrayTmp;

	//Lower Left
	Pt01 = ptLowerLeft;
	//lower Right
	Pt02 = Pt01;
	Pt02.x = Pt01.x + fWid;
	//Top Right
	Pt03 = Pt02;
	Pt03.y = Pt02.y + fHt;
	//Top Left
	Pt04 = Pt03;
	Pt04.x = Pt03.x - fWid;

	ptArrayTmp.append(Pt01);
	ptArrayTmp.append(Pt02);
	ptArrayTmp.append(Pt03);
	ptArrayTmp.append(Pt04);
	ptArrayTmp.append(Pt01);

	POLYLINE(ptArrayTmp, pszLyr, TRUE);
}
void CXSecStn::RectangleUL(const AcGePoint3d& ptUpperLeft, const double& fWid, const double& fHt, const char* pszLyr)
{
	AcGePoint3d Pt01, Pt02, Pt03, Pt04;
	AcGePoint3dArray ptArrayTmp;

	//Upper Left
	Pt01 = ptUpperLeft;
	//Upper Right
	Pt02 = Pt01;
	Pt02.x = Pt01.x + fWid;
	//Lower Right
	Pt03 = Pt02;
	Pt03.y = Pt02.y - fHt;
	//Lower Left
	Pt04 = Pt03;
	Pt04.x = Pt03.x - fWid;

	ptArrayTmp.append(Pt01);
	ptArrayTmp.append(Pt02);
	ptArrayTmp.append(Pt03);
	ptArrayTmp.append(Pt04);
	ptArrayTmp.append(Pt01);

	POLYLINE(ptArrayTmp, pszLyr, TRUE);
}
void CXSecStn::RectangleLR(const AcGePoint3d& ptLowerRight, const double& fWid, const double& fHt, const char* pszLyr)
{
	AcGePoint3d ptUpperLeft;

	//Upper Left
	ptUpperLeft = ptLowerRight;
	ptUpperLeft.x = ptLowerRight.x - fWid;
	ptUpperLeft.y = ptLowerRight.y + fHt;
	
	RectangleUL(ptUpperLeft, fWid, fHt, pszLyr);
}

void CXSecStn::AnnotateTopSurface(const AcGePoint3dArray& arraySurfacePts, const double& fBlkScale, const double& fMinSpa, const char* pszBlock, const char* pszLyr)
{
	double fDist, fAng;
	AcGePoint3d ptParam;
	
	if(arraySurfacePts.length() < 2)
		return;//ERROR

	if(!IsBlockExists(pszBlock))
	{
		ads_printf("\nERROR: Block \"%s\" does not exists...Insertion failed\n", pszBlock);
	}
	for(fDist = fMinSpa; ; fDist +=fMinSpa)
	{
		if(!_FindPointOnAlgn(arraySurfacePts, fDist, ptParam, fAng))
			return;
		
		BLOCK(pszBlock, ptParam, pszLyr, fAng, fBlkScale, fBlkScale);
	}
}
BOOL CXSecStn::_FindPointOnAlgn(const AcGePoint3dArray& arraySurfacePts, const double& fChg, AcGePoint3d& ptParam, double& fAngle)
{
	double fCurChg, fPrevChg;
	int i;
	
	if(arraySurfacePts.length() < 2)
		return FALSE;//ERROR

	for(i = 1, fCurChg = 0.0, fPrevChg = 0.0; i < arraySurfacePts.length(); i++)
	{
		fCurChg += Dist2D((arraySurfacePts[i - 1]), (arraySurfacePts[i]));
		if((fChg >= fPrevChg) && (fChg <= fCurChg))
		{
			AcGePoint3d ptTmp;

			fAngle = Angle2D(arraySurfacePts[i - 1], arraySurfacePts[i]);
			ads_polar(asDblArray(arraySurfacePts[i - 1]), fAngle, (fChg - fPrevChg), asDblArray(ptParam));
			return TRUE;
		}
		fPrevChg = fCurChg;
	}
	return FALSE;//Chg specified is out of bound
}
