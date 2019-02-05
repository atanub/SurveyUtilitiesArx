// SurveyedStation.cpp : implementation file
//
#include "stdafx.h"
#include "Util.h"
#include "DrawUtil.h"
#include "SurvUtilApp.h"
#include "SurveyedStation.h"
#include "ResultDlg.h"
#include "ConvertRDPlanDataFileDlg.h"

const char* CSurveyedStation::m_pszDataSegStartTag		= "START CHAINAGE";
const char* CSurveyedStation::m_pszDataSegEndTag		= "END CHAINAGE";
const int	CSurveyedStation::m_iMaxNoOfErrsReporting	= 500;


CSurveyedStation::CSurveyedStation()
{
}
CSurveyedStation::~CSurveyedStation()
{
	_Reset();
}
BOOL CSurveyedStation::_ReadPID(const char* pszFile, CStringArray& arrayErrs, BOOL& bIsEOF)
{
	CStdioFile	FileInp;
	CString		strTmp;
	long iLineNo;

	bIsEOF = FALSE;
	if(!FileInp.Open(pszFile, CFile::modeRead|CFile::typeText))
	{
		strTmp.Format("ERROR: Unable to open input file \"%s\"", pszFile);
		arrayErrs.Add(strTmp);
		return FALSE;
	}
	ads_printf("\rPlease wait... reading data file");
	
	if(m_arrayPID.length() > 0)
	{//Make this empty
		for(; (m_arrayPID.length() > 0);)
		{
			m_arrayPID.removeFirst();
		}
	}
	for(iLineNo = 0;;)
	{
		CStringArray strWordArray;
		
		if(ReadNextValidLine(FileInp, strWordArray, iLineNo))
		{
			if(strWordArray.GetSize() > 2)
			{
				strTmp.Format("WARNING: Invalid data found at line#%d (file : \"%s\")", iLineNo, pszFile);
				arrayErrs.Add(strTmp);
			}
			if(strWordArray.GetSize() < 2)
			{
				strTmp.Format("ERROR: Invalid data found at line#%d (file : \"%s\")", iLineNo, pszFile);
				arrayErrs.Add(strTmp);
			}
			else
			{
				AcGePoint3d ptTmp;

				ptTmp.x	= atof(strWordArray[1]);//Easting
				ptTmp.y	= atof(strWordArray[0]);//Northing
				ptTmp.z	= 0;//Default

				m_arrayPID.append(ptTmp);
			}
		}
		else
		{//EOF Reached!!
			bIsEOF = TRUE;
			break;
		}
	}
	return (m_arrayPID.length() > 0);
}

BOOL CSurveyedStation::_ReadStnNEZ(CStdioFile& FileInp, double& fChg, AcGePoint3dArray* pArrayData, CStringArray* pArrayLyrs, CStringArray& arrayErrs, long& iLineNo, BOOL& bIsEOF) const
{
	CString		strTmp;

	//Read the Header info ("START CHAINAGE") first
	bIsEOF = FALSE;
	for(;;)
	{
		CStringArray strWordArray;
		
		if(ReadNextValidLine(FileInp, strWordArray, iLineNo))
		{
			if(strWordArray.GetSize() > 2)
			{
				strTmp.Format("WARNING: Invalid data found at line#%d (file : \"%s\")", iLineNo, FileInp.GetFileName());
				arrayErrs.Add(strTmp);
			}
			if(strWordArray.GetSize() < 2)
			{
				strTmp.Format("ERROR: Invalid data found at line#%d (file : \"%s\")", iLineNo, FileInp.GetFileName());
				arrayErrs.Add(strTmp);
			}
			else
			{
				CString strTmp;
				
				strTmp.Format("%s %s", strWordArray[0], strWordArray[1]);
				
				if(0 != strTmp.CompareNoCase(m_pszDataSegStartTag))
				{
					strTmp.Format("ERROR: Data segment boundary tag \"%s\" not found at line#%d (file : \"%s\")", m_pszDataSegStartTag, iLineNo, FileInp.GetFileName());
					arrayErrs.Add(strTmp);
					return FALSE;
				}
				//NOTE: Chainage will be calculated by the program
				fChg = -10.0;
				//fChg = atof(strWordArray[2]);//Chainage
				//if(fChg < 0.0)
				//{
				//	strTmp.Format("ERROR: Invalid chainage (%s) found at line#%d (file : \"%s\")", strWordArray[2], m_pszDataSegStartTag, iLineNo, FileInp.GetFileName());
				//	arrayErrs.Add(strTmp);
				//	return FALSE;
				//}
				break;
			}
		}
		else
		{//EOF Reached!!
			//strTmp.Format("ERROR: Data segment boundary tag \"%s\" not found at line#%d (file : \"%s\")", m_pszDataSegStartTag, iLineNo, FileInp.GetFileName());
			//arrayErrs.Add(strTmp);
			bIsEOF = TRUE;
			return FALSE;
		}
	}
	//
	//Read the data <<N>><<E>><<Elev>><<Lyr>>
	//
	for(;;)
	{
		CStringArray strWordArray;
		
		if(ReadNextValidLine(FileInp, strWordArray, iLineNo))
		{
			if(strWordArray.GetSize() >= 2)
			{//Check if EOF Station Data
				CString strTmp;
				
				strTmp.Format("%s %s", strWordArray[0], strWordArray[1]);
				if(0 == strTmp.CompareNoCase(m_pszDataSegEndTag))
				{
					return (pArrayData->length() > 0);
				}
			}
			if(strWordArray.GetSize() > 4)
			{
				strTmp.Format("WARNING: Invalid(extra) data found at line#%d (file : \"%s\")", iLineNo, FileInp.GetFileName());
				arrayErrs.Add(strTmp);
			}
			if(strWordArray.GetSize() < 4)
			{
				strTmp.Format("ERROR: Invalid data found at line#%d (file : \"%s\")", iLineNo, FileInp.GetFileName());
				arrayErrs.Add(strTmp);
			}
			else
			{
				//Check...
				if(pArrayLyrs->GetSize() != pArrayData->length())
				{
					strTmp.Format("FATAL ERROR: Array lengths mismatch in func '_ReadStnOffElev()'");
					arrayErrs.Add(strTmp);
					return FALSE;
				}
				AcGePoint3d ptTmp;

				ptTmp.y	= atof(strWordArray[0]);//Northing
				ptTmp.x	= atof(strWordArray[1]);//Easting
				ptTmp.z	= atof(strWordArray[2]);//Elev

				pArrayData->append(ptTmp);
				pArrayLyrs->Add(strWordArray[3]);//Layer Name
			}
		}
		else
		{//EOF Reached!!
			bIsEOF = TRUE;
			break;
		}
	}
	return FALSE;//End Tag not found!!
}

BOOL CSurveyedStation::_ReadStnOffElev(CStdioFile& FileInp, double& fChg, AcGePoint3dArray* pArrayData, CStringArray* pArrayLyrs, CStringArray& arrayErrs, long& iLineNo, BOOL& bIsEOF) const
{
	CString	strTmp;
	
	//Read the Header info ("START CHAINAGE") first
	bIsEOF = FALSE;
	fChg = 0.0;
	for(;;)
	{
		CStringArray strWordArray;
		
		if(ReadNextValidLine(FileInp, strWordArray, iLineNo))
		{
			if(strWordArray.GetSize() > 3)
			{
				strTmp.Format("WARNING: Invalid data found at line#%d (file : \"%s\")", iLineNo, FileInp.GetFileName());
				arrayErrs.Add(strTmp);
			}
			if(strWordArray.GetSize() < 3)
			{
				strTmp.Format("ERROR: Invalid data found at line#%d (file : \"%s\")", iLineNo, FileInp.GetFileName());
				arrayErrs.Add(strTmp);
			}
			else
			{
				CString strTmp;
				
				strTmp.Format("%s %s", strWordArray[0], strWordArray[1]);
				
				if(0 != strTmp.CompareNoCase(m_pszDataSegStartTag))
				{
					strTmp.Format("ERROR: Data segment boundary tag \"%s\" not found at line#%d (file : \"%s\")", m_pszDataSegStartTag, iLineNo, FileInp.GetFileName());
					arrayErrs.Add(strTmp);
					return FALSE;
				}
				fChg = atof(strWordArray[2]);//Chainage
				if(fChg < 0.0)
				{
					strTmp.Format("ERROR: Invalid chainage (%s) found at line#%d (file : \"%s\")", strWordArray[2], m_pszDataSegStartTag, iLineNo, FileInp.GetFileName());
					arrayErrs.Add(strTmp);
					return FALSE;
				}
				break;
			}
		}
		else
		{//EOF Reached!!
			//strTmp.Format("ERROR: Data segment boundary tag \"%s\" not found at line#%d (file : \"%s\")", m_pszDataSegStartTag, iLineNo, FileInp.GetFileName());
			//arrayErrs.Add(strTmp);
			bIsEOF = TRUE;
			return FALSE;
		}
	}
	//
	//Read the data <<Offset>><<Elev>><<Code=Layer>>
	//
	for(;;)
	{
		CStringArray strWordArray;
		
		if(ReadNextValidLine(FileInp, strWordArray, iLineNo))
		{
			if(strWordArray.GetSize() >= 2)
			{//Check if EOF Station Data
				CString strTmp;
				
				strTmp.Format("%s %s", strWordArray[0], strWordArray[1]);
				if(0 == strTmp.CompareNoCase(m_pszDataSegEndTag))
				{
					return TRUE;
				}
			}
			if(strWordArray.GetSize() > 3)
			{
				strTmp.Format("WARNING: Invalid (extra) data found at line#%d (file : \"%s\")", iLineNo, FileInp.GetFileName());
				arrayErrs.Add(strTmp);
			}
			if(strWordArray.GetSize() < 3)
			{
				strTmp.Format("ERROR: Invalid data found at line#%d (file : \"%s\")", iLineNo, FileInp.GetFileName());
				arrayErrs.Add(strTmp);
			}
			else
			{
				//Check...
				if(pArrayLyrs->GetSize() != pArrayData->length())
				{
					strTmp.Format("FATAL ERROR: Array lengths mismatch in func '_ReadStnOffElev()'");
					arrayErrs.Add(strTmp);
					return FALSE;
				}
				AcGePoint3d ptTmp;

				ptTmp.x	= atof(strWordArray[0]);//Offset
				ptTmp.y	= atof(strWordArray[1]);//Elevation
				ptTmp.z	= 0.0;//Unused

				pArrayData->append(ptTmp);
				pArrayLyrs->Add(strWordArray[2]);//Layer Name
			}
		}
		else
		{//EOF Reached!!
			bIsEOF = TRUE;
			break;
		}
	}
	return FALSE;
}
BOOL CSurveyedStation::ReadOffElevData(const char* pszPIDFile, const char* pszCSDataFile, CStringArray& arrayErrs)
{
	CStdioFile fileCSData;
	CString		strTmp;
	BOOL bIsEOF;
	int i;
	long iLineNo;

	_Reset();
	
	iLineNo = 0L;
	if(pszPIDFile != 0L)
	{
		if(!_ReadPID(pszPIDFile, arrayErrs, bIsEOF))
		{
			return FALSE;
		}
	}

	if(!fileCSData.Open(pszCSDataFile, CFile::modeRead|CFile::typeText))
	{
		strTmp.Format("ERROR: Unable to open input file \"%s\"", pszCSDataFile);
		arrayErrs.Add(strTmp);
		return FALSE;
	}
	for(bIsEOF = FALSE, i = 0, iLineNo = 0L; !bIsEOF; i++)
	{
		double fChg;
		AcGePoint3dArray* pPt3DArray;
		CStringArray* pArrayLyrNames;
	
		
		pPt3DArray = new AcGePoint3dArray();
		pArrayLyrNames = new CStringArray();
		if(_ReadStnOffElev(fileCSData, fChg, pPt3DArray, pArrayLyrNames, arrayErrs, iLineNo, bIsEOF))
		{
			if(m_fChgArray.length() > 0)
			{
				double fChgPrev;
				
				fChgPrev = m_fChgArray.last();
				if(fChg <= fChgPrev)
				{
					strTmp.Format("ERROR: Invalid chainage (Current chainage %f is lesser than previous chainage %f) found near line #%d in file \"%s\"", fChg, fChgPrev, iLineNo, pszCSDataFile);
					arrayErrs.Add(strTmp);
					delete pPt3DArray;
					delete pArrayLyrNames;
					return FALSE;
				}
			}
			m_stnDataArray.append(pPt3DArray);
			m_fChgArray.append(fChg);
			m_arrayLyrGroups.append(pArrayLyrNames);
		}
		else
		{//Error Reading Station
			delete pPt3DArray;
			delete pArrayLyrNames;
			//don't break...keep reading;
		}
		if(ads_usrbrk() == 1)
		{
			strTmp.Format("ERROR: User Break encountered at line #%d(CH:%.3f), while reading file %s", iLineNo, fChg, pszCSDataFile);
			ads_printf("\n%s", strTmp);
			arrayErrs.Add(strTmp);
			return FALSE;
		}
		if(arrayErrs.GetSize() > m_iMaxNoOfErrsReporting)
		{
			strTmp.Format("Checking terminated, Please fix the listed errors & then try again");
			arrayErrs.Add(strTmp);
			return FALSE;
		}
	}
	return ((m_stnDataArray.length() > 0) && (arrayErrs.GetSize() <= 0));
}
//Special Case
//for Stn data having Co-ordinate instead of Chainage
BOOL CSurveyedStation::ReadOffElevDataEx(const char* pszPIDFile, const char* pszCSDataFile, CStringArray& arrayErrs)
{
	CStdioFile fileCSData;
	CString		strTmp;
	BOOL bIsEOF;
	int i;
	long iLineNo;

	_Reset();
	
	iLineNo = 0L;
	if(pszPIDFile != 0L)
	{
		if(!_ReadPID(pszPIDFile, arrayErrs, bIsEOF))
		{
			return FALSE;
		}
	}

	if(!fileCSData.Open(pszCSDataFile, CFile::modeRead|CFile::typeText))
	{
		strTmp.Format("ERROR: Unable to open input file \"%s\"", pszCSDataFile);
		arrayErrs.Add(strTmp);
		return FALSE;
	}
	for(bIsEOF = FALSE, i = 0, iLineNo = 0L; !bIsEOF; i++)
	{
		AcGePoint3dArray* pPt3DArray;
		AcGePoint3d ptStnCoOrd;
		CStringArray* pArrayLyrNames;
		
		pPt3DArray = new AcGePoint3dArray();
		pArrayLyrNames = new CStringArray();
		if(_ReadStnOffElevEx(fileCSData, ptStnCoOrd, pPt3DArray, pArrayLyrNames, arrayErrs, iLineNo, bIsEOF))
		{
			m_stnDataArray.append(pPt3DArray);
			m_arrayStnCoOrds.append(ptStnCoOrd);
			m_arrayLyrGroups.append(pArrayLyrNames);
		}
		else
		{//Error Reading Station
			delete pPt3DArray;
			delete pArrayLyrNames;
			//don't break...keep reading;
		}
		if(ads_usrbrk() == 1)
		{
			strTmp.Format("ERROR: User Break encountered at line #%d(Station N:%.3f, E:%.3f), while reading file %s", iLineNo, ptStnCoOrd.y, ptStnCoOrd.x, pszCSDataFile);
			ads_printf("\n%s", strTmp);
			arrayErrs.Add(strTmp);
			return FALSE;
		}
		if(arrayErrs.GetSize() > m_iMaxNoOfErrsReporting)
		{
			strTmp.Format("Checking terminated, Please fix the listed errors & then try again");
			arrayErrs.Add(strTmp);
			return FALSE;
		}
	}
	return ((m_stnDataArray.length() > 0) && (arrayErrs.GetSize() <= 0));
}
//Special Case
//for Stn data having Co-ordinate instead of Chainage
BOOL CSurveyedStation::_ReadStnOffElevEx(CStdioFile& FileInp, AcGePoint3d& ptStnCoOrd, AcGePoint3dArray* pArrayData, CStringArray* pArrayLyrs, CStringArray& arrayErrs, long& iLineNo, BOOL& bIsEOF) const
{
	CString	strTmp;
	
	//Read the Header info ("START CHAINAGE") first
	bIsEOF = FALSE;
	for(;;)
	{
		CStringArray strWordArray;
		
		if(ReadNextValidLine(FileInp, strWordArray, iLineNo))
		{
			if(strWordArray.GetSize() > 4)
			{
				strTmp.Format("WARNING: Invalid data found at line#%d (file : \"%s\")", iLineNo, FileInp.GetFileName());
				arrayErrs.Add(strTmp);
			}
			if(strWordArray.GetSize() < 4)
			{
				strTmp.Format("ERROR: Invalid data found at line#%d (file : \"%s\")", iLineNo, FileInp.GetFileName());
				arrayErrs.Add(strTmp);
			}
			else
			{
				CString strTmp;
				
				strTmp.Format("%s %s", strWordArray[0], strWordArray[1]);
				
				if(0 != strTmp.CompareNoCase(m_pszDataSegStartTag))
				{
					strTmp.Format("ERROR: Data segment boundary tag \"%s\" not found at line#%d (file : \"%s\")", m_pszDataSegStartTag, iLineNo, FileInp.GetFileName());
					arrayErrs.Add(strTmp);
					return FALSE;
				}
				ptStnCoOrd.y = atof(strWordArray[2]);//N
				ptStnCoOrd.x = atof(strWordArray[3]);//E
				ptStnCoOrd.z = 0.0;
				break;
			}
		}
		else
		{//EOF Reached!!
			//strTmp.Format("ERROR: Data segment boundary tag \"%s\" not found at line#%d (file : \"%s\")", m_pszDataSegStartTag, iLineNo, FileInp.GetFileName());
			//arrayErrs.Add(strTmp);
			bIsEOF = TRUE;
			return FALSE;
		}
	}
	//
	//Read the data <<Offset>><<Elev>><<Code=Layer>>
	//
	for(;;)
	{
		CStringArray strWordArray;
		
		if(ReadNextValidLine(FileInp, strWordArray, iLineNo))
		{
			if(strWordArray.GetSize() >= 2)
			{//Check if EOF Station Data
				CString strTmp;
				
				strTmp.Format("%s %s", strWordArray[0], strWordArray[1]);
				if(0 == strTmp.CompareNoCase(m_pszDataSegEndTag))
				{
					return TRUE;
				}
			}
			if(strWordArray.GetSize() > 3)
			{
				strTmp.Format("WARNING: Invalid (extra) data found at line#%d (file : \"%s\")", iLineNo, FileInp.GetFileName());
				arrayErrs.Add(strTmp);
			}
			if(strWordArray.GetSize() < 3)
			{
				strTmp.Format("ERROR: Invalid data found at line#%d (file : \"%s\")", iLineNo, FileInp.GetFileName());
				arrayErrs.Add(strTmp);
			}
			else
			{
				//Check...
				if(pArrayLyrs->GetSize() != pArrayData->length())
				{
					strTmp.Format("FATAL ERROR: Array lengths mismatch in func '_ReadStnOffElevEx()'");
					arrayErrs.Add(strTmp);
					return FALSE;
				}
				AcGePoint3d ptTmp;

				ptTmp.x	= atof(strWordArray[0]);//Offset
				ptTmp.y	= atof(strWordArray[1]);//Elevation
				ptTmp.z	= 0.0;//Unused

				pArrayData->append(ptTmp);
				pArrayLyrs->Add(strWordArray[2]);//Layer Name
			}
		}
		else
		{//EOF Reached!!
			bIsEOF = TRUE;
			break;
		}
	}
	return FALSE;
}
BOOL CSurveyedStation::ReadNEZData(const char* pszPIDFile, const char* pszCSDataFile, CStringArray& arrayErrs)
{
	CStdioFile fileCSData;
	CString		strTmp;
	BOOL bIsEOF;
	int i;
	long iLineNo;

	_Reset();
	
	if(pszPIDFile != 0L)
	{
		if(!_ReadPID(pszPIDFile, arrayErrs, bIsEOF))
		{
			return FALSE;
		}
	}

	if(!fileCSData.Open(pszCSDataFile, CFile::modeRead|CFile::typeText))
	{
		strTmp.Format("ERROR: Unable to open input file \"%s\"", pszCSDataFile);
		arrayErrs.Add(strTmp);
		return FALSE;
	}
	for(bIsEOF = FALSE, i = 0, iLineNo = 0L; !bIsEOF; i++)
	{
		double fChg;
		AcGePoint3dArray* pPt3DArray;
		CStringArray* pArrayLyrNames;

		pPt3DArray = new AcGePoint3dArray();
		pArrayLyrNames = new CStringArray();
		if(_ReadStnNEZ(fileCSData, fChg, pPt3DArray, pArrayLyrNames, arrayErrs, iLineNo, bIsEOF))
		{
			//NOTE : Chainage will be calculated by program
			//if(m_fChgArray.length() > 0)
			//{
			//	double fChgPrev;
			//	
			//	fChgPrev = m_fChgArray.last();
			//	if(fChg <= fChgPrev)
			//	{
			//		strTmp.Format("ERROR: Invalid chainage (Current chainage %f is lesser than previous chainage %f) found near line #%d in file \"%s\"", fChg, fChgPrev, iLineNo, pszCSDataFile);
			//		arrayErrs.Add(strTmp);
			//		delete pPt3DArray;
			//		return FALSE;
			//	}
			//}
			m_stnDataArray.append(pPt3DArray);
			m_fChgArray.append(fChg);
			m_arrayLyrGroups.append(pArrayLyrNames);
		}
		else
		{//Error Reading Station
			delete pPt3DArray;
			delete pArrayLyrNames;
			//don't break...keep reading;
		}
		if(ads_usrbrk() == 1)
		{
			strTmp.Format("ERROR: User Break encountered at line #%d, while reading file %s", iLineNo, pszCSDataFile);
			ads_printf("\n%s", strTmp);
			arrayErrs.Add(strTmp);
			return FALSE;
		}
		if(arrayErrs.GetSize() > m_iMaxNoOfErrsReporting)
		{
			strTmp.Format("Checking terminated, Please fix the listed errors & then try again");
			arrayErrs.Add(strTmp);
			return FALSE;
		}
	}
	return ((m_stnDataArray.length() > 0) && (arrayErrs.GetSize() <= 0));
}
void CSurveyedStation::_Reset()
{
	if(m_arrayPID.length() > 0)
	{//Make PID empty
		for(; (m_arrayPID.length() > 0);)
		{
			m_arrayPID.removeFirst();
		}
	}
	if(m_fChgArray.length() > 0)
	{//Make Chg Array empty
		for(; (m_fChgArray.length() > 0);)
		{
			m_fChgArray.removeFirst();
		}
	}

	if(m_stnDataArray.length() > 0)
	{//Make stn data Array empty
		for(; (m_stnDataArray.length() > 0);)
		{
			AcGePoint3dArray* pPtr;
			
			pPtr = (AcGePoint3dArray*)m_stnDataArray.first();
			delete pPtr;
			m_stnDataArray.removeFirst();
		}
	}
	if(m_arrayLyrGroups.length() > 0)
	{//Make layer group data Array empty
		for(; (m_arrayLyrGroups.length() > 0);)
		{
			CStringArray* pPtr;
			
			pPtr = (CStringArray*)m_arrayLyrGroups.first();
			delete pPtr;
			m_arrayLyrGroups.removeFirst();
		}
	}
	if(m_arrayStnCoOrds.length() > 0)
	{//Make array empty
		for(; (m_arrayStnCoOrds.length() > 0);)
		{
			m_arrayStnCoOrds.removeFirst();
		}
	}
}
BOOL CSurveyedStation::ModifyStnNEZFile(const char* pszFileOut, const double& fElevIncr, CStringArray& arrayErrs) const
{
	int i;
	CStdioFile	FileOut;
	CString strTmp, strToWrite;

	if(m_stnDataArray.length() <= 0)
	{
		strTmp.Format("ERROR: No data found for conversion", pszFileOut);
		ads_printf("\n%s", strTmp);
		arrayErrs.Add(strTmp);
		return FALSE;
	}
	if(!FileOut.Open(pszFileOut, CFile::modeWrite|CFile::modeCreate|CFile::typeText))
	{
		strTmp.Format("ERROR: Unable to open input file \"%s\"", pszFileOut);
		arrayErrs.Add(strTmp);
		return FALSE;
	}

	for(i = 0; i < m_stnDataArray.length(); i++)
	{
		const AcGePoint3dArray* pPtr = (AcGePoint3dArray*)m_stnDataArray.at(i);
		const double fChg = m_fChgArray.at(i);

		{//Write..
			int j;

			ads_printf("\rPlease wait... writing station data at chainage :%f(Stn# %d of %d)", fChg, (i+1), m_stnDataArray.length());
			strToWrite.Format("%s %f\n", m_pszDataSegStartTag, fChg);
			FileOut.WriteString(strToWrite);
			for(j = 0; j < pPtr->length(); j++)
			{
				AcGePoint3d Pt = pPtr->at(j);

				//<<N>><<E>><<Elev>>
				strToWrite.Format("%f\t%f\t%f\n", Pt.y, Pt.x, (Pt.z + fElevIncr));
				FileOut.WriteString(strToWrite);
			}
			strToWrite.Format("%s\n", m_pszDataSegEndTag);
			FileOut.WriteString(strToWrite);
		}
	}
	ads_printf("\n\nDone! Total %d# of stations written", i);
	return TRUE;
}
BOOL CSurveyedStation::ModifyStnOffZFile(const char* pszFileOut, const double& fElevIncr, CStringArray& arrayErrs) const
{
	int i;
	CStdioFile	FileOut;
	CString strTmp, strToWrite;

	if(m_stnDataArray.length() <= 0)
	{
		strTmp.Format("ERROR: No data found for conversion", pszFileOut);
		ads_printf("\n%s", strTmp);
		arrayErrs.Add(strTmp);
		return FALSE;
	}
	if(!FileOut.Open(pszFileOut, CFile::modeWrite|CFile::modeCreate|CFile::typeText))
	{
		strTmp.Format("ERROR: Unable to open input file \"%s\"", pszFileOut);
		arrayErrs.Add(strTmp);
		return FALSE;
	}

	for(i = 0; i < m_stnDataArray.length(); i++)
	{
		const AcGePoint3dArray* pPtr = (AcGePoint3dArray*)m_stnDataArray.at(i);
		const double fChg = m_fChgArray.at(i);

		{//Write..
			int j;

			ads_printf("\rPlease wait... writing station data at chainage :%f(Stn# %d of %d)", fChg, (i+1), m_stnDataArray.length());
			strToWrite.Format("%s %f\n", m_pszDataSegStartTag, fChg);
			FileOut.WriteString(strToWrite);
			for(j = 0; j < pPtr->length(); j++)
			{
				AcGePoint3d Pt = pPtr->at(j);
				
				//<<Offset>><<Elev>>
				strToWrite.Format("%f\t%f\n", Pt.x, (Pt.y + fElevIncr));
				FileOut.WriteString(strToWrite);
			}
			strToWrite.Format("%s\n", m_pszDataSegEndTag);
			FileOut.WriteString(strToWrite);
		}
	}
	ads_printf("\n\nDone! Total %d# of stations written", i);
	return TRUE;
}
BOOL CSurveyedStation::DrawFromNEZData(const DRAWPARAMRDPLANFRTXTFILE& Rec, CStringArray& arrayLog)
{
	int i;
	CString strMsg;

	if(!ReadNEZData(Rec.szDataFilePID, Rec.szDataFileSTN, arrayLog))
		return FALSE;

	//Draw PID
	POLYLINE(m_arrayPID, Rec.szLyrPID, Rec.bConsiderZ);
	
	//Make Chg Array empty //////////////////////////////////////////
	if(m_fChgArray.length() > 0)
	{//
		for(; (m_fChgArray.length() > 0);)
		{
			m_fChgArray.removeFirst();
		}
	}
	//Create New Chg Array //////////////////////////////////////////
	for(i = 0; i < m_stnDataArray.length(); i++)
	{
		const AcGePoint3dArray* pPtr = (AcGePoint3dArray*)m_stnDataArray.at(i);
		const CStringArray* pArrayLyrs = (CStringArray*)m_arrayLyrGroups[i];
		int iIndexA, iIndexB;

		//Check...
		if(pArrayLyrs->GetSize() != pPtr->length())
		{
			strMsg.Format("FATAL ERROR: Array lengths mismatch in func 'DrawFromNEZData()' Iteration#%d", i);
			arrayLog.Add(strMsg);
			return FALSE;
		}
		{//
			AcGePoint3d ptA, ptB;
			double fChgParam, fAlignmentAng;

			_FindEdgePts(*pPtr, iIndexA, iIndexB);
			ptA = pPtr->at(iIndexA);
			ptB = pPtr->at(iIndexB);

			if(!_FindChg(m_arrayPID, ptA, ptB, fChgParam, fAlignmentAng))
			{//ERROR
				_DrawNEZStnPts(*pPtr, pArrayLyrs, Rec, 0.0);//DEBUG
				strMsg.Format("Station data for data set #%d is invalid, Failed to calculate chainage", (i+1));
				arrayLog.Add(strMsg);
				return FALSE;
			}
			_DrawNEZStnPts(*pPtr, pArrayLyrs, Rec, fAlignmentAng);
			m_fChgArray.append(fChgParam);
			if(ads_usrbrk() == 1)
			{
				CString strTmp;

				strTmp.Format("ERROR: User Break encountered, Annotationdone upto CH:%.3f", fChgParam);
				ads_printf("\n%s", strTmp);
				arrayLog.Add(strTmp);
				return FALSE;
			}
		}
	}
	
	return TRUE;
}
void CSurveyedStation::_DrawNEZStnPts(const AcGePoint3dArray& arrayPts, const CStringArray* pArrayLyrs, const DRAWPARAMRDPLANFRTXTFILE& Rec, const double& fAngle) const
{
	int i;
	for(i = 0; i < arrayPts.length(); i++)
	{
		LAYER((LPCSTR)pArrayLyrs->GetAt(i));
		DrawSpotLevel(arrayPts[i], Rec.szBlkName, (LPCSTR)pArrayLyrs->GetAt(i), fAngle, Rec.dNumAnnSize, Rec.dBlkScaleX);
		DRAWPOINT(arrayPts[i], (LPCSTR)pArrayLyrs->GetAt(i));
	}
}
BOOL CSurveyedStation::_FindChg(const AcGePoint3dArray& arrayPID, const AcGePoint3d& ptParamA, const AcGePoint3d& ptParamB, double& fChgParam, double& fAlignmentAng)
{
	int i;
	AcGePoint3d ptPrev, ptCur, ptResult, ptParamADummy, ptParamBDummy;

	if(arrayPID.length() < 2)
	{
		ads_printf("\nERROR: Invalid #of points defined as PI points\n");
		return FALSE;
	}

	ptPrev = arrayPID[0];
	fChgParam = 0.0;
	for(i = 1; i < arrayPID.length(); i++)
	{
		ptCur = arrayPID[i];
		ptPrev = arrayPID[i - 1];
		
		ptPrev.z = 0.0;
		ptCur.z = 0.0;
		ptParamADummy = ptParamA;
		ptParamADummy.z = 0.0;
		ptParamBDummy = ptParamB;
		ptParamBDummy.z = 0.0;

		if(RTNORM == ads_inters(asDblArray(ptPrev), asDblArray(ptCur), asDblArray(ptParamADummy), asDblArray(ptParamBDummy), 1, asDblArray(ptResult)))
		{//Intersection point found
			fChgParam += Dist2D(ptPrev, ptResult);
			fAlignmentAng = Angle2D(ptPrev, ptResult);
			return TRUE;
		}
		fChgParam += Dist2D(ptPrev, ptCur);
	}
	return FALSE;//Not found
}
void CSurveyedStation::_FindMaxDistantPt(const AcGePoint3dArray& ptArrayParam, const int& iIndexBase, int& iIndexDest, double& fDist)
{
	int i;
	double fDistMax;
	
	iIndexDest = -1;	//Invalid
	fDist = -1.0;		//Invalid
	fDistMax = -1.0;	//Invalid

	for(i = 0; i < ptArrayParam.length(); i++)
	{
		fDist = Dist2D(ptArrayParam[iIndexBase], ptArrayParam[i]);
		if(fDist > fDistMax)
		{
			fDistMax = fDist;
			iIndexDest = i;
		}
	}
}
void CSurveyedStation::_FindEdgePts(const AcGePoint3dArray& ptArrayParam, int& iIndexA, int& iIndexB)
{
	int i, iIndex;
	
	AcGePoint3dArray ptBasPtDestPtAndMaxDistArray;
	for(i = 0; i < ptArrayParam.length(); i++)
	{
		int iIndexDest;
		AcGePoint3d ptSortedData;

		_FindMaxDistantPt(ptArrayParam, i, iIndexDest, ptSortedData.z);
		//AcGePoint3d.x  = Base Point Index
		//AcGePoint3d.y  = Destination Point Index
		//AcGePoint3d.z  = distance between Base  & Destination Point
	
		ptSortedData.x  = (double)i;
		ptSortedData.y  = (double)iIndexDest;

		ptBasPtDestPtAndMaxDistArray.append(ptSortedData);
	}

	iIndexA = 0;
	iIndexB = 0;
	//Find indexes having most distant location
	iIndex = 0;
	for(i = 1; i < ptBasPtDestPtAndMaxDistArray.length(); i++)
	{
		if(ptBasPtDestPtAndMaxDistArray[i].z > ptBasPtDestPtAndMaxDistArray[iIndex].z)
			iIndex = i;
	}
	iIndexA = (int)ptBasPtDestPtAndMaxDistArray[iIndex].x;
	iIndexB = (int)ptBasPtDestPtAndMaxDistArray[iIndex].y;
}
BOOL CSurveyedStation::DrawFromOffElevData(const DRAWPARAMRDPLANFRTXTFILE& Rec, CStringArray& arrayLog)
{
	CString strTmp;
	int i, iCount;
	CStdioFile fileOut;
	const CString strDataSegHeader("\n*Detail at Chainage : %.6f\n*|<< Offset >>|<< Elevation >>|<< Northing >>|<< Easting >>|<< Layer >>|");
	const CString strDataFmt("\n%.6f\t%.6f\t%.6f\t%.6f\t%s");

	if(!ReadOffElevData(Rec.szDataFilePID, Rec.szDataFileSTN, arrayLog))
		return FALSE;

	//Draw PID
	ads_printf("\nPlease wait...Drawing C/L of Road...");
	POLYLINE(m_arrayPID, Rec.szLyrPID, Rec.bConsiderZ);
	ads_printf("\nPlease wait...Drawing Stations...");

	if(strlen(Rec.szDataFileOut) > 0)
	{
		if(!fileOut.Open(Rec.szDataFileOut, CFile::modeCreate|CFile::modeWrite|CFile::typeText))
		{
			strTmp.Format("ERROR: Unable to open input file \"%s\"", Rec.szDataFileOut);
			arrayLog.Add(strTmp);
			return FALSE;
		}
	}
	//Draw Points Across PID
	for(i = 0, iCount = 0; i < m_fChgArray.length(); i++)
	{
		AcGePoint3d ptAtZeroOffset;
		double fAngleOfPath;
		
		if(_FindCoOrdAtZeroOffset(m_fChgArray[i], arrayLog, ptAtZeroOffset, fAngleOfPath))
		{
			int j;
			const AcGePoint3dArray* pStnDataArray = (AcGePoint3dArray*)m_stnDataArray[i];
			const CStringArray* pArrayLyrs = (CStringArray*)m_arrayLyrGroups[i];
			const double fAngleAcrossPath = (fAngleOfPath - PI / 2.0) ;
			//Check...
			if(pArrayLyrs->GetSize() != pStnDataArray->length())
			{
				strTmp.Format("FATAL ERROR: Array lengths mismatch in func 'DrawFromOffElevData()' Iteration#%d", i);
				arrayLog.Add(strTmp);
				return FALSE;
			}
			
			iCount++;
			ads_printf("\rPlease wait...Drawing Station at CH:%.3f (%d of %d)...", m_fChgArray[i], iCount, m_fChgArray.length());
			//Data file output
			if(strlen(Rec.szDataFileOut) > 0)
			{
				strTmp.Format(strDataSegHeader, m_fChgArray[i]);
				fileOut.WriteString(strTmp);
			}
			for(j = 0; j < pStnDataArray->length(); j++)
			{
				AcGePoint3d pt3DTmp;
				
				pt3DTmp = _CalcPoint(ptAtZeroOffset, pStnDataArray->at(j).x, fAngleOfPath);
				pt3DTmp.z = pStnDataArray->at(j).y;

				LAYER((LPCSTR)pArrayLyrs->GetAt(j));
				DRAWPOINT(pt3DTmp, (LPCSTR)pArrayLyrs->GetAt(j));
				DrawSpotLevel(pt3DTmp, Rec.szBlkName, (LPCSTR)pArrayLyrs->GetAt(j), fAngleOfPath, Rec.dNumAnnSize, Rec.dBlkScaleX);
				
				//OLD DRAWPOINT(pt3DTmp, Rec.szLyrAnnotation);
				//OLD DrawSpotLevel(pt3DTmp, Rec.szBlkName, Rec.szLyrAnnotation, fAngleOfPath, Rec.dNumAnnSize, Rec.dBlkScaleX);

				//Data file output
				if(strlen(Rec.szDataFileOut) > 0)
				{
					strTmp.Format(strDataFmt, pStnDataArray->at(j).x, pStnDataArray->at(j).y, pt3DTmp.y, pt3DTmp.x, (LPCSTR)pArrayLyrs->GetAt(j));
					fileOut.WriteString(strTmp);
				}
			}
		}
		else
		{
			strTmp.Format("ERROR: Failed to draw points at CH:%.3f\nPossible Cause: Invalid Chainage", m_fChgArray[i]);
			ads_printf("\n%s", strTmp);
			arrayLog.Add(strTmp);
		}
		if(ads_usrbrk() == 1)
		{
			strTmp.Format("ERROR: User Break encountered at CH:%.3f\nTotal %d of %d# of stations annotated!", m_fChgArray[i], iCount, m_fChgArray.length());
			ads_printf("\n%s", strTmp);
			arrayLog.Add(strTmp);
			return FALSE;
		}
	}
	ads_printf("\nDone ! Total %d of %d# of stations annotated!", iCount, m_fChgArray.length());
	return TRUE;
}
BOOL CSurveyedStation::DrawFromOffElevDataEx(const DRAWPARAMRDPLANFRTXTFILE& Rec, CStringArray& arrayLog)
{
	const short iLuprec = (short)LUPREC();
	CString strTmp;
	int i, iCount;
	CStdioFile fileOut;
	const CString strDataSegHeader("\n*Detail at Chainage : %.6f\n*|<< Offset >>|<< Elevation >>|<< Northing >>|<< Easting >>|<< Layer >>|");
	const CString strDataFmt("\n%.6f\t%.6f\t%.6f\t%.6f\t%s");
	if(!ReadOffElevDataEx(Rec.szDataFilePID, Rec.szDataFileSTN, arrayLog))
		return FALSE;

	//Draw PID
	ads_printf("\nPlease wait...Drawing C/L of Road...");
	POLYLINE(m_arrayPID, Rec.szLyrPID, Rec.bConsiderZ);
	ads_printf("\nPlease wait...Drawing Stations...");

	if(strlen(Rec.szDataFileOut) > 0)
	{
		if(!fileOut.Open(Rec.szDataFileOut, CFile::modeCreate|CFile::modeWrite|CFile::typeText))
		{
			strTmp.Format("ERROR: Unable to open input file \"%s\"", Rec.szDataFileOut);
			arrayLog.Add(strTmp);
			return FALSE;
		}
	}
	//Draw Points Across PID
	for(i = 0, iCount = 0; i < m_arrayStnCoOrds.length(); i++)
	{
		const AcGePoint3d& ptAtZeroOffset = m_arrayStnCoOrds[i];
		double fAngleOfLeftMostPt;
		
		//Calculate (the angle bi-sector) i.e. the angle of line joining the vertex point & the left most point
		if(_FindLeftSideAngle(ptAtZeroOffset, fAngleOfLeftMostPt))
		{
			int j;
			AcGePoint3dArray arraySPLevels;
			const AcGePoint3dArray* pStnDataArray = (AcGePoint3dArray*)m_stnDataArray[i];
			const CStringArray* pArrayLyrs = (CStringArray*)m_arrayLyrGroups[i];
			
			fAngleOfLeftMostPt += PI;
			if(fAngleOfLeftMostPt > (PI * 2.0))
				fAngleOfLeftMostPt -= (PI * 2.0);
				
			//Check...
			if(pArrayLyrs->GetSize() != pStnDataArray->length())
			{
				strTmp.Format("FATAL ERROR: Array lengths mismatch in func 'DrawFromOffElevData()' Iteration#%d", i);
				arrayLog.Add(strTmp);
				return FALSE;
			}
			
			iCount++;
			ads_printf("\rPlease wait...Drawing Station at (N:%.3f E:%.3f) #%d of %d...", ptAtZeroOffset.y, ptAtZeroOffset.x, iCount, m_arrayStnCoOrds.length());
			//Data file output
			if(strlen(Rec.szDataFileOut) > 0)
			{
				strTmp.Format(strDataSegHeader, m_fChgArray[i]);
				fileOut.WriteString(strTmp);
			}
			for(j = 0; j < pStnDataArray->length(); j++)
			{
				AcGePoint3d pt3DTmp;
				
				pt3DTmp = _CalcPointAtOffset(ptAtZeroOffset, pStnDataArray->at(j).x, fAngleOfLeftMostPt);
				pt3DTmp.z = pStnDataArray->at(j).y;

				LAYER((LPCSTR)pArrayLyrs->GetAt(j));
				DRAWPOINT(pt3DTmp, (LPCSTR)pArrayLyrs->GetAt(j));
				//DrawSpotLevel(pt3DTmp, Rec.szBlkName, (LPCSTR)pArrayLyrs->GetAt(j), (fAngleOfLeftMostPt + PI / 2.0), Rec.dNumAnnSize, Rec.dBlkScaleX);
				//Draw Co-Ordinate Marker entities (using ContourARX API)
				InsCoOrdMarker(Rec.dNumAnnSize, Rec.dBlkScaleX, (fAngleOfLeftMostPt + PI / 2.0), iLuprec, (LPCSTR)pArrayLyrs->GetAt(j), pt3DTmp);
				
				//Data file output
				if(strlen(Rec.szDataFileOut) > 0)
				{
					strTmp.Format(strDataFmt, pStnDataArray->at(j).x, pStnDataArray->at(j).y, pt3DTmp.y, pt3DTmp.x, (LPCSTR)pArrayLyrs->GetAt(j));
					fileOut.WriteString(strTmp);
				}
			}
		}
		else
		{
			strTmp.Format("ERROR: Failed to draw points\nPossible Cause: Invalid Station Co-Ordinate [N:%.3f, E:%.3f]", ptAtZeroOffset.y, ptAtZeroOffset.x);
			ads_printf("\n%s", strTmp);
			arrayLog.Add(strTmp);
		}
		if(ads_usrbrk() == 1)
		{
			strTmp.Format("ERROR: User Break encountered at Station:[N:%.3f, E:%.3f]\nTotal %d of %d# of stations annotated!", ptAtZeroOffset.y, ptAtZeroOffset.x, iCount, m_arrayStnCoOrds.length());
			ads_printf("\n%s", strTmp);
			arrayLog.Add(strTmp);
			return FALSE;
		}
	}
	//
	//
	ads_printf("\nDone ! Total %d of %d# of stations annotated!", iCount, m_arrayStnCoOrds.length());
	return TRUE;
}
BOOL CSurveyedStation::_FindLeftSideAngle(const AcGePoint3d& ptAtZeroOffset, double& fAngleOfLeftMostPt) const
{
	//Check whether exists in PID list or not
	int i, iIndex;
	double fTmp;

	fAngleOfLeftMostPt = 0.0;
	if(m_arrayPID.length() <= 1)
		return FALSE;//# of points insufficient

	//
	for(i = 0, iIndex = -1; i < m_arrayPID.length(); i++)
	{
		if(ptAtZeroOffset == m_arrayPID[i])
		{
			iIndex = i;
		}
	}
	if(iIndex < 0)
		return FALSE;//Not found in PID

	if(iIndex == 0)
	{//1st Index
		fAngleOfLeftMostPt = Angle2D(m_arrayPID[iIndex], m_arrayPID[iIndex+1]) + (PI / 2.0) ;
		
		if(fAngleOfLeftMostPt < 0.0)
			fAngleOfLeftMostPt += (PI * 2.0);
		if(fAngleOfLeftMostPt > (PI * 2.0))
			fAngleOfLeftMostPt -= (PI * 2.0);

		fTmp = RTOD(fAngleOfLeftMostPt);
		return TRUE;
	}
	else if(iIndex == (m_arrayPID.length() - 1))
	{//last index
		fAngleOfLeftMostPt = Angle2D(m_arrayPID[iIndex-1], m_arrayPID[iIndex]) + (PI / 2.0) ;
		
		if(fAngleOfLeftMostPt < 0.0)
			fAngleOfLeftMostPt += (PI * 2.0);
		if(fAngleOfLeftMostPt > (PI * 2.0))
			fAngleOfLeftMostPt -= (PI * 2.0);

		fTmp = RTOD(fAngleOfLeftMostPt);
		return TRUE;
	}
	else
	{//intermediate
		const AcGePoint3d& ptLeft = (m_arrayPID[iIndex-1]);
		const AcGePoint3d& ptRite = (m_arrayPID[iIndex+1]);

		const AcGePoint2d ptLeft2D(ptLeft.x, ptLeft.y);
		const AcGePoint2d ptRite2D(ptRite.x, ptRite.y);
		const AcGeCircArc2d objTmp(ptLeft2D, AcGePoint2d(ptAtZeroOffset.x, ptAtZeroOffset.y), ptRite2D);

		double fAngLeft, fAngRite;

		fAngLeft = Angle2D(ptAtZeroOffset, ptLeft);
		fAngRite = Angle2D(ptAtZeroOffset, ptRite);

		{//Check whether both the lines having same angle or not
			double fAngA,  fAngB;

			fAngA = Angle2D(ptLeft, ptAtZeroOffset);
			fAngB = Angle2D(ptAtZeroOffset, ptRite);
			
			if(fAngA == fAngB)
			{
				fAngleOfLeftMostPt = fAngB + (PI / 2.0);
				return TRUE;
			}

		}
		if(objTmp.isClockWise() == Adesk::kTrue)
		{//Clockwise
			if(fAngLeft > fAngRite)
			{//4th Quadrant
				fAngleOfLeftMostPt = fAngLeft + (((PI * 2.0) - (fAngLeft - fAngRite)) / 2.0);
				fAngleOfLeftMostPt += PI;
				
				if(fAngleOfLeftMostPt > (PI * 2.0))
					fAngleOfLeftMostPt -= (PI * 2.0);

				fTmp = RTOD(fAngleOfLeftMostPt);
				ads_printf("\n\n\nfAngleOfLeftMostPt = %f", fTmp);
			}
			else
			{
				fAngleOfLeftMostPt = fAngLeft + ((fAngRite - fAngLeft) / 2.0) + PI;
			}
		}
		else
		{//Anti-Clockwise
			if(fAngRite > fAngLeft)
			{//4th Quadrant
				fAngleOfLeftMostPt = fAngLeft - (((PI * 2.0) - (fAngRite - fAngLeft)) / 2.0);
				if(fAngleOfLeftMostPt < 0.0)
					fAngleOfLeftMostPt = fAngleOfLeftMostPt + (PI * 2.0);
			}
			else
			{
				fAngleOfLeftMostPt = fAngLeft - ((fAngLeft - fAngRite) / 2.0);
			}
		}
		if(fAngleOfLeftMostPt < 0.0)
			fAngleOfLeftMostPt += (PI * 2.0);
		if(fAngleOfLeftMostPt > (PI * 2.0))
			fAngleOfLeftMostPt -= (PI * 2.0);
	
		//fAngleOfLeftMostPt += PI;
		//if(fAngleOfLeftMostPt > (PI * 2.0))
		//	fAngleOfLeftMostPt -= (PI * 2.0);
	}
	fTmp = RTOD(fAngleOfLeftMostPt);
	ads_printf("\n\n\nfAngleOfLeftMostPt = %f(final)", fTmp);
	return TRUE;
}
AcGePoint3d CSurveyedStation::_CalcPoint(const AcGePoint3d& ptCenParam, const double& fOffset, const double& fAngleAlongPath) const
{
	AcGePoint3d ptToRet;

	ads_polar(asDblArray(ptCenParam), (fAngleAlongPath + (PI * 0.5)), fOffset, asDblArray(ptToRet));

	return ptToRet;
}
AcGePoint3d CSurveyedStation::_CalcPointAtOffset(const AcGePoint3d& ptCenParam, const double& fOffset, const double& fAngle) const
{
	AcGePoint3d ptToRet;

	ads_polar(asDblArray(ptCenParam), fAngle, fOffset, asDblArray(ptToRet));

	return ptToRet;
}
BOOL CSurveyedStation::_FindCoOrdAtZeroOffset(const double& fChg, CStringArray& arrayLog, AcGePoint3d& ptParam, double& fAngleOfPath) const
{
	int i;
	double fCumDist, fDist;
	CString strTmp;

	fCumDist = 0.0;
	fAngleOfPath = 0.0;
	ptParam.x = 0.0;
	ptParam.y = 0.0;
	ptParam.z = 0.0;
	for(i = 0; i < m_arrayPID.length(); i++)
	{
		if(i > 0)
		{
			fDist = Dist2D(m_arrayPID[i-1], m_arrayPID[i]);
			if((fChg >= fCumDist) && (fChg <= (fCumDist + fDist)))
			{//Found..
				fAngleOfPath = Angle2D(m_arrayPID[i-1], m_arrayPID[i]);
				ads_polar(asDblArray(m_arrayPID[i-1]), fAngleOfPath, (fChg - fCumDist), asDblArray(ptParam));
				return TRUE;
			}
			fCumDist += fDist;
		}
	}
	strTmp.Format("ERROR: Failed to Draw Station detail at CH:%.3f..Possible cause: Chainage is out of bound!", fChg);
	arrayLog.Add(strTmp);
	return FALSE;
}
void CSurveyedStation::InsCoOrdMarker(const double& fTxtSize, const double& fMarkerScale, const double& fAngleInRad, const short& iLuprec, const char* pszLyr, const AcGePoint3d& ptOfIns) 
{ 
	const char* pszFunc = "InsCoOrdMarker";
	int i, iStatus;
	resbuf* pRBToSend;
	resbuf* pRBToRes;
	
	if(!CSurvUtilApp::LoadContourARX(TRUE))
	{
		return;
	}
	pRBToSend = ads_buildlist(RTSTR, pszFunc, RTNONE); 
	{//Insert Parameters 'n points  to RB 1st
		const double* pfParams[] = {&fTxtSize, &fMarkerScale, &fAngleInRad};
		resbuf* pRBTmp;

		pRBTmp = pRBToSend;;
		for(i = 0; i < sizeof(pfParams) / sizeof(double*); i++)
		{
			pRBTmp->rbnext = ads_buildlist(RTREAL, *(pfParams[i]), RTNONE);
			pRBTmp = pRBTmp->rbnext;
		}
		pRBTmp->rbnext = ads_buildlist(RTSHORT, iLuprec, RTNONE);
		pRBTmp = pRBTmp->rbnext;

		pRBTmp->rbnext = ads_buildlist(RT3DPOINT, asDblArray(ptOfIns), RTNONE);
		pRBTmp = pRBTmp->rbnext;

		pRBTmp->rbnext = ads_buildlist(RTSTR, pszLyr, RTNONE);
		pRBTmp = pRBTmp->rbnext;
	}
 
	pRBToRes = 0L;
	if(pRBToSend != NULL)
	{ 
		iStatus = ads_invoke(pRBToSend, &pRBToRes); 
		if(iStatus != RTNORM)
		{
			ads_printf("\n\nFATAL ERROR: Failed to invoke ContourARX API @ SurveyedStation.cpp\n\n");
		}
		if(pRBToRes != 0L)
		{//No need to check the result....
			ads_relrb(pRBToRes); 
		}
		ads_relrb(pRBToSend); 
	} 
}
void DrawRdPlanOffZFromTxtFile()
{
	DRAWPARAMRDPLANFRTXTFILE Rec;

	if(!GetDrawParamRDPlanFrTxtFile(FALSE, Rec))
		return;

	CSurveyedStation objStnGen;
	CStringArray arrayLog;

	LAYER(Rec.szLyrAnnotation, 1);
	LAYER(Rec.szLyrPID, 2);
	if(!objStnGen.DrawFromOffElevData(Rec, arrayLog))
	{
		ShowResultDlg(arrayLog);
	}
}
void ModifyRdPlanOffZTxtFile()
{
	CString strInpFile, strOutFile;
	double fElevIncr;
	CSurveyedStation objStnGen;
	CStringArray arrayLog;

	if(!GetRDPlanDataFileConversionData(strInpFile, strOutFile, fElevIncr))
		return;
	if(!objStnGen.ReadOffElevData(0L, strInpFile, arrayLog))
	{
		ShowResultDlg(arrayLog);
		return;
	}
	if(!objStnGen.ModifyStnOffZFile(strOutFile, fElevIncr, arrayLog))
	{
		ShowResultDlg(arrayLog);
		return;
	}
}
void DrawRdPlanNEZFromTxtFile()
{
	DRAWPARAMRDPLANFRTXTFILE Rec;
	
	if(!GetDrawParamRDPlanFrTxtFile(TRUE, Rec))
		return;

	CSurveyedStation objStnGen;
	CStringArray arrayLog;

	LAYER(Rec.szLyrAnnotation, 1);
	LAYER(Rec.szLyrPID, 2);
	if(!objStnGen.DrawFromNEZData(Rec, arrayLog))
	{
		ShowResultDlg(arrayLog);
	}
}
void ModifyRdPlanNEZTxtFile()
{
	CString strInpFile, strOutFile;
	double fElevIncr;
	CSurveyedStation objStnGen;
	CStringArray arrayLog;

	if(!GetRDPlanDataFileConversionData(strInpFile, strOutFile, fElevIncr))
		return;

	if(!objStnGen.ReadNEZData(0L, strInpFile, arrayLog))
	{
		ShowResultDlg(arrayLog);
		return;
	}
	if(!objStnGen.ModifyStnNEZFile(strOutFile, fElevIncr, arrayLog))
	{
		ShowResultDlg(arrayLog);
		return;
	}
}
void DrawRdPlanOffZFromTxtFileEx()
{
	DRAWPARAMRDPLANFRTXTFILE Rec;

	if(!GetDrawParamRDPlanFrTxtFile(FALSE, Rec))
		return;

	CSurveyedStation objStnGen;
	CStringArray arrayLog;

	LAYER(Rec.szLyrAnnotation, 1);
	LAYER(Rec.szLyrPID, 2);
	if(!objStnGen.DrawFromOffElevDataEx(Rec, arrayLog))
	{
		ShowResultDlg(arrayLog);
	}
}

