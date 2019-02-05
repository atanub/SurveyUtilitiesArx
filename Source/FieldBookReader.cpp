//FieldBookReader.cpp
//////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FieldBookReader.h"
#include "SurvUtilApp.h"

//CFieldBookReader
//////////////////////////////////////////////////////////////////////////////////////////
const CString	CFieldBookReader::m_strConstAlignmentSheet	= CString("Alignment");				//Field Book Alignment
const CString	CFieldBookReader::m_strConstSurveyObjSheet	= CString("Surveyed Objects");		//Field Book Surveyed Objects
const CString	CFieldBookReader::m_strConstObjSheet		= CString("Objects Definitions");	//Field Book Objects Definitions
const CString	CFieldBookReader::m_strConstScaleSheet		= CString("Scale");					//Field Book Scale
const int		CFieldBookReader::m_iMaxNoOfErrors			= 32;

CFieldBookReader::CFieldBookReader() : CExcelReaderGen()
{
	m_pData = 0L;
}
CFieldBookReader::~CFieldBookReader()
{
	UnInitialize(TRUE);
}
BOOL CFieldBookReader::ReadAllSheets(CStringArray* pErrStrArray)
{
	CString strMsg;
	BOOL bSuccess;

	if(CSurvUtilApp::IsCancelPressed())
	{
		strMsg.Format("ERROR: User termination !!");
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	bSuccess = FindInStrArray(m_strConstAlignmentSheet, m_SheetNamesArray, FALSE);
	bSuccess = bSuccess && FindInStrArray(m_strConstSurveyObjSheet, m_SheetNamesArray, FALSE);
	bSuccess = bSuccess && FindInStrArray(m_strConstObjSheet, m_SheetNamesArray, FALSE);
	bSuccess = bSuccess && FindInStrArray(m_strConstScaleSheet, m_SheetNamesArray, FALSE);

	if(!bSuccess)
	{
		strMsg.Format("ERROR: Invalid Field Book WorkBook\nThe WorkBook should contain all of the following sheets \"%s\", \"%s\", \"%s\"", m_strConstAlignmentSheet, m_strConstSurveyObjSheet, m_strConstObjSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	////////////////////////////////////////////////////
	if(m_pData != 0L)
	{
		delete m_pData;
		m_pData = 0L;
	}
	m_pData = new CFBSheetData();
	////////////////////////////////////////////////////
	if(!_ReadAlignment(pErrStrArray))
	{
		return FALSE;
	}
	if(!_ReadSurveyObjs(pErrStrArray))
	{
		return FALSE;
	}
	if(!_ReadSurveyObjDefs(pErrStrArray))
	{
		return FALSE;
	}
	if(!_ReadScale(pErrStrArray))
	{
		return FALSE;
	}
	return bSuccess;
}
void CFieldBookReader::UnInitialize(const BOOL& bResetData /* = FALSE*/)
{
	CExcelReaderGen::UnInitialize(bResetData);
	if(bResetData)
	{//
		if(m_pData != 0L)
			delete m_pData;

		m_pData = 0L;
	}
}
BOOL CFieldBookReader::_ReadAlignment(CStringArray* pErrStrArray)
{
	const char chConstMaxCol = 'C';
	const char chConstMinCol = 'A';
	CXLWorksheet Worksheet;
	CString strMsg;
	BOOL bIsAllRowsValid;
	
	if(!_GetWBookSheet(m_strConstAlignmentSheet, &Worksheet))
	{
		strMsg.Format("ERROR: Sheet \"%s\" not found", (LPCSTR)m_strConstAlignmentSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}

	int iStart, iEnd;
	if(!_GetDataSegmentLimits(Worksheet, iStart, iEnd, pErrStrArray, m_strConstGenTagStart, m_strConstGenTagEnd))
	{
		strMsg.Format("ERROR: Limit (Start/End Row Identifier) not found in sheet \"%s\"", (LPCSTR)m_strConstAlignmentSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	bIsAllRowsValid = TRUE;
	{//Read Sheet Data ..Angle/Distance..
		int i;
		FBALIGNMENT Rec;

		for(i = iStart + 1; i < iEnd; i++)
		{
			char j;
			CString strCell, strTemp;
			LPDISPATCH pRange;
			CXLRange Range;
			BOOL bImpResult;
			BOOL bIsRowDataValid;
			
			bIsRowDataValid = TRUE;
			for(j = chConstMinCol; j <= chConstMaxCol; j++)
			{
				COleVariant OleVar;

				strCell = MakeCellID(i, j);
				VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
				Range.AttachDispatch(pRange);
				OleVar = Range.GetValue();

				{//Update Progress Dialog
					strMsg.Format("Reading Data Sheet '%s' Cell '%s'", (LPCSTR)m_strConstAlignmentSheet, strCell);
					CSurvUtilApp::ProgDlgNextStep(strMsg);
				}
				if(j == (chConstMinCol + 0))
				{//Angle in String
					Rec.fAngleInDeg = 0.0;
					if(!_ParseRawXLData(OleVar, &strTemp))
					{
						strMsg.Format("ERROR: Invalid angle value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstAlignmentSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid)
							bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
					if(!CFBSheetData::ParseAngleString("' ", strTemp, Rec.fAngleInDeg))
					{
						strMsg.Format("ERROR: Invalid angle value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstAlignmentSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid)
							bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
				}
				else
				{//Start/End chainage as Double
					double dValue;
					if(((m_chConstMinCol + 1) != j) && ((m_chConstMinCol + 2) != j))
					{
						ASSERT(FALSE);
						if(bIsAllRowsValid)
							bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
						return FALSE;
					}
					if(!_ParseRawXLData(OleVar, &dValue))
					{
						strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstAlignmentSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid)
							bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
					if(j == (chConstMinCol + 1))
					{//Start chainage
						Rec.fStartChg = dValue;
						if(Rec.fStartChg < 0.0)
						{
							strMsg.Format("ERROR: Invalid 'Start Chainage' found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstAlignmentSheet, strCell);
							if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
							if(bIsAllRowsValid)
								bIsAllRowsValid = FALSE;
							bIsRowDataValid = FALSE;
						}
					}
					else
					{//End chainage
						ASSERT(j == (chConstMinCol + 2));
						Rec.fEndChg = dValue;
						//Calculate
						Rec.fChainage = (Rec.fEndChg - Rec.fStartChg);
						if(Rec.fEndChg < 0.0)
						{
							strMsg.Format("ERROR: Invalid 'End Chainage' found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstAlignmentSheet, strCell);
							if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
							if(bIsAllRowsValid)
								bIsAllRowsValid = FALSE;
							bIsRowDataValid = FALSE;
						}
						else
						{
							if(Rec.fChainage <= 0.0)
							{
								strMsg.Format("ERROR: Invalid 'End Chainage' found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstAlignmentSheet, strCell);
								if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
								if(bIsAllRowsValid)
									bIsAllRowsValid = FALSE;
								bIsRowDataValid = FALSE;
							}
						}
					}
				}
			}
			if(bIsRowDataValid)
			{
				bImpResult = m_pData->ImportRec(Rec);
				if(!bImpResult)
				{
					strMsg.Format("ERROR : Some internal errors occured...Check sheet '%s' Row '%d'", (LPCSTR)m_strConstAlignmentSheet, i);
					if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				}
				bIsAllRowsValid = bIsAllRowsValid && bImpResult;
			}
			if(CSurvUtilApp::IsCancelPressed())
			{
				strMsg.Format("ERROR: User termination !!");
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
		}
	}
	return bIsAllRowsValid;
}
BOOL CFieldBookReader::_ReadScale(CStringArray* pErrStrArray)
{
	CXLWorksheet Worksheet;
	CString strMsg;
	BOOL bIsAllRowsValid;
	
	if(!_GetWBookSheet(m_strConstScaleSheet, &Worksheet))
	{
		strMsg.Format("ERROR: Sheet \"%s\" not found", (LPCSTR)m_strConstScaleSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}

	int iStart, iEnd;
	if(!_GetDataSegmentLimits(Worksheet, iStart, iEnd, pErrStrArray, m_strConstGenTagStart, m_strConstGenTagEnd))
	{
		strMsg.Format("ERROR: Limit (Start/End Row Identifier) not found in sheet \"%s\"", (LPCSTR)m_strConstScaleSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	const int iLastRowRead = iEnd;
	bIsAllRowsValid = TRUE;
	{//Read Sheet Data ..[Start Chg][End Chg][Scale]
		const char chConstMaxCol = 'C';
		const char chConstMinCol = 'A';
		int i;

		for(i = iStart + 1; i < iEnd; i++)
		{
			FBALGNSCALE Rec;
			CString strCell, strTemp;
			LPDISPATCH pRange;
			CXLRange Range;
			BOOL bImpResult;
			BOOL bIsRowDataValid;
			char j;
			
			bIsRowDataValid = TRUE;
			for(j = chConstMinCol; j <= chConstMaxCol; j++)
			{
				COleVariant OleVar;
				double dValue;

				strCell = MakeCellID(i, j);
				VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
				Range.AttachDispatch(pRange);
				OleVar = Range.GetValue();

				{//Update Progress Dialog
					strMsg.Format("Reading Data Sheet '%s' Cell '%s'", (LPCSTR)m_strConstScaleSheet, strCell);
					CSurvUtilApp::ProgDlgNextStep(strMsg);
				}
				{//Xtract Double value

					if(!_ParseRawXLData(OleVar, &dValue))
					{
						strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstScaleSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid)
							bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
				}
				switch(j)
				{
				case (chConstMinCol + 0)://Start Chg
					Rec.fStartChg = dValue;
					break;
				case (chConstMinCol + 1)://End Chg
					Rec.fEndChg = dValue;
					break;
				case (chConstMinCol + 2)://Scale
					Rec.fScale = dValue;
					break;
				default:
					Rec.fStartChg = 0.0;
					Rec.fEndChg = 0.0;
					Rec.fScale = 0.0;
					break;
				}
			}
			if(bIsRowDataValid)
			{
				bImpResult = m_pData->ImportRec(Rec);
				if(!bImpResult)
				{
					strMsg.Format("ERROR : Some internal errors occured...Check sheet '%s' Row '%d'", (LPCSTR)m_strConstScaleSheet, i);
					if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				}
				bIsAllRowsValid = bIsAllRowsValid && bImpResult;
			}
		}
	}

	if(!_GetDataSegmentLimits(Worksheet, iStart, iEnd, pErrStrArray, m_strConstGenTagStart, m_strConstGenTagEnd, (iLastRowRead + 1)))
	{
		strMsg.Format("ERROR: Limit (Start/End Row Identifier) for 'Scale across alignment' not found in sheet \"%s\"", (LPCSTR)m_strConstScaleSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	{//Read Sheet Data ..[Scale]
		const char chConstMaxCol = 'A';
		const char chConstMinCol = 'A';
		int i;
		double dValue;
		CString strCell;

		for(i = iStart + 1; i < iEnd; i++)
		{
			CString strTemp;
			LPDISPATCH pRange;
			CXLRange Range;
			BOOL bImpResult;
			BOOL bIsRowDataValid;
			char j;
			
			bIsRowDataValid = TRUE;
			for(j = chConstMinCol; j <= chConstMaxCol; j++)
			{
				COleVariant OleVar;

				strCell = MakeCellID(i, j);
				VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
				Range.AttachDispatch(pRange);
				OleVar = Range.GetValue();

				{//Update Progress Dialog
					strMsg.Format("Reading Data Sheet '%s' Cell '%s'", (LPCSTR)m_strConstScaleSheet, strCell);
					CSurvUtilApp::ProgDlgNextStep(strMsg);
				}
				{//Xtract Double value
					if(!_ParseRawXLData(OleVar, &dValue))
					{
						strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstScaleSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid)
							bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
				}
			}
			if(bIsRowDataValid)
			{
				bImpResult = m_pData->SetFBScaleAcrossAlgn(dValue);
				if(!bImpResult)
				{
					strMsg.Format("ERROR : Some internal errors occured...Check sheet '%s' Row '%d'", (LPCSTR)m_strConstScaleSheet, i);
					if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				}
				bIsAllRowsValid = bIsAllRowsValid && bImpResult;
			}
		}
		if((iEnd - iStart) > 2)
		{//More than one Scale across the alignment specified
			strMsg.Format("Warning: Multiple scales found for across the alignment data\rOnly the scale (%.3f) provided in the last cell(%s:%s) to be taken into account", dValue, m_strConstScaleSheet, strCell);
			if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		}
	}
	return bIsAllRowsValid;
}
BOOL CFieldBookReader::_ReadSurveyObjs(CStringArray* pErrStrArray)
{
	//BitCode	
	//Bit	Description
	//1	Nothing
	//2	Draw Text
	//4	Insert Block

	const char chConstMaxCol = 'H';
	const char chConstMinCol = 'A';
	CXLWorksheet Worksheet;
	CString strMsg;
	BOOL bIsAllRowsValid;
	
	if(!_GetWBookSheet(m_strConstSurveyObjSheet, &Worksheet))
	{
		strMsg.Format("ERROR: Sheet \"%s\" not found", (LPCSTR)m_strConstSurveyObjSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}

	int iStart, iEnd;
	if(!_GetDataSegmentLimits(Worksheet, iStart, iEnd, pErrStrArray, m_strConstGenTagStart, m_strConstGenTagEnd))
	{
		strMsg.Format("ERROR: Limit (Start/End Row Identifier) not found in sheet \"%s\"", (LPCSTR)m_strConstSurveyObjSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	bIsAllRowsValid = TRUE;
	{//Read Sheet Data ....
		int i;
		FBSURVEYOBJ Rec;

		for(i = iStart + 1; i < iEnd; i++)
		{
			CString strCell;
			LPDISPATCH pRange;
			CXLRange Range;
			BOOL bImpResult;
			BOOL bIsRowDataValid;
			char j;
			
			bIsRowDataValid = TRUE;
			for(j = chConstMinCol; j <= chConstMaxCol; j++)
			{
				COleVariant OleVar;
				CString strTemp;
				double dValue;

				strCell = MakeCellID(i, j);
				VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
				Range.AttachDispatch(pRange);
				OleVar = Range.GetValue();

				{//Update Progress Dialog
					strMsg.Format("Reading Data Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSurveyObjSheet, strCell);
					CSurvUtilApp::ProgDlgNextStep(strMsg);
				}
				switch(j)
				{
				case chConstMinCol+0://Point#
				case chConstMinCol+1://BitCode
				case chConstMinCol+2://Chg
				case chConstMinCol+3://Offset
				case chConstMinCol+4://RL
					if(!_ParseRawXLData(OleVar, &dValue))
					{
						strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", m_strConstSurveyObjSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
					if(j == chConstMinCol+0)
					{//Point#
						Rec.iPointNo = (long)dValue;
					}
					else if(j == chConstMinCol+1)
					{//BitCode
						Rec.iBitCode = (long)dValue;
						if((Rec.iBitCode < 1) || (Rec.iBitCode > 7))
						{
							strMsg.Format("ERROR: Invalid Bit Code found in Sheet '%s' Cell '%s'", m_strConstSurveyObjSheet, strCell);
							if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
							if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
							bIsRowDataValid = FALSE;
						}
					}
					else if(j == chConstMinCol+2)
					{//Chg
						Rec.fChainage = dValue;//Chg
						if(Rec.fChainage < 0.0)
						{
							strMsg.Format("ERROR: Invalid chainage value found in Sheet '%s' Cell '%s'", m_strConstSurveyObjSheet, strCell);
							if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
							if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
							bIsRowDataValid = FALSE;
						}
					}
					else if(j == chConstMinCol+3)
					{//Offset
						Rec.fOffset = dValue;//Offset
						//if(Rec.fOffset == 0.0)
						//{
						//	strMsg.Format("ERROR: Invalid Offset value found in Sheet '%s' Cell '%s'", m_strConstSurveyObjSheet, strCell);
						//	if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						//	if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
						//	bIsRowDataValid = FALSE;
						//}
					}
					else if(j == chConstMinCol+4)
					{//Elev
						Rec.fElev = dValue;//RL
					}
					break;
				case chConstMinCol+5://Lyr
					strcpy(Rec.szLyr, "");
					if(!_ParseRawXLData(OleVar, &strTemp))
					{
						strMsg.Format("ERROR: Invalid layer name found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSurveyObjSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid)
							bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
					if((j == chConstMinCol+5) || (j == chConstMinCol+5))
					{
						if(!CSurvUtilApp::IsValidACADSymName(strTemp))
						{
							strMsg.Format("ERROR: Invalid layer name \"%s\" found in sheet '%s' cell: '%s'", (LPCSTR)strTemp, (LPCSTR)m_strConstSurveyObjSheet, strCell);
							if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
							if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
							bIsRowDataValid = FALSE;
						}
						else
						{
							strcpy(Rec.szLyr, (LPCSTR)strTemp);
						}
					}
					break;
				case chConstMinCol+6://Text
					strcpy(Rec.szText, "");
					if(Rec.iBitCode & 2)
					{
						if(!_ParseRawXLData(OleVar, &strTemp))
						{
							strMsg.Format("ERROR: Invalid text found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSurveyObjSheet, strCell);
							if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
							if(bIsAllRowsValid)
								bIsAllRowsValid = FALSE;
							bIsRowDataValid = FALSE;
						}
						else
						{
							strcpy(Rec.szText, (LPCSTR)strTemp);
						}
					}
					break;
				case chConstMinCol+7://Block
					strcpy(Rec.szBlock, "");
					if(Rec.iBitCode & 4)
					{
						if(!_ParseRawXLData(OleVar, &strTemp))
						{
							strMsg.Format("ERROR: Invalid Block name found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstSurveyObjSheet, strCell);
							if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
							if(bIsAllRowsValid)
								bIsAllRowsValid = FALSE;
							bIsRowDataValid = FALSE;
						}
						else
						{
							strcpy(Rec.szBlock, (LPCSTR)strTemp);
						}
					}
					break;
				default:
					break;
				}
			}
			if(bIsRowDataValid)
			{
				bImpResult = m_pData->ImportRec(Rec);
				if(!bImpResult)
				{
					strMsg.Format("ERROR : Chainages are not in ascending order...Check sheet '%s' Row '%d'", (LPCSTR)m_strConstSurveyObjSheet, i);
					if(pErrStrArray != 0L)
					{
						pErrStrArray->Add(strMsg);
						if(pErrStrArray->GetSize() >= m_iMaxNoOfErrors)
						{
							strMsg.Format("Maximum number (%d) of errors encountered while reading sheet '%s' Row '%d'", m_iMaxNoOfErrors, (LPCSTR)m_strConstSurveyObjSheet, i);
							pErrStrArray->Add(strMsg);
							return FALSE;
						}
					}
				}
				bIsAllRowsValid = bIsAllRowsValid && bImpResult;
			}
			if(CSurvUtilApp::IsCancelPressed())
			{
				strMsg.Format("ERROR: User termination !!");
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				return FALSE;
			}
		}
	}
	return bIsAllRowsValid;
}
BOOL CFieldBookReader::_ReadSurveyObjDefs(CStringArray* pErrStrArray)
{
	const char chConstMaxCol = 'B';
	const char chConstMinCol = 'A';
	const char* szValidEntityList[] = {"LINE", "PLINE"};
	CXLWorksheet Worksheet;
	CString strMsg;
	BOOL bIsAllRowsValid;
	
	if(!_GetWBookSheet(m_strConstObjSheet, &Worksheet))
	{
		strMsg.Format("ERROR: Sheet \"%s\" not found", (LPCSTR)m_strConstObjSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}

	int iStart, iEnd;
	if(!_GetDataSegmentLimits(Worksheet, iStart, iEnd, pErrStrArray, m_strConstGenTagStart, m_strConstGenTagEnd))
	{
		strMsg.Format("ERROR: Limit (Start/End Row Identifier) not found in sheet \"%s\"", (LPCSTR)m_strConstObjSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	bIsAllRowsValid = TRUE;
	{//Read Sheet Data ..Angle/Distance..
		int i;
		FBOBJDEF Rec;

		for(i = iStart + 1; i < iEnd; i++)
		{
			CString strCell;
			LPDISPATCH pRange;
			CXLRange Range;
			BOOL bImpResult;
			BOOL bIsRowDataValid;
			char j;
			
			bIsRowDataValid = TRUE;
			for(j = chConstMinCol; j <= chConstMaxCol; j++)
			{
				COleVariant OleVar;

				strCell = MakeCellID(i, j);
				VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
				Range.AttachDispatch(pRange);
				OleVar = Range.GetValue();

				{//Update Progress Dialog
					strMsg.Format("Reading Data Sheet '%s' Cell '%s'", (LPCSTR)m_strConstObjSheet, strCell);
					CSurvUtilApp::ProgDlgNextStep(strMsg);
				}
				if(j == (chConstMinCol + 0))
				{//Entity name in String
					CString strTemp;

					strcpy(Rec.szEntity, "");
					if(!_ParseRawXLData(OleVar, &strTemp))
					{
						strMsg.Format("ERROR: Invalid Entity name found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstObjSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid)
							bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
					if((strTemp.CompareNoCase(szValidEntityList[0]) == 0) || (strTemp.CompareNoCase(szValidEntityList[1]) == 0))
					{
						strcpy(Rec.szEntity, (LPCSTR)strTemp);
					}
				}
				else
				{//String....Point #s separated with comma
					CString strTemp;
					const CString strWhite(" \t,");

					if((m_chConstMinCol + 1) != j)
					{
						ASSERT(FALSE);
						if(bIsAllRowsValid)
							bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
						return FALSE;
					}
					if(!_ParseRawXLData(OleVar, &strTemp))
					{
						strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstObjSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid)
							bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
					if(Rec.arrayPtNos.GetSize() > 0)
						Rec.arrayPtNos.RemoveAll();

					if(!CFBSheetData::GetPointNosFromString(strWhite, strTemp, Rec.arrayPtNos))
					{
						strMsg.Format("ERROR: Invalid Point numbers found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstObjSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid)
							bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
					if(!m_pData->IsAllPointNosExists(Rec.arrayPtNos))
					{
						strMsg.Format("ERROR: Invalid Point numbers found in Sheet '%s' Cell '%s'\nNot all the points specified is defined !", (LPCSTR)m_strConstObjSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid)
							bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
				}
				if(CSurvUtilApp::IsCancelPressed())
				{
					strMsg.Format("ERROR: User termination !!");
					if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
					return FALSE;
				}
			}
			if(bIsRowDataValid)
			{
				bImpResult = m_pData->ImportRec(Rec);
				if(!bImpResult)
				{
					strMsg.Format("ERROR : Some internal errors occured...Check sheet '%s' Row '%d'", (LPCSTR)m_strConstObjSheet, i);
					if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				}
				bIsAllRowsValid = bIsAllRowsValid && bImpResult;
			}
		}
	}
	return bIsAllRowsValid;
}

