//ExcelReaderGen.h
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef _EXCELREADER_GEN_H
#define _EXCELREADER_GEN_H

#include "excel8.h"

class CExcelReaderGen
{
public:
	CExcelReaderGen();
	~CExcelReaderGen();
	
	BOOL Initialize();
	virtual void UnInitialize(const BOOL& = FALSE);

public:	
	BOOL Open(const CString&, CStringArray&);
	void Close();
	virtual BOOL ReadAllSheets(CStringArray*) = 0;//Pure Virtual Func.

public:	
	static BOOL FindInStrArray(const CString&, const CStringArray&, const BOOL& = TRUE);
	static CString MakeCellID(const int&, const char&);

protected:
	BOOL _GetWBookSheet(const CString&, CXLWorksheet*);
	BOOL _GetDataSegmentLimits(CXLWorksheet&, int&, int&, CStringArray*, const CString&, const CString&, const int& = -1) const;

	BOOL _ParseRawXLData(const COleVariant&, double*) const;
	BOOL _ParseRawXLData(const COleVariant&, CString*, const BOOL& = FALSE) const;

	void _DumpOleVarType(const COleVariant&) const;

protected:	
	CXLApplication	m_XLApp;
	CXLWorkbooks	m_XLWbksArray;
	CXLWkbook		m_XLWbkCur;
	CXLWksheets		m_XLWSheetArrayCur;
	CStringArray	m_SheetNamesArray;

	BOOL			m_bIsAppLaunched;
	BOOL			m_bIsWBAlreadyOpened;
	CString			m_strWBName;

protected:
	static const CString	m_strConstXLProgID;
	
	static const CString	m_strConstGenTagStart;
	static const CString	m_strConstGenTagEnd;

	static const int		m_iConstMaxRow;
	static const int		m_iConstMinRow;
	static const char		m_chConstMinCol;
};
#endif //_EXCELREADER_GEN_H
