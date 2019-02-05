//FieldBookReader.h
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef _FIELDBOOKREADER_H
#define _FIELDBOOKREADER_H

#include "ExcelReaderGen.h"
#include "FBSheetData.h"

class CFieldBookReader : public CExcelReaderGen
{
public:
	CFieldBookReader();
	virtual ~CFieldBookReader();

public:
	BOOL ReadAllSheets(CStringArray*);
	
	BOOL HasData() const { return (m_pData != 0L);}
	CFBSheetData* GetData() { return m_pData;}
	void UnInitialize(const BOOL& = FALSE);
private:	
	BOOL _ReadSurveyObjDefs(CStringArray* pErrStrArray);
	BOOL _ReadSurveyObjs(CStringArray* pErrStrArray);
	BOOL _ReadScale(CStringArray* pErrStrArray);
	BOOL _ReadAlignment(CStringArray* pErrStrArray);

private:	
	CFBSheetData*	m_pData;

private:
	static const CString	m_strConstAlignmentSheet;	//Field Book Alignment
	static const CString	m_strConstSurveyObjSheet;	//Field Book Surveyed Objects
	static const CString	m_strConstObjSheet;			//Field Book Objects Definitions
	static const CString	m_strConstScaleSheet;		//Field Book Scale
	static const int		m_iMaxNoOfErrors;
};
#endif //_FIELDBOOKREADER_H
