//ExcelReaderGen.cpp
//////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ExcelReaderGen.h"

#include "SurvUtilApp.h"

//CExcelReaderGen
//////////////////////////////////////////////////////////////////////////////////////////
const CString	CExcelReaderGen::m_strConstXLProgID = CString("Excel.Application");

const CString	CExcelReaderGen::m_strConstGenTagStart = CString("START");
const CString	CExcelReaderGen::m_strConstGenTagEnd = CString("END");

const int		CExcelReaderGen::m_iConstMaxRow = 65536;
const int		CExcelReaderGen::m_iConstMinRow = 1;
const char		CExcelReaderGen::m_chConstMinCol = 'A';

CExcelReaderGen::CExcelReaderGen()
{
	m_bIsAppLaunched = FALSE;
	m_bIsWBAlreadyOpened = FALSE;
}
CExcelReaderGen::~CExcelReaderGen()
{
	UnInitialize(TRUE);
}

CString CExcelReaderGen::MakeCellID(const int& iRow, const char& chCol)
{
	CString strCell;
	
	VERIFY((chCol <= 'Z') &&  (chCol >= 'A'));

	strCell.Format("%c%d", chCol, iRow);
	return strCell;
}
BOOL CExcelReaderGen::FindInStrArray(const CString& str, const CStringArray& strArray, const BOOL& bCaseSensitive /*= TRUE*/)
{
	int i, iLim;
	iLim = strArray.GetSize();
	for(i = 0; i < iLim; i++)
	{
		if(bCaseSensitive)
		{
			if(str.CompareNoCase(strArray[i]) == 0)
				return TRUE;
		}
		else
		{
			if(str.Compare(strArray[i]) == 0)
				return TRUE;
		}
	}
	return FALSE;
}

BOOL CExcelReaderGen::Open(const CString& strFile, CStringArray& strArrayErrs)
{
	VERIFY(m_XLApp.m_lpDispatch != 0L);//Initialize not called....
	VERIFY(m_XLWbksArray.m_lpDispatch == 0L);
	
	m_strWBName = strFile;
	{//Get opened WBooks.....
		LPDISPATCH pDispatch;
		
		VERIFY(pDispatch = m_XLApp.GetWorkbooks());
		m_XLWbksArray.AttachDispatch(pDispatch);
	}
	{//Open the file if it's not already opened
		int i, iCount;
		BOOL bIsOpened;

		iCount = m_XLWbksArray.GetCount();
		bIsOpened = FALSE;
		for(i = 1; i <= iCount; i++) 
		{
			LPDISPATCH pDispatch;
			
			pDispatch = m_XLWbksArray.GetItem(COleVariant((short)i));
			m_XLWbkCur.AttachDispatch(pDispatch);
			if(m_XLWbkCur.GetFullName().CompareNoCase(strFile) == 0)
			{//File (Wbk) is Already opened ...
				m_bIsWBAlreadyOpened = TRUE;
				bIsOpened = TRUE;
				break;
			}
			else
			{
				m_XLWbkCur.ReleaseDispatch();
			}
		}
		if(bIsOpened == FALSE)
		{
			LPDISPATCH pDispatch;
			TRY
			{
				pDispatch = m_XLWbksArray.Open((LPCSTR)strFile);
			}
			CATCH_ALL(e)
			{
				m_XLWbksArray.ReleaseDispatch();
				{//
					CString strTmp;
					char szErr[512];
					
					e->GetErrorMessage(szErr, 512);
					strArrayErrs.Add(CString("ERROR: ") + CString(szErr));
				}
				return FALSE;//Error openning file
			}
			END_CATCH_ALL

			if(pDispatch == 0L)
				return FALSE;//Error openning file
			m_bIsWBAlreadyOpened = FALSE;
			m_XLWbkCur.AttachDispatch(pDispatch);
		}
		{//Enquire Sheet names & store them in CStringArray member var
			LPDISPATCH pDispatch;
			int i, iCount;
			CXLWorksheet Worksheet;

			pDispatch = m_XLWbkCur.GetSheets();
			m_XLWSheetArrayCur.AttachDispatch(pDispatch);
			
			iCount = m_XLWSheetArrayCur.GetCount();
			for(i = 1; i <= iCount; i++) 
			{
				pDispatch = m_XLWSheetArrayCur.GetItem(COleVariant((short)i));
				Worksheet.AttachDispatch(pDispatch);
#ifdef _DEBUG //***************
				{//Debugging 
					ads_printf("\nSheet #%d, Name \"%s\"", i, (LPCSTR)(Worksheet.GetName()));
				}
#endif //_DEBUG ***************
				m_SheetNamesArray.Add(Worksheet.GetName());
				Worksheet.ReleaseDispatch();
			}
		}
	}
	return TRUE;
}
void CExcelReaderGen::Close()
{
	if(!m_bIsWBAlreadyOpened)
	{//Close the WBook if it's not already opened by user
		int i, iCount;

		iCount = m_XLWbksArray.GetCount();
		for(i = 1; i <= iCount; i++) 
		{
			LPDISPATCH pDispatch;
			CXLWkbook XLWbk;
			
			pDispatch = m_XLWbksArray.GetItem(COleVariant((short)i));
			XLWbk.AttachDispatch(pDispatch);
			XLWbk.SetSaved(TRUE);
			
			if(XLWbk.GetFullName().CompareNoCase(m_strWBName) == 0)
			{
				if(!m_bIsWBAlreadyOpened)
				{
					XLWbk.Save();
					XLWbk.Close(COleVariant((short)1), COleVariant((char*)0L), COleVariant((short)0));
				}
			}
			XLWbk.ReleaseDispatch();
		}
	}
	m_XLWbksArray.ReleaseDispatch();
	m_XLWbkCur.ReleaseDispatch();
	m_XLWSheetArrayCur.ReleaseDispatch();
	m_SheetNamesArray.RemoveAll();
}
BOOL CExcelReaderGen::Initialize()
{
	BOOL bVisibleFlag;
	LPDISPATCH pDisp;
	LPUNKNOWN pUnk;
	CLSID clsid;

	if(m_XLApp.m_lpDispatch != 0L)
		return FALSE;//Already initialized

	{//If Excel is already running, attach to it, otherwise start it
		if(::CLSIDFromProgID(L"Excel.Application", &clsid) != S_OK)
		{
			AfxMessageBox("Error: Failed to fetch CLSID", MB_ICONSTOP);
			return FALSE;
		}
		if(::GetActiveObject(clsid, NULL, &pUnk) == S_OK) 
		{
			VERIFY(pUnk->QueryInterface(IID_IDispatch, (void**) &pDisp) == S_OK);
			m_XLApp.AttachDispatch(pDisp);
			pUnk->Release();
			m_bIsAppLaunched = FALSE;
		} 
		else 
		{
			if(!m_XLApp.CreateDispatch(m_strConstXLProgID))
			{
				AfxMessageBox("Error: Failed to run Excel", MB_ICONSTOP);
				return FALSE;
			}
			m_bIsAppLaunched = TRUE;
			bVisibleFlag = FALSE;
			m_XLApp.SetVisible(bVisibleFlag);//
		}
		return TRUE;
	}
	ASSERT(FALSE);//Control should never come here
	return FALSE;
}
void CExcelReaderGen::UnInitialize(const BOOL& bResetData /* = FALSE*/)
{
	if(m_XLApp.m_lpDispatch != 0L)
	{
		if(m_SheetNamesArray.GetSize() > 0)
			Close();

		if(m_bIsAppLaunched)
			m_XLApp.Quit();
		m_XLApp.ReleaseDispatch();
	}
	if(bResetData)
	{//Should be implemented by subclass
	}
}
BOOL CExcelReaderGen::_GetWBookSheet(const CString& strSheetID, CXLWorksheet* pWorksheet)
{
	VERIFY(pWorksheet != 0L);
	VERIFY(pWorksheet->m_lpDispatch == 0L);

	if(!FindInStrArray(strSheetID, m_SheetNamesArray, FALSE))
		return FALSE;

	{//Get the correct Sheet....
		LPDISPATCH pDispatch;
		int i, iCount;
		
		iCount = m_XLWSheetArrayCur.GetCount();
		for(i = 1; i <= iCount; i++) 
		{
			pDispatch = m_XLWSheetArrayCur.GetItem(COleVariant((short)i));
			pWorksheet->AttachDispatch(pDispatch);

			if(pWorksheet->GetName().CompareNoCase(strSheetID) == 0)
				return TRUE;
			pWorksheet->DetachDispatch();
		}
	}
	return FALSE;
}

BOOL CExcelReaderGen::_ParseRawXLData(const COleVariant& OleVar, double* pdVal) const
{
	if(OleVar.vt != VT_R8)
	{
		_DumpOleVarType(OleVar);
		return FALSE;
	}
	//Contains double value
	*pdVal = OleVar.dblVal;
	return TRUE;
}
BOOL CExcelReaderGen::_ParseRawXLData(const COleVariant& OleVar, CString* pStr, const BOOL& bAcceptEmpty /*= FALSE*/) const
{
	if(bAcceptEmpty && (OleVar.vt == VT_EMPTY))
	{
		*pStr = "";
		return TRUE;
	}
	if(OleVar.vt != VT_BSTR)
	{
		_DumpOleVarType(OleVar);
		if(bAcceptEmpty)
		{
			*pStr = "";
			return TRUE;
		}
		return FALSE;
	}
	{//Contains String
		CString strCellVal(V_BSTR(&OleVar));
		
		strCellVal.TrimLeft();
		strCellVal.TrimRight();
		*pStr = strCellVal;
	}
	return TRUE;
}
BOOL CExcelReaderGen::_GetDataSegmentLimits(CXLWorksheet& Worksheet, int& iStart, int& iEnd, CStringArray* pErrStrArray, const CString& strConstTagStart, const CString& strConstTagEnd, const int& iStartRow /*= -1*/) const
{
	int i;
	iStart = iEnd = 0;
	
	if(iStartRow > 0)
	{
		i = iStartRow;
	}
	else
	{
		i = m_iConstMinRow;
	}
	for(; i <= m_iConstMaxRow; i++)
	{
		CString strCell, strTemp;
		LPDISPATCH pRange;
		CXLRange Range;
		COleVariant OleVar;
		CString strCellVal;
		
		strCell = MakeCellID(i, (int)m_chConstMinCol);
		VERIFY(pRange = Worksheet.GetRange(COleVariant(strCell), COleVariant(strCell)));
		Range.AttachDispatch(pRange);
		OleVar = Range.GetValue();

		if(CSurvUtilApp::IsCancelPressed())
		{
			const CString strMsg("ERROR: User termination !!");
			if(pErrStrArray != 0L) pErrStrArray->Add(strMsg);
			return FALSE;
		}

		if(_ParseRawXLData(OleVar, &strCellVal))
		{
			if(strCellVal.CompareNoCase(strConstTagStart) == 0)
			{
				if(iStart != 0)
					return FALSE;//Already found once
				iStart = i;
			}
			if(strCellVal.CompareNoCase(strConstTagEnd) == 0)
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
void CExcelReaderGen::_DumpOleVarType(const COleVariant& OleVar) const
{
	return;
#ifdef _DEBUG
	switch(OleVar.vt)
	{
	case VT_EMPTY:	ads_printf("\nVT_EMPTY"); break;
	case VT_NULL:	ads_printf("\nVT_NULL\n"); break;
	case VT_I2:		ads_printf("\nVT_I2\n"); break;
	case VT_I4:		ads_printf("\nVT_I4\n"); break;
	case VT_R4:		ads_printf("\nVT_R4\n"); break;
	case VT_R8:		ads_printf("\nVT_R8\n"); break;
	case VT_DATE:	ads_printf("\nVT_DATE\n"); break;
	case VT_BSTR:	ads_printf("\nVT_BSTR\n"); break;
	case VT_DISPATCH:	ads_printf("\nVT_DISPATCH\n"); break;
	case VT_ERROR:		ads_printf("\nVT_ERROR\n"); break;
	case VT_BOOL:		ads_printf("\nVT_BOOL\n"); break;
	case VT_VARIANT:	ads_printf("\nVT_VARIANT\n"); break;
	case VT_UNKNOWN:	ads_printf("\nVT_UNKNOWN\n"); break;
	case VT_DECIMAL:	ads_printf("\nVT_DECIMAL\n"); break;
	case VT_I1:			ads_printf("\nVT_I1\n"); break;
	case VT_UI1:		ads_printf("\nVT_UI1\n"); break;
	case VT_UI2:		ads_printf("\nVT_UI2\n"); break;
	case VT_UI4:		ads_printf("\nVT_UI4\n"); break;
	case VT_I8:			ads_printf("\nVT_I8\n"); break;
	case VT_UI8:		ads_printf("\nVT_UI8\n"); break;
	case VT_INT:		ads_printf("\nVT_INT\n"); break;
	case VT_UINT:		ads_printf("\nVT_UINT\n"); break;
	case VT_VOID:		ads_printf("\nVT_VOID\n"); break;
	case VT_HRESULT:	ads_printf("\nVT_HRESULT\n"); break;
	case VT_PTR:			ads_printf("\nVT_PTR\n"); break;
	case VT_SAFEARRAY:		ads_printf("\nVT_SAFEARRAY\n"); break;
	case VT_CARRAY:			ads_printf("\nVT_CARRAY\n"); break;
	case VT_USERDEFINED:	ads_printf("\nVT_USERDEFINED\n"); break;
	case VT_LPSTR:			ads_printf("\nVT_LPSTR\n"); break;
	case VT_LPWSTR:			ads_printf("\nVT_LPWSTR\n"); break;
	case VT_FILETIME:		ads_printf("\nVT_FILETIME\n"); break;
	case VT_BLOB:			ads_printf("\nVT_BLOB\n"); break;
	case VT_STREAM:			ads_printf("\nVT_STREAM\n"); break;
	case VT_STORAGE:		ads_printf("\nVT_STORAGE\n"); break;
	case VT_STREAMED_OBJECT: ads_printf("\nVT_STREAMED_OBJECT\n"); break;
	case VT_STORED_OBJECT:	ads_printf("\nVT_STORED_OBJECT\n"); break;
	case VT_BLOB_OBJECT:	ads_printf("\nVT_BLOB_OBJECT\n"); break;
	case VT_CF:				ads_printf("\nVT_CF\n"); break;
	case VT_CLSID:			ads_printf("\nVT_CLSID\n"); break;
	case VT_VECTOR:			ads_printf("\nVT_VECTOR\n"); break;
	case VT_ARRAY:			ads_printf("\nVT_ARRAY\n"); break;
	case VT_BYREF:			ads_printf("\nVT_BYREF\n"); break;
	case VT_RESERVED:		ads_printf("\nVT_RESERVED\n"); break;
	case VT_ILLEGAL:		ads_printf("\nVT_ILLEGAL\n"); break;
	case VT_ILLEGALMASKED:	ads_printf("\nVT_ILLEGALMASKED\n"); break;
	default: break;
	}
#endif //_DEBUG
}
