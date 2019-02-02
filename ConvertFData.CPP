// ConvertFData.cpp : implementation file
//
#include "stdafx.h"
#include "Util.h"
#include "ConvertFData.h"
#include "ResultDlg.h"


static int		_GetCode(const CString&);
static CString	_XTractDesc(const CString&);
static BOOL		_XTractWords(const CString&, CStringArray&);
static BOOL		_XTractFBookData(const CString&, CFBookDataRecs&, CStringArray&);
static void		_WriteData(CStdioFile&, const CFBookDataRecs&);

BOOL ConvertRawFData(const CString& strInpFile, const CString& strOutFile)
{
	CStdioFile	FileInp, FileOut;
	CString strInpData;
	BOOL bFlag;
	
	bFlag = FileInp.Open(strInpFile, CFile::modeRead|CFile::typeText);
	if(!bFlag)
	{
		ads_printf("\nERROR: Unable to open input file %s", (LPCSTR)strInpFile);
		return FALSE;
	}
	
	bFlag = FileOut.Open(strOutFile, CFile::modeCreate|CFile::modeWrite|CFile::typeText);
	if(!bFlag)
	{
		ads_printf("\nERROR: Unable to open output file %s", (LPCSTR)strOutFile);
		return FALSE;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	CStringArray strArray;
	int i;
	CString strDataLine;
	
	do
	{
		CString StringTmp;
		bFlag = FileInp.ReadString(StringTmp);
		if(!bFlag)
		{
			if((_GetCode(strDataLine) == 11) && !strDataLine.IsEmpty())
				strArray.Add(strDataLine);
			break;
		}
		
		StringTmp.TrimLeft();
		StringTmp.TrimRight();
		if(StringTmp.IsEmpty() == FALSE)
		{
			if(_GetCode(StringTmp) == 11)
			{
				if(!strDataLine.IsEmpty())
					strArray.Add(strDataLine);
				strDataLine = StringTmp;
			}
			else
			{
				strDataLine = strDataLine + CString(" ") + StringTmp;
			}
		}
	}
	while(bFlag);

	//Write data
	CStringArray strArrayErrs;
	CString strToWrite;
	
	strToWrite.Format("%7s\t%17s\t%17s\t%17s\t%12s\t%12s\n", "Point#", "Northing", "Easting", "Elevation", "Layer", "Description");
	FileOut.WriteString(strToWrite);
	for(i = 0; i < strArray.GetSize(); i++)
	{
		CFBookDataRecs RecArray;
		
		//TRACE("\n<%s>", (LPCSTR)strArray[i]);
		_XTractFBookData(strArray[i], RecArray, strArrayErrs);//Don't break if err occured
		_WriteData(FileOut, RecArray);
		
		DeallocatePtrArray(RecArray);
	}
	FileInp.Close();
	FileOut.Close();
	if(strArrayErrs.GetSize() > 0)
	{
		if(AfxMessageBox("Error encountered while converting raw data...\nWould you like to view errors ?", MB_ICONINFORMATION|MB_YESNO) == IDYES)
		{
			ShowResultDlg(strArrayErrs);
		}
		return FALSE;
	}
	return TRUE;
}
static int _GetCode(const CString& strWord)
{
	ASSERT(strlen((LPCSTR)strWord) >= 2);
	CString strTmp;
	int iCode;

	strTmp = strWord.Mid(0, 2);
	iCode = atoi((LPCSTR)strTmp);

	//TRACE("\n_GetCode>> %s>>%d", (LPCSTR)strWord, iCode);
	return iCode;
}
static CString _XTractDesc(const CString& strWord)
{
	CString strDesc;
	int i, iLim, iCode, iIndex;

	iCode = _GetCode(strWord);
	ASSERT((iCode == 41) || (iCode == 42));
	iIndex = strWord.Find('+');
	ASSERT(iIndex >= 0);
	iIndex++;
	
	strDesc = strWord.Mid(iIndex);

	iLim = strlen((LPCSTR)strDesc);
	for(i = 1, iIndex = -1; i < iLim; i++)
	{
		if(!((strDesc[i - 1] == '0') && (strDesc[i] == '0')))
		{
			iIndex = i;
			break;
		}
	}
	if(iIndex < 0)
		strDesc = "";
	else
		strDesc = strDesc.Mid(iIndex);
	
	return strDesc;
}
static BOOL _XTractFBookData(const CString& strDataLine, CFBookDataRecs& RecArray, CStringArray& strArrayErrs)
{
	ASSERT(RecArray.GetSize() == 0);
	typedef enum ValidDatFields {NothingFound = 0, PtNoFound = 1, NFound = 2, EFound = 4, ZFound = 8, DescFound = 16, XDescFound = 32};

	CString strErr;
	CStringArray strArray, strArrayDescWord1, strArrayDescWord2;
	FIELDBOOKDATA Rec;
	int i, iCode;
	ValidDatFields DataFound;

	DataFound = NothingFound;
	_XTractWords(strDataLine, strArray);
	
	memset(&Rec, 0, sizeof(FIELDBOOKDATA));
	for(i = 0; i < strArray.GetSize(); i++)
	{
		CString strTemp;
		int iIndex;
		iCode = _GetCode(strArray[i]);
		switch(iCode)
		{
		case 11:
			iIndex = strArray[i].Find('+');
			ASSERT(iIndex >= 0);
			iIndex++;
			strTemp = strArray[i].Mid(iIndex);
			Rec.lPtNo = atol((LPCSTR)strTemp);
			DataFound = (ValidDatFields)(DataFound|PtNoFound);
			break;
		case 81:
			iIndex = strArray[i].Find('+');
			ASSERT(iIndex >= 0);
			iIndex++;
			strTemp = strArray[i].Mid(iIndex);
			Rec.SPoint.fEasting = atof((LPCSTR)strTemp);
			Rec.SPoint.fEasting = Rec.SPoint.fEasting / 1000.0;
			DataFound = (ValidDatFields)(DataFound|EFound);
			break;
		case 82:
			iIndex = strArray[i].Find('+');
			ASSERT(iIndex >= 0);
			iIndex++;
			strTemp = strArray[i].Mid(iIndex);
			Rec.SPoint.fNorthing = atof((LPCSTR)strTemp);
			Rec.SPoint.fNorthing = Rec.SPoint.fNorthing / 1000.0;
			DataFound = (ValidDatFields)(DataFound|NFound);
			break;
		case 83:
			iIndex = strArray[i].Find('+');
			ASSERT(iIndex >= 0);
			iIndex++;
			strTemp = strArray[i].Mid(iIndex);
			Rec.SPoint.fElev = atof((LPCSTR)strTemp);
			Rec.SPoint.fElev = Rec.SPoint.fElev / 1000.0;
			DataFound = (ValidDatFields)(DataFound|ZFound);
			break;
		case 41:	
			strArrayDescWord1.Add(strArray[i]); 
			DataFound = (ValidDatFields)(DataFound|DescFound);
			break;
		case 42:
			strArrayDescWord2.Add(strArray[i]);
			DataFound = (ValidDatFields)(DataFound|XDescFound);
			break;
		default:
			ASSERT(FALSE);
			break;
		}
	}
	//Create ERR strings if exists ////////////////////////// START
	if((DataFound & NothingFound) || ((DataFound & PtNoFound) == FALSE))
	{
		strErr.Format("ERROR: Unknown data line found...Conversion failed");
		strArrayErrs.Add(strErr);
		return FALSE;
	}
	if((DataFound & NFound) == FALSE)
	{
		strErr.Format("ERROR: Northing value not found for Point#%d...Conversion failed", Rec.lPtNo);
		strArrayErrs.Add(strErr);
		return FALSE;
	}
	if((DataFound & EFound) == FALSE)
	{
		strErr.Format("ERROR: Easting value not found for Point#%d...Conversion failed", Rec.lPtNo);
		strArrayErrs.Add(strErr);
		return FALSE;
	}
	if((DataFound & ZFound) == FALSE)
	{//Optional
		strErr.Format("WARNING: Elevation value not found for Point#%d", Rec.lPtNo);
		strArrayErrs.Add(strErr);
	}
	if((DataFound & DescFound) == FALSE)
	{//Optional
		strErr.Format("WARNING: No description (i.e. layer) found for Point#%d", Rec.lPtNo);
		strArrayErrs.Add(strErr);
	}
	if((DataFound & XDescFound) == FALSE)
	{//Optional
		strErr.Format("WARNING: No Extra description found for Point#%d", Rec.lPtNo);
		strArrayErrs.Add(strErr);
	}
	//Create ERR strings if exists ////////////////////////// END

	{////First Description
		FIELDBOOKDATA* pRec;

		for(i = 0; i < strArrayDescWord1.GetSize(); i++)
		{
			pRec = new FIELDBOOKDATA;
			memset(pRec, 0, sizeof(FIELDBOOKDATA));
			memcpy(pRec, &Rec, sizeof(FIELDBOOKDATA));
			strcpy(pRec->szDesc, (LPCSTR)_XTractDesc(strArrayDescWord1[i]));
			RecArray.Add(pRec);
		}
	}
	{////Last Description
		ASSERT(RecArray.GetSize() >= strArrayDescWord2.GetSize());

		FIELDBOOKDATA* pRec;

		for(i = 0; i < strArrayDescWord2.GetSize(); i++)
		{
			pRec = RecArray[i];
			strcpy(pRec->szXtraDesc, (LPCSTR)_XTractDesc(strArrayDescWord2[i]));
		}
	}
	if(RecArray.GetSize() <= 0)
	{//Chk whether this Record is valid or not.......Pt#/Northing/Easting fields are must...Elev/Desc/XDesc are optional
		if((DataFound & PtNoFound) && (DataFound & NFound) && (DataFound & EFound))
		{
			FIELDBOOKDATA* pRec;
			pRec = new FIELDBOOKDATA;
			memset(pRec, 0, sizeof(FIELDBOOKDATA));
			memcpy(pRec, &Rec, sizeof(FIELDBOOKDATA));
			RecArray.Add(pRec);
			return TRUE;
		}
	}
	return FALSE;
}
static void _WriteData(CStdioFile& File, const CFBookDataRecs& RecArray)
{
	int i;
	CString strToWrite;

	for(i = 0; i < RecArray.GetSize(); i++)
	{
		const FIELDBOOKDATA* pRec = RecArray[i];
		strToWrite.Format("%7d\t%12.4f\t%12.4f\t%12.4f\t%12s\t%12s\n",
						  pRec->lPtNo,
						  pRec->SPoint.fNorthing, pRec->SPoint.fEasting, pRec->SPoint.fElev,
						  pRec->szDesc, pRec->szXtraDesc
						 );
		File.WriteString(strToWrite);
	}
}

static BOOL _XTractWords(const CString& strDataLine, CStringArray& strArray)
{
	ASSERT(strArray.GetSize() == 0);
	const CString strWhite(" \t");
	CString strTemp;
	BOOL bFlag;
	int i, iIndex;
	
	strTemp = strDataLine;
	do
	{
		int iCode;
		iIndex = strTemp.FindOneOf(strWhite);
		bFlag = (iIndex >= 0);
		if(!bFlag)
		{
			iCode = _GetCode(strTemp);
			if((iCode == 11) || (iCode == 81) || (iCode == 82) || (iCode == 83) || (iCode == 41) || (iCode == 42))
				strArray.Add(strTemp);
			break;
		}
		{//Add only valid words.........
			CString strWord;

			strWord = strTemp.Mid(0, iIndex);
			strWord.TrimLeft();
			strWord.TrimRight();

			iCode = _GetCode(strWord);
			if((iCode == 11) || (iCode == 81) || (iCode == 82) || (iCode == 83) || (iCode == 41) || (iCode == 42))
				strArray.Add(strWord);
		}
		strTemp = strTemp.Mid(++iIndex);
	}
	while(bFlag);

	//Debug Dump
	TRACE("\n_XTractWords>>Dump.................");
	for(i = 0; i < strArray.GetSize(); i++)
		TRACE("\n<%s>", (LPCSTR)strArray[i]);
	return TRUE;
}
