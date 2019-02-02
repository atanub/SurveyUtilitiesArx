//OTravXLSReader.cpp
//////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "OTravXLSReader.h"
#include "SurvUtilApp.h"

//COTravXLSReader
//////////////////////////////////////////////////////////////////////////////////////////
const CString	COTravXLSReader::m_strConstSheet	= CString("OTraverse");

COTravXLSReader::COTravXLSReader()
{
	CExcelReaderGen::CExcelReaderGen();
	m_pData = 0L;
}
COTravXLSReader::~COTravXLSReader()
{
	UnInitialize(TRUE);
}
BOOL COTravXLSReader::ReadAllSheets(CStringArray* pErrStrArray)
{
	CString strMsg;
	BOOL bSuccess;

	if(CSurvUtilApp::IsCancelPressed())
	{
		strMsg.Format("ERROR: User termination !!");
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	bSuccess = FindInStrArray(m_strConstSheet, m_SheetNamesArray, FALSE);

	if(!bSuccess)
	{
		strMsg.Format("ERROR: Invalid XLS data file chosen\nThe WorkBook should contain all of the following sheets \"%s\"", m_strConstSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	////////////////////////////////////////////////////
	if(m_pData != 0L)
	{
		delete m_pData;
		m_pData = 0L;
	}
	m_pData = new COTravData();
	////////////////////////////////////////////////////
	if(!_Read(pErrStrArray))
	{
		return FALSE;
	}
	return bSuccess;
}
void COTravXLSReader::UnInitialize(const BOOL& bResetData /* = FALSE*/)
{
	CExcelReaderGen::UnInitialize(bResetData);
	if(bResetData)
	{//
		if(m_pData != 0L)
			delete m_pData;

		m_pData = 0L;
	}
}
//First GPS point
//
BOOL COTravXLSReader::_ReadFirstGPSPoint(CXLWorksheet& Worksheet, const int& iRow, TRAVVERTEX& Rec, CStringArray* pErrStrArray) const
{//[Station][EMPTY][EMPTY][EMPTY][Northing][Easting]
	const char chConstMaxCol = 'F';
	const char chConstMinCol = 'A';
	char j;
	CString strMsg, strCell, strTemp;

	for(j = chConstMinCol; j <= chConstMaxCol; j++)
	{
		COleVariant OleVar;
		LPDISPATCH pRange;
		CXLRange Range;

		strCell = MakeCellID(iRow, j);
		VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
		Range.AttachDispatch(pRange);
		OleVar = Range.GetValue();

		{//Update Progress Dialog
			strMsg.Format("Reading Data Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
			CSurvUtilApp::ProgDlgNextStep(strMsg);
		}
		if(j == (chConstMinCol + 0))
		{//Station Name
			if(!_ParseRawXLData(OleVar, &strTemp))
			{
				strMsg.Format("ERROR: Invalid Station Name found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
			if(strlen(strTemp) >= _MAX_PATH)
			{
				return FALSE;
			}
			//Valid station Name
			strcpy(Rec.szStn, (LPCSTR)strTemp);
		}
		else if((j == (chConstMinCol + 1)) || (j == (chConstMinCol + 2)) || (j == (chConstMinCol + 3)))
		{//Empty cells (B/C/D)
		}
		else
		{//Double Values [Northing][Easting] for cells E/F
			double fTmp;
			
			if(!_ParseRawXLData(OleVar, &(fTmp)))
			{
				strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
			if((chConstMinCol + 4) == j)
				Rec.fNorthing = fTmp;//N
			else if((chConstMinCol + 5) == j)
				Rec.fEasting = fTmp;//E
			else
				return FALSE;
		}
	}
	return TRUE;
}
//<2nd> GPS point
//
BOOL COTravXLSReader::_ReadSecondGPSPoint(CXLWorksheet& Worksheet, const int& iRow, TRAVVERTEX& Rec, CStringArray* pErrStrArray) const
{//[Station][Bearing][Angle][Distance]
	const char chConstMaxCol = 'D';
	const char chConstMinCol = 'A';
	char j;
	CString strMsg, strCell, strTemp;

	for(j = chConstMinCol; j <= chConstMaxCol; j++)
	{
		COleVariant OleVar;
		LPDISPATCH pRange;
		CXLRange Range;

		strCell = MakeCellID(iRow, j);
		VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
		Range.AttachDispatch(pRange);
		OleVar = Range.GetValue();

		{//Update Progress Dialog
			strMsg.Format("Reading Data Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
			CSurvUtilApp::ProgDlgNextStep(strMsg);
		}
		if(j == (chConstMinCol + 0))
		{//Station Name
			if(!_ParseRawXLData(OleVar, &strTemp))
			{
				strMsg.Format("ERROR: Invalid Station Name found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
			if(strlen(strTemp) >= _MAX_PATH)
			{
				return FALSE;
			}
			//Valid station Name
			strcpy(Rec.szStn, (LPCSTR)strTemp);
		}
		else if((chConstMinCol + 1) == j)
		{//Bearing Angle in String
			Rec.fBearing = 0.0;
			if(!_ParseRawXLData(OleVar, &strTemp))
			{
				strMsg.Format("ERROR: Invalid Bearing value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
			if(!CFBSheetData::ParseAngleString("- ", strTemp, Rec.fBearing))
			{
				strMsg.Format("ERROR: Invalid Bearing value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
		}
		else if((chConstMinCol + 2) == j)
		{//Observed included Angle in String
			Rec.fIncAngle = 0.0;
			if(!_ParseRawXLData(OleVar, &strTemp))
			{
				strMsg.Format("ERROR: Invalid angle value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
			if(!CFBSheetData::ParseAngleString("- ", strTemp, Rec.fIncAngle))
			{
				strMsg.Format("ERROR: Invalid angle value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
		}
		else if((chConstMinCol + 3) == j)
		{//Double Values Observed Distance
			if(!_ParseRawXLData(OleVar, &(Rec.fDistance)))
			{
				strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
		}
		else
			return FALSE;
	}
	return TRUE;
}
//<2nd Last> GPS point
//
BOOL COTravXLSReader::_ReadThirdGPSPoint(CXLWorksheet& Worksheet, const int& iRow, TRAVVERTEX& Rec, CStringArray* pErrStrArray) const
{//[Station][EMPTY][Angle][Distance][N][E]
	const char chConstMaxCol = 'F';
	const char chConstMinCol = 'A';
	char j;
	CString strMsg, strCell, strTemp;

	for(j = chConstMinCol; j <= chConstMaxCol; j++)
	{
		COleVariant OleVar;
		LPDISPATCH pRange;
		CXLRange Range;

		strCell = MakeCellID(iRow, j);
		VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
		Range.AttachDispatch(pRange);
		OleVar = Range.GetValue();

		{//Update Progress Dialog
			strMsg.Format("Reading Data Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
			CSurvUtilApp::ProgDlgNextStep(strMsg);
		}
		if(j == (chConstMinCol + 0))
		{//Station Name
			if(!_ParseRawXLData(OleVar, &strTemp))
			{
				strMsg.Format("ERROR: Invalid Station Name found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
			if(strlen(strTemp) >= _MAX_PATH)
			{
				return FALSE;
			}
			//Valid station Name
			strcpy(Rec.szStn, (LPCSTR)strTemp);
		}
		else if((chConstMinCol + 1) == j)
		{//Empty
		}
		else if((chConstMinCol + 2) == j)
		{//Observed included Angle in String
			Rec.fIncAngle = 0.0;
			if(!_ParseRawXLData(OleVar, &strTemp))
			{
				strMsg.Format("ERROR: Invalid angle value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
			if(!CFBSheetData::ParseAngleString("- ", strTemp, Rec.fIncAngle))
			{
				strMsg.Format("ERROR: Invalid angle value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
		}
		else
		{//[Observed Distance][N][E] as Double
			double fTmp;
			
			if(!_ParseRawXLData(OleVar, &(fTmp)))
			{
				strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
			if((chConstMinCol + 3) == j)
				Rec.fDistance = fTmp;//Dist
			else if((chConstMinCol + 4) == j)
				Rec.fNorthing = fTmp;//N
			else if((chConstMinCol + 5) == j)
				Rec.fEasting = fTmp;//E
			else
				return FALSE;
		}
	}
	return TRUE;
}
//< Last> 4th GPS point
//
BOOL COTravXLSReader::_ReadFourthGPSPoint(CXLWorksheet& Worksheet, const int& iRow, TRAVVERTEX& Rec, CStringArray* pErrStrArray) const
{//[Station][Bearing][EMPTY][Distance]
	const char chConstMaxCol = 'D';
	const char chConstMinCol = 'A';
	char j;
	CString strMsg, strCell, strTemp;

	for(j = chConstMinCol; j <= chConstMaxCol; j++)
	{
		COleVariant OleVar;
		LPDISPATCH pRange;
		CXLRange Range;

		strCell = MakeCellID(iRow, j);
		VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
		Range.AttachDispatch(pRange);
		OleVar = Range.GetValue();

		{//Update Progress Dialog
			strMsg.Format("Reading Data Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
			CSurvUtilApp::ProgDlgNextStep(strMsg);
		}
		if(j == (chConstMinCol + 0))
		{//Station Name
			if(!_ParseRawXLData(OleVar, &strTemp))
			{
				strMsg.Format("ERROR: Invalid Station Name found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
			if(strlen(strTemp) >= _MAX_PATH)
			{
				return FALSE;
			}
			//Valid station Name
			strcpy(Rec.szStn, (LPCSTR)strTemp);
		}
		else if((chConstMinCol + 1) == j)
		{//Bearing
			Rec.fBearing = 0.0;
			if(!_ParseRawXLData(OleVar, &strTemp))
			{
				strMsg.Format("ERROR: Invalid bearing value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
			if(!CFBSheetData::ParseAngleString("- ", strTemp, Rec.fBearing))
			{
				strMsg.Format("ERROR: Invalid bearing value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
		}
		else if((chConstMinCol + 2) == j)
		{//Empty
			continue;
		}
		else if((chConstMinCol + 3) == j)
		{//[Observed Distance]
			if(!_ParseRawXLData(OleVar, &(Rec.fDistance)))
			{
				strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}

//Intermediate Points
//
BOOL COTravXLSReader::_ReadGenDataRow(CXLWorksheet& Worksheet, const int& iRow, TRAVVERTEX& Rec, CStringArray* pErrStrArray) const
{//[Station Name][EMPTY][Inc. Angle][Distance]
	const char chConstMaxCol = 'D';
	const char chConstMinCol = 'A';
	char j;
	CString strMsg, strCell, strTemp;

	for(j = chConstMinCol; j <= chConstMaxCol; j++)
	{
		COleVariant OleVar;
		LPDISPATCH pRange;
		CXLRange Range;

		strCell = MakeCellID(iRow, j);
		VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
		Range.AttachDispatch(pRange);
		OleVar = Range.GetValue();

		{//Update Progress Dialog
			strMsg.Format("Reading Data Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
			CSurvUtilApp::ProgDlgNextStep(strMsg);
		}
		if(j == (chConstMinCol + 0))
		{//Station Name
			if(!_ParseRawXLData(OleVar, &strTemp))
			{
				strMsg.Format("ERROR: Invalid Station Name found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
			if(strlen(strTemp) >= _MAX_PATH)
			{
				return FALSE;
			}
			//Valid station Name
			strcpy(Rec.szStn, (LPCSTR)strTemp);
		}
		else if((chConstMinCol + 1) == j)
		{//Empty Cell
			continue;
		}
		else if((chConstMinCol + 2) == j)
		{//Observed Angle in String
			Rec.fIncAngle = 0.0;
			if(!_ParseRawXLData(OleVar, &strTemp))
			{
				strMsg.Format("ERROR: Invalid angle value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
			if(!CFBSheetData::ParseAngleString("- ", strTemp, Rec.fIncAngle))
			{
				strMsg.Format("ERROR: Invalid angle value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
		}
		else if((chConstMinCol + 3) == j)
		{//Observed Distance as Double
			if(!_ParseRawXLData(OleVar, &(Rec.fDistance)))
			{
				strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}
BOOL COTravXLSReader::_Read(CStringArray* pErrStrArray)
{
	CXLWorksheet Worksheet;
	CString strMsg;
	BOOL bIsAllRowsValid;
	
	if(!_GetWBookSheet(m_strConstSheet, &Worksheet))
	{
		strMsg.Format("ERROR: Sheet \"%s\" not found", (LPCSTR)m_strConstSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}

	int iStart, iEnd;
	if(!_GetDataSegmentLimits(Worksheet, iStart, iEnd, pErrStrArray, m_strConstGenTagStart, m_strConstGenTagEnd))
	{
		strMsg.Format("ERROR: Limit (Start/End Row Identifier) not found in sheet \"%s\"", (LPCSTR)m_strConstSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	bIsAllRowsValid = TRUE;
	{//Read Sheet Data ..[Station][Incl. Angle][Distance][N][E]
		int i;
		TRAVVERTEX Rec;

		for(i = iStart + 1; i < iEnd; i++)
		{
			BOOL bImpResult;
			BOOL bIsRowDataValid;
			
			memset(&Rec, 0, sizeof(TRAVVERTEX));

			bIsRowDataValid = TRUE;
			if(i == (iStart + 1))
			{//for the 1st data line [Station]
				if(!_ReadFirstGPSPoint(Worksheet, i, Rec, pErrStrArray))
				{
					bIsAllRowsValid = FALSE;
					bIsRowDataValid = FALSE;
				}
			}
			else if(i == (iStart + 2))
			{//for the <2nd> data line 
			 //[Station][Bearing][Angle][Distance]
				if(!_ReadSecondGPSPoint(Worksheet, i, Rec, pErrStrArray))
				{
					bIsAllRowsValid = FALSE;
					bIsRowDataValid = FALSE;
				}
			}
			else if(i == (iEnd - 2))
			{//for the <2nd Last> data line third GPS point
			 //[Station][EMPTY][Angle][Distance][N][E]
				if(!_ReadThirdGPSPoint(Worksheet, i, Rec, pErrStrArray))
				{
					bIsAllRowsValid = FALSE;
					bIsRowDataValid = FALSE;
				}
			}
			else if(i == (iEnd - 1))
			{//for the <Last> data line fourth GPS point
			 //[Station][Bearing][EMPTY][Distance]
				if(!_ReadFourthGPSPoint(Worksheet, i, Rec, pErrStrArray))
				{
					bIsAllRowsValid = FALSE;
					bIsRowDataValid = FALSE;
				}
			}
			else
			{//Intermediate Stations [Station Name][EMPTY][Inc. Angle][Distance]
				if(!_ReadGenDataRow(Worksheet, i, Rec, pErrStrArray))
				{
					bIsAllRowsValid = FALSE;
					bIsRowDataValid = FALSE;
				}
			}
			if(bIsRowDataValid)
			{
				bImpResult = m_pData->ImportRec(Rec, (i == (iEnd - 1)));
				if(!bImpResult)
				{
					strMsg.Format("ERROR : Some internal errors occured...Check sheet '%s' Row '%d'", (LPCSTR)m_strConstSheet, i);
					if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				}
				bIsAllRowsValid = bIsAllRowsValid && bImpResult;
			}
		}
	}
	return bIsAllRowsValid;
}

