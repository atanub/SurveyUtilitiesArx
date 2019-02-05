//OTravXLSReader.h
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef _OTRAVXLSREADER_H
#define _OTRAVXLSREADER_H

#include "ExcelReaderGen.h"
#include "OTravData.h"

class COTravXLSReader : public CExcelReaderGen
{
public:
	COTravXLSReader();
	virtual ~COTravXLSReader();

public:
	BOOL ReadAllSheets(CStringArray*);
	
	BOOL HasData() const { return (m_pData != 0L);}
	COTravData* GetData() { return m_pData;}
	void UnInitialize(const BOOL& = FALSE);
private:	
	BOOL _Read(CStringArray* pErrStrArray);
	BOOL _ReadGenDataRow(CXLWorksheet&, const int&, TRAVVERTEX&, CStringArray*) const;
	
	BOOL _ReadFirstGPSPoint(CXLWorksheet&, const int&, TRAVVERTEX&, CStringArray*) const;
	BOOL _ReadSecondGPSPoint(CXLWorksheet&, const int&, TRAVVERTEX&, CStringArray*) const;
	BOOL _ReadThirdGPSPoint(CXLWorksheet&, const int&, TRAVVERTEX&, CStringArray*) const;
	BOOL _ReadFourthGPSPoint(CXLWorksheet&, const int&, TRAVVERTEX&, CStringArray*) const;

private:
	COTravData*	m_pData;

private:
	static const CString	m_strConstSheet;	//Field Book Alignment
};
#endif //_OTRAVXLSREADER_H


//Hrudayalaya@sify.com
//Debi Shetty
//PG Girish