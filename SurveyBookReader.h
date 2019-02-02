//SurveyBookReader.h
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef _SURVEYBOOKREADER_H
#define _SURVEYBOOKREADER_H

#include "ExcelReaderGen.h"
#include "SurveyData.h"
#include "RoadProfile.h"
#include "excel8.h"

class CSurveyBookReader : public CExcelReaderGen
{
public:
	CSurveyBookReader();
	virtual ~CSurveyBookReader();

	typedef enum {PID, PIN} MkSheetType;
public:	
	CSurveyData*	GetDataObjPtr() { return &m_SurveyData;}

	BOOL ReadAllSheets(CStringArray*);
	BOOL ReadMDFSheet(CStringArray*);//Master Data
	BOOL ReadPIDSheet(CStringArray*);//Point of Intersection
	BOOL ReadPRDSheet(CStringArray*);//Profile
	BOOL ReadPINSheet(CStringArray*);//Point Insertion
	BOOL ReadRXDSheet(CStringArray*);//Road X Secn (Plan)
	BOOL ReadXSNSheet(CStringArray*);//X Secn (Elev)

	BOOL HasMDFData() const;//Master Data
	BOOL HasPIDData() const;//Point of Intersection
	BOOL HasPRDData() const;//Profile
	BOOL HasPINData() const;//Point Insertion
	BOOL HasRXDData() const;//Road X Secn
	BOOL HasXSNData() const;//X Secn

	void UnInitialize(const BOOL& = FALSE);
private:
	BOOL _ReadMarkDataSheet(MkSheetType, CStringArray*);//for "Point of Intersection" & "Point Insertion"

	BOOL _GetNextRXDataSegmentLimit(CXLWorksheet&, const int&, const int&, int&, int&) const;
	BOOL _GetNextRXDataSegment(CXLWorksheet&, const int&, const int&, CMarkRecs&, CStringArray*) const;

	BOOL _GetNextXSNDataSegment(CXLWorksheet&, const int&, const int&, CPRDataRecs&, CStringArray*) const;

private:	
	CSurveyData		m_SurveyData;//To hold Proccessed data.......
private:
	static const CString	m_strConstYes;
	static const CString	m_strConstNo;

	static const CString	m_strConstMDFSheet;//Master Data
	static const CString	m_strConstRXDSheet;//Road X Secn
	static const CString	m_strConstPIDSheet;//Point of Intersection
	static const CString	m_strConstPRDSheet;//Profile
	static const CString	m_strConstPINSheet;//Point Insertion
	static const CString	m_strConstXSNSheet;//Cross Sections(Elev)

	static const char	m_chConstPtSubPtSeparator;
	static const char	m_chConstMaxColMDF;
	static const char	m_chConstMaxColRXD;
	static const char	m_chConstMaxColPRD;
	static const char	m_chConstMaxColMARK;
};
#endif //_SURVEYBOOKREADER_H
