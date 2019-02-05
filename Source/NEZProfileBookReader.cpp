//NEZProfileBookReader.cpp
//////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "NEZProfileBookReader.h"
#include "SurvUtilApp.h"

//CNEZProfileBookReader
//////////////////////////////////////////////////////////////////////////////////////////
const CString	CNEZProfileBookReader::m_strConstRoadPflChgZSheet = CString("ProfileChainageElev");		//Road Profile Chainage-Z

const CString	CNEZProfileBookReader::m_strConstPflXistTagStart = CString("START EXISTING LEVEL");
const CString	CNEZProfileBookReader::m_strConstPflXistTagEnd = CString("END EXISTING LEVEL");
const CString	CNEZProfileBookReader::m_strConstPflDsgnTagStart = CString("START DESIGN LEVEL");
const CString	CNEZProfileBookReader::m_strConstPflDsgnTagEnd = CString("END DESIGN LEVEL");

const char		CNEZProfileBookReader::m_chConstMaxColChgZPfl = 'B';

CNEZProfileBookReader::CNEZProfileBookReader() : CExcelReaderGen()
{
	m_pRoadProfileData = 0L;
}
CNEZProfileBookReader::~CNEZProfileBookReader()
{
	UnInitialize(TRUE);
}
BOOL CNEZProfileBookReader::ReadAllSheets(CStringArray* pErrStrArray)
{
	CString strMsg;
	BOOL bSuccess;

	if(CSurvUtilApp::IsCancelPressed())
	{
		strMsg.Format("ERROR: User termination !!");
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	if(!FindInStrArray(m_strConstRoadPflChgZSheet, m_SheetNamesArray, FALSE))
	{
		strMsg.Format("ERROR: Sheet \"%s\" not found", (LPCSTR)m_strConstRoadPflChgZSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;//It's must to have this sheet present
	}
	bSuccess = ReadChgZPflSheet(pErrStrArray);
	if(CSurvUtilApp::IsCancelPressed())
	{
		strMsg.Format("ERROR: User termination !!");
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	return bSuccess;
}
//Road Profile
BOOL CNEZProfileBookReader::HasRoadProfileData() const
{
	if(m_pRoadProfileData == 0L)
		return FALSE;
	return m_pRoadProfileData->HasData();
}
void CNEZProfileBookReader::UnInitialize(const BOOL& bResetData /* = FALSE*/)
{
	CExcelReaderGen::UnInitialize(bResetData);
	if(bResetData)
	{
		if(m_pRoadProfileData != 0L)
			delete m_pRoadProfileData;
		
		m_pRoadProfileData = 0L;
	}
}
//
//Distance-Elevation Profile
//Road Profile Existing/Design ProfileData
//
BOOL CNEZProfileBookReader::ReadChgZPflSheet(CStringArray* pErrStrArray)
{
	CXLWorksheet Worksheet;
	BOOL bIsAllRowsValid;
	CString strMsg;

	CSurvUtilApp::ProgDlgNextStep("Reading Profile Data Sheet..", 1);

	if(!_GetWBookSheet(m_strConstRoadPflChgZSheet, &Worksheet))
	{
		strMsg.Format("ERROR: Sheet \"%s\" not found", m_strConstRoadPflChgZSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}

	int iStart, iEnd;
	CSurvUtilApp::ProgDlgNextStep("Finding data segment limits of Profile Data Sheet..");
	if(!_GetDataSegmentLimits(Worksheet, iStart, iEnd, pErrStrArray, m_strConstPflXistTagStart, m_strConstPflXistTagEnd))
	{
		strMsg.Format("ERROR: Limit (Start/End Row ID) not found in sheet \"%s\"", m_strConstRoadPflChgZSheet);
		if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
		return FALSE;
	}
	bIsAllRowsValid = TRUE;
	CSurvUtilApp::ProgDlgNextStep("Reading Profile Data..", 5);
	{//Read Sheet Data ..Profile Data..Xisting Level
		const char	chConstMaxCol = 'E';
		const char	chConstDsgnLevOfFirstOrdCol = 'F';
		int i;
		double	dStartLevOfDsgnPfl;
		ROADPROFILE_CHG_Z	Rec;

		Rec.dChainage = 0.0;
		Rec.dElevXisting = 0.0;
		Rec.dElevDsgn = 0.0;
		Rec.lBit = 0L;
		strcpy(Rec.szDesc, "");
		strcpy(Rec.szBlock, "");

		if(m_pRoadProfileData != 0L)
		{
			delete m_pRoadProfileData;
		}
		m_pRoadProfileData = new CRoadProfile(FALSE);

		for(i = iStart + 1; i < iEnd; i++)
		{
			CString strCell;
			LPDISPATCH pRange;
			CXLRange Range;
			BOOL bIsRowDataValid;
			char j;
			
			bIsRowDataValid = TRUE;
			for(j = m_chConstMinCol; j <= chConstMaxCol; j++)
			{
				CString strTemp;
				COleVariant OleVar;

				strCell = MakeCellID(i, j);
				VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
				Range.AttachDispatch(pRange);
				OleVar = Range.GetValue();
				
				{//Update Progress Dialog
					strTemp.Format("Reading Profile Data Sheet Cell \"%s\"", (LPCSTR)strCell);
					CSurvUtilApp::ProgDlgNextStep(strTemp);
				}
				if((j == chConstMaxCol) || (j == (chConstMaxCol - 1)))
				{//For Block & Text Input
					CString StrDesc;
					BOOL bAcceptBlank;

					//Whether cell can Empty or not .....
					switch(j)
					{
					case (chConstMaxCol - 0)://Block Name Input
						bAcceptBlank = !(Rec.lBit & CSurveyData::BitCode::OnlyBlk);
						break;
					case (chConstMaxCol - 1)://Text Input
						bAcceptBlank = !(Rec.lBit & CSurveyData::BitCode::OnlyTxt);
						break;
					default:
						bAcceptBlank = FALSE;
						break;
					}
					if(!_ParseRawXLData(OleVar, &StrDesc, bAcceptBlank))
					{
						strMsg.Format("ERROR: Invalid Cell value found in Sheet '%s' Cell '%s'", m_strConstRoadPflChgZSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
					if(bAcceptBlank) StrDesc = "!!!INVALID_FIELD!!!";
					switch(j)
					{
					case chConstMaxCol - 0://Block Name Input
						strcpy(Rec.szBlock, StrDesc);
						break;
					case chConstMaxCol - 1://Text Input
						strcpy(Rec.szDesc, StrDesc);
						break;
					default:
						ASSERT(FALSE);
						bIsAllRowsValid = FALSE;
						return FALSE;
					}
				}
				else
				{//For BitCode/Chg/Elev
					double dValue;
					const char	chConstMinCol = 'A';
					const int iMaxBitAllowedCode = CSurveyData::BitCode::OrdLine | CSurveyData::BitCode::OnlyTxt | CSurveyData::BitCode::OnlyBlk | CSurveyData::BitCode::MidPt;
					//Bit Code : 8 = CSurveyData::BitCode::MidPt = Suppress Basic Annotation

					if(!_ParseRawXLData(OleVar, &dValue))
					{
						strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", m_strConstRoadPflChgZSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
					switch(j)
					{
					case (chConstMinCol + 0):
						Rec.lBit = (long)dValue;//BitCode
						if((Rec.lBit < CSurveyData::BitCode::Nothing) || (Rec.lBit > iMaxBitAllowedCode))
						{
							strMsg.Format("ERROR: Invalid Bit Code found in Sheet '%s' Cell '%s'", m_strConstRoadPflChgZSheet, strCell);
							if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
							if(bIsAllRowsValid) bIsAllRowsValid = FALSE;
							bIsRowDataValid = FALSE;
						}
						break;
					case (chConstMinCol + 1):
						Rec.dChainage = dValue;//Chainage
						break;
					case (chConstMinCol + 2):
						Rec.dElevXisting = dValue;//Existing Elev 
						break;
					default:
						ASSERT(FALSE);
						bIsAllRowsValid = FALSE;
						return FALSE;
					}
				}
				if((i == iStart + 1) && (j == chConstMaxCol))
				{//Get the Design Elevation of start chainage of Profile in Col# 'F'
					strCell = MakeCellID(i, chConstDsgnLevOfFirstOrdCol);
					VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
					Range.AttachDispatch(pRange);
					OleVar = Range.GetValue();

					if(!_ParseRawXLData(OleVar, &dStartLevOfDsgnPfl))
					{
						strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'..Expected starting level of Design Profile", m_strConstRoadPflChgZSheet, strCell);
						if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
						bIsAllRowsValid = FALSE;
						bIsRowDataValid = FALSE;
					}
					else
					{
						m_pRoadProfileData->SetStartLevOfDsgnPfl(dStartLevOfDsgnPfl);
					}
				}
			}
			if(bIsRowDataValid)
			{
				if(!m_pRoadProfileData->ImportRec(Rec))
				{
					strMsg.Format("ERROR: Invalid value found!... Pl. Check sheet '%s' Row '%d'", m_strConstRoadPflChgZSheet, i);
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
	{//Read Sheet Data ..Road Profile Data..Dsgn Level
		int i;
		GRADDETAIL	Rec;

		ASSERT(m_pRoadProfileData != 0L);
		Rec.dDist = 0.0;
		Rec.dSlope1inValue = 0.0;

		if(!_GetDataSegmentLimits(Worksheet, iStart, iEnd, pErrStrArray, m_strConstPflDsgnTagStart, m_strConstPflDsgnTagEnd, iEnd + 1))
		{
			strMsg.Format("ERROR: Limit of Design Level Data (Start/End Row ID) not found in sheet \"%s\"", m_strConstRoadPflChgZSheet);
			if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
			return FALSE;
		}
		for(i = iStart + 1; i < iEnd; i++)
		{
			CString strCell;
			LPDISPATCH pRange;
			CXLRange Range;
			BOOL bIsRowDataValid;
			double dValue;
			char j;
			
			bIsRowDataValid = TRUE;
			for(j = m_chConstMinCol; j <= m_chConstMaxColChgZPfl; j++)
			{
				CString strTemp;
				COleVariant OleVar;

				strCell = MakeCellID(i, j);
				
				strTemp.Format("Reading Profile Data Sheet Cell \"%s\"", strCell);
				CSurvUtilApp::ProgDlgNextStep(strTemp);

				VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
				Range.AttachDispatch(pRange);
				OleVar = Range.GetValue();

				if(!_ParseRawXLData(OleVar, &dValue))
				{
					strMsg.Format("ERROR: Invalid value found in Sheet '%s' Cell '%s'", m_strConstRoadPflChgZSheet, strCell);
					if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
					bIsAllRowsValid = FALSE;
					bIsRowDataValid = FALSE;
				}
				else
				{
					const char	chConstMinCol = 'A';
					switch(j)
					{
					case (chConstMinCol + 0):
						Rec.dDist = dValue;
						break;
					case (chConstMinCol + 1):
						{//Convert from '1 in ??' to %ge ....
							Rec.dSlope1inValue = dValue;
						}
						break;
					default:
						ASSERT(FALSE);
						bIsAllRowsValid = FALSE;
						return FALSE;
					}
				}
			}
			if(bIsRowDataValid)
			{
				if(!m_pRoadProfileData->ImportGradRec(Rec))
				{
					strMsg.Format("ERROR: Invalid value found!... Pl. Check sheet '%s' Row '%d'", m_strConstRoadPflChgZSheet, i);
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
