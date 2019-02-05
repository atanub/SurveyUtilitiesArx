//SurveyBookReader.cpp
//////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "SurveyBookReader.h"
#include "SurvUtilApp.h"

//CSurveyBookReader
//////////////////////////////////////////////////////////////////////////////////////////
const CString	CSurveyBookReader::m_strConstYes = CString("yes");
const CString	CSurveyBookReader::m_strConstNo = CString("no");

const CString	CSurveyBookReader::m_strConstMDFSheet = CString("Master");	//Master Data
const CString	CSurveyBookReader::m_strConstRXDSheet = CString("RoadXSecn");//Road X Secn
const CString	CSurveyBookReader::m_strConstPIDSheet = CString("PID");		//Point of Intersection
const CString	CSurveyBookReader::m_strConstPRDSheet = CString("Profile");	//Profile
const CString	CSurveyBookReader::m_strConstPINSheet = CString("InsPoints");//Point Insertion
const CString	CSurveyBookReader::m_strConstXSNSheet = CString("XSecn");	//Cross Sections

const char		CSurveyBookReader::m_chConstPtSubPtSeparator = ',';
const char		CSurveyBookReader::m_chConstMaxColMDF = 'F';
const char		CSurveyBookReader::m_chConstMaxColRXD = 'B';
const char		CSurveyBookReader::m_chConstMaxColMARK = 'B';
const char		CSurveyBookReader::m_chConstMaxColPRD = 'E';

CSurveyBookReader::CSurveyBookReader() : CExcelReaderGen()
{
}
CSurveyBookReader::~CSurveyBookReader()
{
	UnInitialize(TRUE);
}
BOOL CSurveyBookReader::HasMDFData() const
{
	return (m_SurveyData.GetMasterDataRecPtr()->GetSize() > 0);
}
BOOL CSurveyBookReader::HasPIDData() const
{
	return (m_SurveyData.GetPIDataRecPtr()->GetSize() > 0);
}
BOOL CSurveyBookReader::HasPINData() const
{
	return (m_SurveyData.GetPINDataRecPtr()->GetSize() > 0);
}
BOOL CSurveyBookReader::HasPRDData() const
{
	return (m_SurveyData.GetPRDataRecPtr()->GetSize() > 0);
}
BOOL CSurveyBookReader::HasRXDData() const
{
	return (m_SurveyData.GetRXSecDataRecPtr()->GetSize() > 0);
}
BOOL CSurveyBookReader::HasXSNData() const
{
	return (m_SurveyData.GetXSNDataRecPtr()->GetSize() > 0);
}
void CSurveyBookReader::UnInitialize(const BOOL& bResetData /* = FALSE*/)
{
	CExcelReaderGen::UnInitialize(bResetData);
	if(bResetData)
	{
		m_SurveyData.Reset();
	}
}
BOOL CSurveyBookReader::ReadAllSheets(CStringArray* pErrStrArray)
{
	CString strMsg;
	BOOL bSuccess, bTemp;

	if(CSurvUtilApp::IsCancelPressed())
	{
		strMsg.Format("ERROR: User termination !!");
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	if(!FindInStrArray(m_strConstMDFSheet, m_SheetNamesArray, FALSE))
	{
		strMsg.Format("ERROR: Sheet \"%s\" not found", (LPCSTR)m_strConstMDFSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;//It's must to have this sheet present
	}
	bSuccess = ReadMDFSheet(pErrStrArray);
	if(CSurvUtilApp::IsCancelPressed())
	{
		strMsg.Format("ERROR: User termination !!");
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}

	if(FindInStrArray(m_strConstPIDSheet, m_SheetNamesArray, FALSE))
	{
		bTemp = ReadPIDSheet(pErrStrArray);
		bSuccess = bSuccess && bTemp;
	}
	if(CSurvUtilApp::IsCancelPressed())
	{
		strMsg.Format("ERROR: User termination !!");
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	if(FindInStrArray(m_strConstPINSheet, m_SheetNamesArray, FALSE))
	{
		bTemp = ReadPINSheet(pErrStrArray);
		bSuccess = bSuccess && bTemp;
	}
	if(CSurvUtilApp::IsCancelPressed())
	{
		strMsg.Format("ERROR: User termination !!");
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	if(FindInStrArray(m_strConstPRDSheet, m_SheetNamesArray, FALSE))
	{
		bTemp = ReadPRDSheet(pErrStrArray);
		bSuccess = bSuccess && bTemp;
	}
	if(CSurvUtilApp::IsCancelPressed())
	{
		strMsg.Format("ERROR: User termination !!");
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	if(FindInStrArray(m_strConstRXDSheet, m_SheetNamesArray, FALSE))
	{
		bTemp = ReadRXDSheet(pErrStrArray);
		bSuccess = bSuccess && bTemp;
	}
	if(FindInStrArray(m_strConstXSNSheet, m_SheetNamesArray, FALSE))
	{
		bTemp = ReadXSNSheet(pErrStrArray);
		bSuccess = bSuccess && bTemp;
	}
	return bSuccess;
}
//Point of Intersection
BOOL CSurveyBookReader::ReadPIDSheet(CStringArray* pErrStrArray)
{
	CSurvUtilApp::ProgDlgNextStep("Reading Point of Intersection Data Sheet..", 1);
	return _ReadMarkDataSheet(MkSheetType::PID, pErrStrArray);
}
//Point Insertion
BOOL CSurveyBookReader::ReadPINSheet(CStringArray* pErrStrArray)
{
	CSurvUtilApp::ProgDlgNextStep("Reading Insert Points Data Sheet..", 1);
	return _ReadMarkDataSheet(MkSheetType::PIN, pErrStrArray);
}

BOOL CSurveyBookReader::_ReadMarkDataSheet(MkSheetType SheetType, CStringArray* pErrStrArray)
{
	CXLWorksheet Worksheet;
	CString strMsg, strSheetName;
	BOOL bIsAllRowsValid;
	
	if(SheetType == MkSheetType::PIN)
		strSheetName = m_strConstPINSheet;
	else
		strSheetName = m_strConstPIDSheet;

	if(!_GetWBookSheet(strSheetName, &Worksheet))
	{
		strMsg.Format("ERROR: Sheet \"%s\" not found", (LPCSTR)strSheetName);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}

	int iStart, iEnd;
	if(!_GetDataSegmentLimits(Worksheet, iStart, iEnd, pErrStrArray, m_strConstGenTagStart, m_strConstGenTagEnd))
	{
		strMsg.Format("ERROR: Limit (Start/End Row Identifier) not found in sheet \"%s\"", (LPCSTR)strSheetName);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	bIsAllRowsValid = TRUE;
	{//Read Sheet Data ..Mark Data..
		int i;
		MARK Rec;

		for(i = iStart + 1; i < iEnd; i++)
		{
			CString strCell;
			LPDISPATCH pRange;
			CXLRange Range;
			BOOL bImpResult;
			BOOL bIsRowDataValid;
			char j;
			
			bIsRowDataValid = TRUE;
			for(j = m_chConstMinCol; j <= m_chConstMaxColMARK; j++)
			{
				COleVariant OleVar;

				strCell = MakeCellID(i, j);
				VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
				Range.AttachDispatch(pRange);
				OleVar = Range.GetValue();

				{//Update Progress Dialog
					strMsg.Format("Reading Data Sheet '%s' Cell '%s'", (LPCSTR)strSheetName, strCell);
					CSurvUtilApp::ProgDlgNextStep(strMsg);
				}
				if(j == (m_chConstMinCol + 1))
				{//Point Number as String
					CString strTemp;
					if(!_ParseRawXLData(OleVar, &strTemp))
					{
						strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", (LPCSTR)strSheetName, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid)
							bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
					if(!CSurveyData::ParsePointStr(strTemp, Rec.lPtSlNo, Rec.lPtSlNoSub, m_chConstPtSubPtSeparator))
					{
						strMsg.Format("ERROR: Invalid point number found in Sheet '%s' Cell '%s'", (LPCSTR)strSheetName, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid)
							bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
				}
				else
				{//Field Book No. as Double
					double dValue;
					if((m_chConstMinCol + 0) != j)
					{
						ASSERT(FALSE);
						if(bIsAllRowsValid)
							bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
						return FALSE;
					}
					if(!_ParseRawXLData(OleVar, &dValue))
					{
						strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", (LPCSTR)strSheetName, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid)
							bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
					Rec.lFBookID = (long)dValue;
				}
			}
			if(bIsRowDataValid)
			{
				if(SheetType == MkSheetType::PIN)
					bImpResult = m_SurveyData.ImportPINDataRec(&Rec);
				else
					bImpResult = m_SurveyData.ImportPIDataRec(&Rec);

				if(!bImpResult)
				{
					strMsg.Format("ERROR : \"%s\" doesn't exists in Master Data...Check sheet '%s' Row '%d'", (LPCSTR)(CSurveyData::MarkToString(Rec)), (LPCSTR)strSheetName, i);
					if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				}
				bIsAllRowsValid = bIsAllRowsValid && bImpResult;
			}
		}
	}
	return bIsAllRowsValid;
}
//Profile
BOOL CSurveyBookReader::ReadPRDSheet(CStringArray* pErrStrArray)
{
	CXLWorksheet Worksheet;
	CString strMsg;
	BOOL bIsAllRowsValid;

	if(!_GetWBookSheet(m_strConstPRDSheet, &Worksheet))
	{
		strMsg.Format("ERROR: Sheet \"%s\" not found", (LPCSTR)m_strConstPRDSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}

	int iStart, iEnd;
	if(!_GetDataSegmentLimits(Worksheet, iStart, iEnd, pErrStrArray, m_strConstGenTagStart, m_strConstGenTagEnd))
	{
		strMsg.Format("ERROR: Limit (Start/End Row Identifier) not found in sheet \"%s\"", (LPCSTR)m_strConstPRDSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	bIsAllRowsValid = TRUE;
	{//Read Sheet Data ..Profile Data..
		int i;
		PROFILEDATA Rec;

		for(i = iStart + 1; i < iEnd; i++)
		{
			BOOL bIsRowDataValid;
			CString strCell;
			LPDISPATCH pRange;
			CXLRange Range;
			char j;
			
			for(j = m_chConstMinCol; j <= m_chConstMaxColPRD; j++)
			{
				CString strTemp;
				COleVariant OleVar;

				strCell = MakeCellID(i, j);
				VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
				Range.AttachDispatch(pRange);
				OleVar = Range.GetValue();

				{//Update Progress Dialog
					strMsg.Format("Reading Profile Data Sheet Cell '%s'", strCell);
					CSurvUtilApp::ProgDlgNextStep(strMsg);
				}

				if((j == m_chConstMaxColPRD) || (j == (m_chConstMaxColPRD - 1) || (j == (m_chConstMaxColPRD - 2))))
				{
					CString StrDesc;
					BOOL bAcceptBlank;

					//Whether cell can Empty or not .....
					switch(j)
					{
					case (m_chConstMaxColPRD - 0)://Block Name Input
						bAcceptBlank = !(Rec.lBit & CSurveyData::BitCode::OnlyBlk);
						break;
					case (m_chConstMaxColPRD - 1)://Text Input
						bAcceptBlank = !(Rec.lBit & CSurveyData::BitCode::OnlyTxt);
						break;
					default:
						bAcceptBlank = FALSE;
						break;
					}
					if(!_ParseRawXLData(OleVar, &StrDesc, bAcceptBlank))
					{
						strMsg.Format("ERROR: Invalid Cell value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstPRDSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
					if(bAcceptBlank) StrDesc = "!!!INVALID_FIELD!!!";
					switch(j)
					{
					case m_chConstMaxColPRD - 0://Block Name Input
						strcpy(Rec.szBlock, StrDesc);
						break;
					case m_chConstMaxColPRD - 1://Text Input
						strcpy(Rec.szText, StrDesc);
						break;
					case m_chConstMaxColPRD - 2://Point# Input
						{
							if(!CSurveyData::ParsePointStr(StrDesc, Rec.Mark.lPtSlNo, Rec.Mark.lPtSlNoSub, m_chConstPtSubPtSeparator))
							{
								strMsg.Format("ERROR: Invalid point number found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstPRDSheet, strCell);
								if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
								if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
								bIsRowDataValid = FALSE;
							}
						}
						break;
					default:
						ASSERT(FALSE);
						if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
						return FALSE;
					}
				}
				else
				{
					double dValue;
					const char	chConstMinCol = 'A';
					const int iMaxBitAllowedCode = CSurveyData::BitCode::OrdLine | CSurveyData::BitCode::OnlyTxt | CSurveyData::BitCode::OnlyBlk | CSurveyData::BitCode::MidPt;
					//Bit Code : 8 = CSurveyData::BitCode::MidPt = Suppress Basic Annotation
					if(!_ParseRawXLData(OleVar, &dValue))
					{
						strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstPRDSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
					switch(j)
					{
					case (chConstMinCol + 0):
						Rec.lBit = (long)dValue;//BitCode
						if((Rec.lBit < CSurveyData::BitCode::Nothing) || (Rec.lBit > iMaxBitAllowedCode))
						{
							strMsg.Format("ERROR: Invalid Bit Code found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstPRDSheet, strCell);
							if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
							if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
							bIsRowDataValid = FALSE;
						}
						break;
					case (chConstMinCol + 1):
						Rec.Mark.lFBookID = (long)dValue;//FBook #
						break;
					default:
						ASSERT(FALSE);
						if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
						return FALSE;
					}
				}
			}
			if(bIsRowDataValid)
			{
				if(!m_SurveyData.ImportPRDataRec(&Rec))
				{
					strMsg.Format("ERROR: \"%s\" doesn't exists in Master Data...Pl. check sheet '%s' Row '%d'", (LPCSTR)(CSurveyData::MarkToString(Rec.Mark)), (LPCSTR)m_strConstPRDSheet, i);
					if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
					if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
				}
			}
		}
	}
	return bIsAllRowsValid;
}
//RoadXSecn/XSecn 
BOOL CSurveyBookReader::ReadRXDSheet(CStringArray* pErrStrArray)
{
	CXLWorksheet Worksheet;
	CString strMsg;
	BOOL bIsAllRowsValid;
	
	bIsAllRowsValid = TRUE;
	if(!_GetWBookSheet(m_strConstRXDSheet, &Worksheet))
	{
		strMsg.Format("ERROR: Sheet \"%s\" not found", (LPCSTR)m_strConstRXDSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	int iStart, iEnd;
	if(!_GetDataSegmentLimits(Worksheet, iStart, iEnd, pErrStrArray, m_strConstGenTagStart, m_strConstGenTagEnd))
	{
		strMsg.Format("ERROR: Limit (Start/End Row Identifier) not found in sheet \"%s\"", (LPCSTR)m_strConstRXDSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	{//Read Sheet Data ...
		int i, j;
		ROADXSECDATA Rec;

		for(i = iStart + 1, j = 1; i < iEnd; j++)
		{
			int iRXDSegStart, iRXDSegEnd;
			BOOL bIsRowDataValid = TRUE;

			Rec.lID = j;
			if(!_GetNextRXDataSegmentLimit(Worksheet, i, iEnd, iRXDSegStart, iRXDSegEnd))
			{
				if((iEnd - 1) != iRXDSegEnd)
				{
					strMsg.Format("ERROR: Road X Secn. data segment abnormal !!Record..Pl. check sheet '%s' from row '%d' to row '%d' ", (LPCSTR)m_strConstRXDSheet, iRXDSegStart, iEnd);
					if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
					return FALSE;
				}
				return TRUE;//No more segment found
			}
			
			if(!_GetNextRXDataSegment(Worksheet, iRXDSegStart, iRXDSegEnd, Rec.MarkArray, pErrStrArray))
			{
				strMsg.Format("ERROR: Failed to import Road X Secn. Record..Pl. check sheet '%s' from row '%d' to row '%d' ", (LPCSTR)m_strConstRXDSheet, iRXDSegStart, iRXDSegEnd);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
				bIsRowDataValid = FALSE;
			}
			i = iRXDSegEnd + 1;
			if(bIsRowDataValid)
			{
				if(!m_SurveyData.ImportRXDataRec(&Rec))
				{
					strMsg.Format("ERROR: Failed to import Road X Secn. Record..Pl. check sheet '%s' from row '%d' to row '%d' ", (LPCSTR)m_strConstRXDSheet, iRXDSegStart, iRXDSegEnd);
					if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
					bIsAllRowsValid = FALSE;
				}
			}
			DeallocatePtrArray(Rec.MarkArray);
		}
	}
	return bIsAllRowsValid;
}
BOOL CSurveyBookReader::_GetNextRXDataSegment(CXLWorksheet& Worksheet, const int& iStart, const int& iEnd, CMarkRecs& MarkArray, CStringArray* pErrStrArray) const
{
	int i;
	BOOL bIsAllRowsValid;

	VERIFY(MarkArray.GetSize() == 0);
	bIsAllRowsValid = TRUE;
	for(i = iStart + 1; i < iEnd; i++)
	{
		LPDISPATCH pRange;
		CXLRange Range;
		CString strTemp, strCell, strMsg;
		COleVariant OleVar;
		double dValue;
		MARK Rec;
		MARK* pRec;
		BOOL bIsRowDataValid = TRUE;

		//Field Book # ********************************************
		strCell = MakeCellID(i, 'A');
		VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
		Range.AttachDispatch(pRange);
		OleVar = Range.GetValue();
		{//Update Progress Dialog
			strMsg.Format("Reading Road X Secn Data..Verifying Cell '%s'", strCell);
			CSurvUtilApp::ProgDlgNextStep(strMsg);
		}
		if(!_ParseRawXLData(OleVar, &dValue))
		{
			strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstRXDSheet, strCell);
			if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
			if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
			bIsRowDataValid = FALSE;
		}
		else
		{
			Rec.lFBookID = (long)dValue;//BitCode
		}

		//Point# ********************************************
		strCell = MakeCellID(i, 'B');
		VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
		Range.AttachDispatch(pRange);
		OleVar = Range.GetValue();
		{//Update Progress Dialog
			strMsg.Format("Reading Road X Secn Data..Verifying Cell '%s'", strCell);
			CSurvUtilApp::ProgDlgNextStep(strMsg);
		}
		if(!_ParseRawXLData(OleVar, &strTemp))
		{
			strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstRXDSheet, strCell);
			if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
			if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
			bIsRowDataValid = FALSE;
		}
		else
		{
			if(!CSurveyData::ParsePointStr(strTemp, Rec.lPtSlNo, Rec.lPtSlNoSub, m_chConstPtSubPtSeparator))
			{
				strMsg.Format("ERROR: Invalid point number found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstRXDSheet, strCell);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
				bIsRowDataValid = FALSE;
			}
			else
			{
				pRec = new MARK;
				memset(pRec, 0, sizeof(MARK));
				memcpy(pRec, &Rec, sizeof(MARK));
				MarkArray.Add(pRec);
			}
		}
	}
	if(!bIsAllRowsValid)
	{
		DeallocatePtrArray(MarkArray);
	}
	return bIsAllRowsValid;
}
BOOL CSurveyBookReader::_GetNextRXDataSegmentLimit(CXLWorksheet& Worksheet, const int& iSearchFrom, const int& iSearchUpto, int& iStart, int& iEnd) const
{
	const CString strConstRXDSegTagStart("START STATION");
	const CString strConstRXDSegTagEnd("END STATION");
	
	int i;
	iStart = iEnd = 0;
	for(i = iSearchFrom; i <= iSearchUpto; i++)
	{
		CString strCell, strTemp, strMsg;
		LPDISPATCH pRange;
		CXLRange Range;
		COleVariant OleVar;
		CString strCellVal;
		
		strCell = MakeCellID(i, m_chConstMinCol);
		VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
		Range.AttachDispatch(pRange);
		OleVar = Range.GetValue();

		{//Update Progress Dialog
			strMsg.Format("Finding RoadXSecn. Segment Limit..Reading Cell(%s)", strCell);
			CSurvUtilApp::ProgDlgNextStep(strMsg);
		}
		if(_ParseRawXLData(OleVar, &strCellVal))
		{
			if(strCellVal.CompareNoCase(strConstRXDSegTagStart) == 0)
			{
				if(iStart != 0)
					return FALSE;//Already found once
				iStart = i;
			}
			if(strCellVal.CompareNoCase(strConstRXDSegTagEnd) == 0)
			{
				iEnd = i;
				if((iStart >= iEnd) || (iStart == 0))
					return FALSE;
				break;
			}
		}
	}
	if((iStart == 0) || (iEnd == 0))
		return FALSE;

	return TRUE;
}
//X Secn (Elev)
BOOL CSurveyBookReader::ReadXSNSheet(CStringArray*  pErrStrArray)
{
	CXLWorksheet Worksheet;
	CString strMsg;
	int iStart, iEnd;
	BOOL bIsAllDataValid, bReadChainage; 
	
	bIsAllDataValid = TRUE;
	if(!_GetWBookSheet(m_strConstXSNSheet, &Worksheet))
	{
		strMsg.Format("ERROR: Sheet \"%s\" not found", (LPCSTR)m_strConstXSNSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}

	if(!_GetDataSegmentLimits(Worksheet, iStart, iEnd, pErrStrArray, m_strConstGenTagStart, m_strConstGenTagEnd))
	{
		strMsg.Format("ERROR: Limit (Start/End Row Identifier) not found in sheet \"%s\"", m_strConstXSNSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	{//Read Chainage Input type..
		CString strCell, strCellVal, strTemp;
		LPDISPATCH pRange;
		CXLRange Range;
		COleVariant OleVar;

		strCell = MakeCellID(iStart + 1, m_chConstMinCol);
		VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
		Range.AttachDispatch(pRange);
		OleVar = Range.GetValue();

		if(!_ParseRawXLData(OleVar, &strCellVal))
		{
			strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", m_strConstXSNSheet, strCell);
			if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
			return FALSE;
		}
		bReadChainage = ((strCellVal.CompareNoCase(m_strConstYes) == 0) || (strCellVal.CompareNoCase(m_strConstNo) == 0));
		if(!bReadChainage)
		{
			strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", m_strConstXSNSheet, strCell);
			if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
			return FALSE;
		}
		bReadChainage = (strCellVal.CompareNoCase(m_strConstYes) == 0);
	}
	m_SurveyData.SetAutoChainageCalnInXSN(!bReadChainage);
	{//Read Sheet Data ...
		int i, j;
	
		bIsAllDataValid = TRUE;
		for(i = iStart + 1, j = 1; i < iEnd; j++)
		{
			int iDataSegStart, iDataSegEnd;
			XSECDATA Rec;

			Rec.dChainage = 0.0;
			if(!_GetNextRXDataSegmentLimit(Worksheet, i, iEnd, iDataSegStart, iDataSegEnd))
			{
				if((iEnd - 1) != iDataSegEnd)
				{
					strMsg.Format("ERROR: Cross Secn. data segment abnormal !!Record..Pl. check sheet '%s' from row '%d' to row '%d' ", m_strConstXSNSheet, iDataSegStart, iEnd);
					if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
					return FALSE;
				}
				return TRUE;//No more segment found
			}
			else
			{//Read the Chaniage...
				Rec.dChainage = 0.0;
				if(bReadChainage)
				{
					CString strCell, strTemp, strCellVal;
					LPDISPATCH pRange;
					CXLRange Range;
					COleVariant OleVar;

					strCell = MakeCellID(iDataSegStart, (char)(m_chConstMinCol + 1));
					VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
					Range.AttachDispatch(pRange);
					OleVar = Range.GetValue();

					if(!_ParseRawXLData(OleVar, &(Rec.dChainage)))
					{
						strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", m_strConstXSNSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						bIsAllDataValid = FALSE;
					}
				}
			}
			
			if(!_GetNextXSNDataSegment(Worksheet, iDataSegStart, iDataSegEnd, Rec.RecArray, pErrStrArray))
			{
				strMsg.Format("ERROR: Failed to import Cross Secn. Record..Pl. check sheet '%s' from row '%d' to row '%d' ", m_strConstXSNSheet, iDataSegStart, iDataSegEnd);
				if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
				bIsAllDataValid = FALSE;
			}
			i = iDataSegEnd + 1;
			if(bIsAllDataValid)
			{
				if(!m_SurveyData.ImportXSNDataRec(&Rec))
				{
					strMsg.Format("ERROR: Failed to import Cross Secn. Record..Pl. check sheet '%s' from row '%d' to row '%d' ", m_strConstXSNSheet, iDataSegStart, iDataSegEnd);
					if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
					bIsAllDataValid = FALSE;
				}
			}
			DeallocatePtrArray(Rec.RecArray);
		}
	}
	return bIsAllDataValid;
}
BOOL CSurveyBookReader::_GetNextXSNDataSegment(CXLWorksheet& Worksheet, const int& iStart, const int& iEnd, CPRDataRecs& RecArray, CStringArray* pErrStrArray) const
{
	int i;
	PROFILEDATA Rec;
	BOOL bIsAllRowsValid;

	VERIFY(RecArray.GetSize() == 0);
	bIsAllRowsValid = TRUE;

	//Read Sheet Data ..Profile Data..
	for(i = iStart + 1; i < iEnd; i++)
	{
		CString strCell, strMsg;
		LPDISPATCH pRange;
		CXLRange Range;
		char j;
		
		for(j = m_chConstMinCol; j <= m_chConstMaxColPRD; j++)
		{
			CString strTemp;
			COleVariant OleVar;

			strCell = MakeCellID(i, j);
			VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
			Range.AttachDispatch(pRange);
			OleVar = Range.GetValue();

			{//Update Progress Dialog
				strMsg.Format("Reading Cross Section Data Sheet Cell '%s'", strCell);
				CSurvUtilApp::ProgDlgNextStep(strMsg);
			}

			if((j == m_chConstMaxColPRD) || (j == (m_chConstMaxColPRD - 1) || (j == (m_chConstMaxColPRD - 2))))
			{
				CString StrDesc;
				BOOL bAcceptBlank;

				//Whether cell can Empty or not .....
				switch(j)
				{
				case (m_chConstMaxColPRD - 0)://Block Name Input
					bAcceptBlank = !(Rec.lBit & CSurveyData::BitCode::OnlyBlk);
					break;
				case (m_chConstMaxColPRD - 1)://Text Input
					bAcceptBlank = !(Rec.lBit & CSurveyData::BitCode::OnlyTxt);
					break;
				default:
					bAcceptBlank = FALSE;
					break;
				}
				if(!_ParseRawXLData(OleVar, &StrDesc, bAcceptBlank))
				{
					strMsg.Format("ERROR: Invalid Cell value found in Sheet '%s' Cell '%s'", m_strConstXSNSheet, strCell);
					if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
					bIsAllRowsValid = FALSE;
				}
				if(bAcceptBlank) StrDesc = "!!!INVALID_FIELD!!!";
				switch(j)
				{
				case m_chConstMaxColPRD - 0://Block Name Input
					strcpy(Rec.szBlock, StrDesc);
					break;
				case m_chConstMaxColPRD - 1://Text Input
					strcpy(Rec.szText, StrDesc);
					break;
				case m_chConstMaxColPRD - 2://Point# Input
					{
						if(!CSurveyData::ParsePointStr(StrDesc, Rec.Mark.lPtSlNo, Rec.Mark.lPtSlNoSub, m_chConstPtSubPtSeparator))
						{
							strMsg.Format("ERROR: Invalid point number found in Sheet '%s' Cell '%s'", m_strConstXSNSheet, strCell);
							if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
							bIsAllRowsValid = FALSE;
						}
					}
					break;
				default:
					ASSERT(FALSE);
					bIsAllRowsValid = FALSE;
					return FALSE;
				}
			}
			else
			{
				const int iMaxBitAllowedCode = CSurveyData::BitCode::OrdLine | CSurveyData::BitCode::OnlyTxt | CSurveyData::BitCode::OnlyBlk | CSurveyData::BitCode::MidPt;
				double dValue;
				const char	chConstMinCol = 'A';

				if(!_ParseRawXLData(OleVar, &dValue))
				{
					strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", m_strConstXSNSheet, strCell);
					if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
					bIsAllRowsValid = FALSE;
				}
				switch(j)
				{
				case (chConstMinCol + 0):
					Rec.lBit = (long)dValue;//BitCode
					if((Rec.lBit < CSurveyData::BitCode::Nothing) || (Rec.lBit > iMaxBitAllowedCode))
					{
						strMsg.Format("ERROR: Invalid Bit Code found in Sheet '%s' Cell '%s'", m_strConstXSNSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						bIsAllRowsValid = FALSE;
					}
					break;
				case (chConstMinCol + 1):
					Rec.Mark.lFBookID = (long)dValue;//FBook #
					break;
				default:
					ASSERT(FALSE);
					bIsAllRowsValid = FALSE;
					return FALSE;
				}
			}
		}
		if(bIsAllRowsValid)
		{
			PROFILEDATA* pRec;

			pRec = new PROFILEDATA;
			memcpy(pRec, &Rec, sizeof(PROFILEDATA));
			RecArray.Add(pRec);
		}
	}
	if(!bIsAllRowsValid)
	{
		DeallocatePtrArray(RecArray);
	}
	return bIsAllRowsValid;
}

BOOL CSurveyBookReader::ReadMDFSheet(CStringArray* pErrStrArray)
{
	CXLWorksheet Worksheet;
	BOOL bIsAllRowsValid;
	CString strMsg;

	CSurvUtilApp::ProgDlgNextStep("Reading Master Data Sheet..", 1);

	if(!_GetWBookSheet(m_strConstMDFSheet, &Worksheet))
	{
		strMsg.Format("ERROR: Sheet \"%s\" not found", (LPCSTR)m_strConstMDFSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}

	int iStart, iEnd;
	CSurvUtilApp::ProgDlgNextStep("Finding data segment limits of Master Data Sheet..");
	if(!_GetDataSegmentLimits(Worksheet, iStart, iEnd, pErrStrArray, m_strConstGenTagStart, m_strConstGenTagEnd))
	{
		strMsg.Format("ERROR: Limit (Start/End Row ID) not found in sheet \"%s\"", (LPCSTR)m_strConstMDFSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	bIsAllRowsValid = TRUE;
	CSurvUtilApp::ProgDlgNextStep("Reading Master Data..", 5);
	{//Read Sheet Data ..Master Data..
		int i;
		MASTERDATA	Rec;

		for(i = iStart + 1; i < iEnd; i++)
		{
			CString strCell;
			LPDISPATCH pRange;
			CXLRange Range;
			BOOL bIsRowDataValid;
			char j;
			
			bIsRowDataValid = TRUE;
			for(j = m_chConstMinCol; j <= m_chConstMaxColMDF; j++)
			{
				CString strTemp;
				COleVariant OleVar;

				strCell = MakeCellID(i, j);
				
				strTemp.Format("Reading Master Data Sheet Cell \"%s\"", (LPCSTR)strCell);
				CSurvUtilApp::ProgDlgNextStep(strTemp);

				VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
				Range.AttachDispatch(pRange);
				OleVar = Range.GetValue();

				if((j == m_chConstMaxColMDF) || (j == (m_chConstMinCol + 1)))
				{
					if(!_ParseRawXLData(OleVar, &strTemp))
					{
						strMsg.Format("ERROR: Invalid point description found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstMDFSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
					if(j == m_chConstMaxColMDF)
					{
						strcpy(Rec.szDesc, (LPCSTR)strTemp);
						if(!CSurvUtilApp::IsValidACADSymName(strTemp))
						{
							strMsg.Format("ERROR: Invalid description \"%s\" found in sheet '%s' cell: '%s'", (LPCSTR)strTemp, (LPCSTR)m_strConstMDFSheet, strCell);
							if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
							if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
							bIsRowDataValid = FALSE;
						}
					}
					else
					{
						if(!CSurveyData::ParsePointStr(strTemp, Rec.Mark.lPtSlNo, Rec.Mark.lPtSlNoSub, m_chConstPtSubPtSeparator))
						{
							strMsg.Format("ERROR: Invalid point number found in sheet '%s' cell '%s'", (LPCSTR)m_strConstMDFSheet, strCell);
							if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
							if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
							bIsRowDataValid = FALSE;
						}
					}
				}
				else
				{
					double dValue;
					const char	chConstMinCol = 'A';

					if(!_ParseRawXLData(OleVar, &dValue))
					{
						strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", (LPCSTR)m_strConstMDFSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
					else
					{
						switch(j)
						{
						case (chConstMinCol + 0):
							Rec.Mark.lFBookID = (long)dValue;
							break;
						case (chConstMinCol + 2):
							Rec.SPoint.fNorthing = dValue;
							break;
						case (chConstMinCol + 3):
							Rec.SPoint.fEasting = dValue;
							break;
						case (chConstMinCol + 4):
							Rec.SPoint.fElev = dValue;
							break;
						default:
							ASSERT(FALSE);
							if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
							return FALSE;
						}
					}
				}
			}
			if(bIsRowDataValid)
			{
				if(!m_SurveyData.ImportMasterDataRec(&Rec))
				{
					strMsg.Format("ERROR: Pl. Check sheet '%s' Row '%d',\"%s\" is already exists", (LPCSTR)m_strConstMDFSheet, i, (LPCSTR)(CSurveyData::MarkToString(Rec.Mark)));
					if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
					if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
				}
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
